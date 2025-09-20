#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include "d3dx12.h"
#include <wrl.h>
#include <string>
#include <sstream>
#include <memory>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"windowscodecs.lib")
using namespace Microsoft::WRL;
using namespace DirectX;
using std::unique_ptr;

namespace DX12TextureHelper
{
	// 纹理转换用，不是 DX12 所支持的格式，DX12 没法用

	// Standard GUID -> DXGI 格式转换结构体
	struct WICTranslate
	{
		GUID wic;
		DXGI_FORMAT format;
	};

	// WIC 格式与 DXGI 像素格式的对应表，该表中的格式为被支持的格式
	static WICTranslate g_WICFormats[] =
	{
		{ GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },
		{ GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },
		{ GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM },
		{ GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM },
		{ GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
		{ GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },
		{ GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
		{ GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },
		{ GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
		{ GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
		{ GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
		{ GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },
		{ GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM }
	};

	// GUID -> Standard GUID 格式转换结构体
	struct WICConvert
	{
		GUID source;
		GUID target;
	};

	// WIC 像素格式转换表
	static WICConvert g_WICConvert[] =
	{
		// 目标格式一定是最接近的被支持的格式
		{ GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray },			// DXGI_FORMAT_R8_UNORM
		{ GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray },			// DXGI_FORMAT_R8_UNORM
		{ GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray },			// DXGI_FORMAT_R8_UNORM
		{ GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf },		// DXGI_FORMAT_R16_FLOAT
		{ GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat },	// DXGI_FORMAT_R32_FLOAT
		{ GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 },		// DXGI_FORMAT_B5G5R5A1_UNORM
		{ GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 },	// DXGI_FORMAT_R10G10B10A2_UNORM
		{ GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT

		{ GUID_WICPixelFormat128bppRGBAFloat,       GUID_WICPixelFormat128bppRGBAFloat },	// DXGI_FORMAT_R32G32B32A32_FLOAT
		{ GUID_WICPixelFormat64bppRGBAHalf,         GUID_WICPixelFormat64bppRGBAHalf },		// DXGI_FORMAT_R16G16B16A16_FLOAT
		{ GUID_WICPixelFormat64bppRGBA,             GUID_WICPixelFormat64bppRGBA },			// DXGI_FORMAT_R16G16B16A16_UNORM
		{ GUID_WICPixelFormat32bppRGBA,             GUID_WICPixelFormat32bppRGBA },			// DXGI_FORMAT_R8G8B8A8_UNORM
		{ GUID_WICPixelFormat32bppBGRA,             GUID_WICPixelFormat32bppBGRA },			// DXGI_FORMAT_B8G8R8A8_UNORM
		{ GUID_WICPixelFormat32bppBGR,              GUID_WICPixelFormat32bppBGR },			// DXGI_FORMAT_B8G8R8X8_UNORM
		{ GUID_WICPixelFormat32bppRGBA1010102XR,    GUID_WICPixelFormat32bppRGBA1010102XR },// DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
		{ GUID_WICPixelFormat32bppRGBA1010102,      GUID_WICPixelFormat32bppRGBA1010102 },	// DXGI_FORMAT_R10G10B10A2_UNORM
		{ GUID_WICPixelFormat16bppBGRA5551,         GUID_WICPixelFormat16bppBGRA5551 },		// DXGI_FORMAT_B5G5R5A1_UNORM
		{ GUID_WICPixelFormat16bppBGR565,           GUID_WICPixelFormat16bppBGR565 },		// DXGI_FORMAT_B5G6R5_UNORM
		{ GUID_WICPixelFormat32bppGrayFloat,        GUID_WICPixelFormat32bppGrayFloat },	// DXGI_FORMAT_R32_FLOAT
		{ GUID_WICPixelFormat16bppGrayHalf,         GUID_WICPixelFormat16bppGrayHalf },		// DXGI_FORMAT_R16_FLOAT
		{ GUID_WICPixelFormat16bppGray,             GUID_WICPixelFormat16bppGray },			// DXGI_FORMAT_R16_UNORM
		{ GUID_WICPixelFormat8bppGray,              GUID_WICPixelFormat8bppGray },			// DXGI_FORMAT_R8_UNORM
		{ GUID_WICPixelFormat8bppAlpha,             GUID_WICPixelFormat8bppAlpha }			// DXGI_FORMAT_A8_UNORM
	};


	// 查表确定兼容的最接近格式是哪个
	bool GetTargetPixelFormat(const GUID* pSourceFormat, GUID* pTargetFormat)
	{
		*pTargetFormat = *pSourceFormat;
		for (size_t i = 0; i < _countof(g_WICConvert); ++i)
		{
			if (InlineIsEqualGUID(g_WICConvert[i].source, *pSourceFormat))
			{
				*pTargetFormat = g_WICConvert[i].target;
				return true;
			}
		}
		return false;		// 找不到，就返回 false
	}

	// 查表确定最终对应的 DXGI 格式是哪一个
	DXGI_FORMAT GetDXGIFormatFromPixelFormat(const GUID* pPixelFormat)
	{
		for (size_t i = 0; i < _countof(g_WICFormats); ++i)
		{
			if (InlineIsEqualGUID(g_WICFormats[i].wic, *pPixelFormat))
			{
				return g_WICFormats[i].format;
			}
		}
		return DXGI_FORMAT_UNKNOWN;		// 找不到，就返回 UNKNOWN
	}
}
// DX12 引擎
class DX12Engine
{
private:
	////test
	//D3D12_RESOURCE_BARRIER beg_barrier = {};				// 渲染开始的资源屏障，呈现 -> 渲染目标
	//D3D12_RESOURCE_BARRIER end_barrier = {};				// 渲染结束的资源屏障，渲染目标 -> 呈现

	int WindowWidth = 800;		// 窗口宽度
	int WindowHeight = 600;	// 窗口高度
	HWND m_hwnd = nullptr;	// 窗口句柄
	ComPtr<ID3D12Debug> m_D3D12DebugDevice; // D3D12 调试接口
	UINT m_DXGICreateFactoryFlag = NULL; // 创建 DXGI 工厂时需要用到的标志
	ComPtr<IDXGIFactory6> m_DXGIFactory; // DXGI 工厂
	ComPtr<IDXGIAdapter1> m_DXGIAdapter; // 显卡适配器
	ComPtr<ID3D12Device> m_D3D12Device; // D3D12 设备

	ComPtr<ID3D12CommandQueue> m_CommandQueue; // 命令队列
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator; // 命令分配器
	ComPtr<ID3D12GraphicsCommandList> m_CommandList; // 命令列表

	ComPtr<IDXGISwapChain4> m_DXGISwapChain; // DXGI 交换链
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap; // RTV 描述符堆
	ComPtr<ID3D12Resource> m_RenderTarget[3]; // 三个渲染目标
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = {}; // RTV 描述符句柄

	UINT RTVDescriptorSize = 0; // RTV 描述符的大小
	UINT FrameIndex = 0; // 帧索引，表示当前渲染的第 i 帧 (第 i 个渲染目标)

	ComPtr<ID3D12Fence> m_Fence; // GPU 同步用的栅栏
	UINT64 FenceValue = 0; // 栅栏值
	HANDLE RenderEvent = nullptr; // GPU 渲染事件
	CD3DX12_RESOURCE_BARRIER barrier{};//  渲染开始的资源屏障，呈现 -> 渲染目标

	std::wstring TextureFilename = L"E:\\DirectX12Project\\DirectX12Pro\\Block/diamond_ore.png"; // 纹理文件路径
	ComPtr<IWICImagingFactory> m_WICFactory;				// WIC 工厂
	ComPtr<IWICBitmapDecoder> m_WICBitmapDecoder;			// 位图解码器
	ComPtr<IWICBitmapFrameDecode> m_WICBitmapDecodeFrame;	// 由解码器得到的单个位图帧
	ComPtr<IWICFormatConverter> m_WICFormatConverter;		// 位图转换器
	ComPtr<IWICBitmapSource> m_WICBitmapSource;				// WIC 位图资源，用于获取位图数据

	UINT TextureWidth = 0;		// 纹理宽度
	UINT TextureHeight = 0;	// 纹理高度
	UINT BitsPerPixel = 0;	// 纹理每像素位数
	UINT BytePerRowSize = 0;	// 纹理每像素字节数
	DXGI_FORMAT TextureFormat = DXGI_FORMAT_UNKNOWN; // 纹理格式
	unique_ptr<BYTE[]> TextureData = nullptr; // 纹理数据

	ComPtr<ID3D12DescriptorHeap> m_SRVHeap; // SRV 描述符堆
	D3D12_CPU_DESCRIPTOR_HANDLE SRV_CPUHandle{}; // SRV CPU 描述符句柄,用来创建描述符的
	D3D12_GPU_DESCRIPTOR_HANDLE SRV_GPUHandle{}; // SRV GPU 描述符句柄，用来绑定到管线

	ComPtr<ID3D12Resource> m_UploadTextureResource;			// 上传堆资源，位于共享内存，用于中转纹理资源
	ComPtr<ID3D12Resource> m_DefaultTextureResource;		// 默认堆资源，位于显存，用于放纹理

	UINT TextureSize = 0; // 纹理数据大小
	UINT UploadResourceRowSize = 0;							// 上传堆资源每行的大小 (单位：字节)
	UINT UploadResourceSize = 0;							// 上传堆资源的总大小 (单位：字节)

	ComPtr<ID3D12Resource> m_CBVResource;				// 常量缓冲区资源
	struct CBuffer								// 常量缓冲结构体
	{
		XMFLOAT4X4 MVPMatrix;		// MVP 矩阵，用于将顶点数据从顶点空间变换到齐次裁剪空间
	};

	CBuffer* MVPBuffer = nullptr;	// 常量缓冲结构体指针，里面存储的是 MVP 矩阵信息，下文 Map 后指针会指向 CBVResource 的地址
	
	XMVECTOR EyePosition = XMVectorSet(4, 3, 4, 1);					// 摄像机在世界空间下的位置
	XMVECTOR FocusPosition = XMVectorSet(0, 1, 1, 1);				// 摄像机在世界空间下观察的焦点位置
	XMVECTOR UpDirection = XMVectorSet(0, 1, 0, 0);					// 世界空间垂直向上的向量
	XMMATRIX ModelMatrix;											// 模型矩阵，模型空间 -> 世界空间
	XMMATRIX ViewMatrix;											// 观察矩阵，世界空间 -> 观察空间
	XMMATRIX ProjectionMatrix;										// 投影矩阵，观察空间 -> 齐次裁剪空间

	ComPtr<ID3D12RootSignature> m_RootSignature; // 根签名
	ComPtr<ID3D12PipelineState> m_PipelineStateObject; // 管线状态对象

	ComPtr<ID3D12Resource> m_VertexResource; // 顶点缓冲区资源
	struct VERTEX											// 顶点数据结构体
	{
		XMFLOAT4 position;									// 顶点位置
		XMFLOAT2 texcoordUV;								// 顶点纹理坐标
	};

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView; // 顶点缓冲区视图
	ComPtr<ID3D12Resource> m_IndexResource; // 索引缓冲区资源
	D3D12_INDEX_BUFFER_VIEW IndexBufferView{}; // 索引缓冲区视图

	// 视口
	D3D12_VIEWPORT viewPort = D3D12_VIEWPORT{ 0, 0, float(WindowWidth), float(WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	// 裁剪矩形
	D3D12_RECT ScissorRect = D3D12_RECT{ 0, 0, WindowWidth, WindowHeight };


public:
	
	void InitWindow(HINSTANCE hins)
	{
		
		WNDCLASS wc = {};					// 用于记录窗口类信息的结构体
		wc.hInstance = hins;				// 窗口类需要一个应用程序的实例句柄 hinstance
		wc.lpfnWndProc = CallBackFunc;		// 窗口类需要一个回调函数，用于处理窗口产生的消息
		wc.lpszClassName = L"DX12 Game";	// 窗口类的名称

		RegisterClass(&wc);					// 注册窗口类，将窗口类录入到操作系统中

		// 使用上文的窗口类创建窗口
		m_hwnd = CreateWindow(wc.lpszClassName, L"DX12画钻石原矿", WS_SYSMENU | WS_OVERLAPPED,
			10, 10, WindowWidth, WindowHeight,
			NULL, NULL, hins, NULL);

		// 因为指定了窗口大小不可变的 WS_SYSMENU 和 WS_OVERLAPPED，应用不会自动显示窗口，需要使用 ShowWindow 强制显示窗口
		ShowWindow(m_hwnd, SW_SHOW);
	}

	void CreateDebugDevice()
	{
		::CoInitialize(nullptr);	// 注意这里！DX12 的所有设备接口都是基于 COM 接口的，我们需要先全部初始化为 nullptr
#if defined(_DEBUG)		// 如果是 Debug 模式下编译，就执行下面的代码

		// 获取调试层设备接口
		D3D12GetDebugInterface(IID_PPV_ARGS(&m_D3D12DebugDevice));
		// 开启调试层
		m_D3D12DebugDevice->EnableDebugLayer();
		// 开启调试层后，创建 DXGI 工厂也需要 Debug Flag
		m_DXGICreateFactoryFlag = DXGI_CREATE_FACTORY_DEBUG;

#endif
	
	}

	bool CreateDevice()
	{
		CreateDXGIFactory2(m_DXGICreateFactoryFlag, IID_PPV_ARGS(&m_DXGIFactory));
		const D3D_FEATURE_LEVEL dx12SupportLevel[] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		for (int i = 0;m_DXGIFactory->EnumAdapters1(i, &m_DXGIAdapter) != DXGI_ERROR_NOT_FOUND; i++)
		{
			for (const auto& level : dx12SupportLevel)
			{
				if (SUCCEEDED(D3D12CreateDevice(m_DXGIAdapter.Get(), level, IID_PPV_ARGS(&m_D3D12Device))))
				{
					DXGI_ADAPTER_DESC1 adap = {};
					m_DXGIAdapter->GetDesc1(&adap);
					OutputDebugStringW(adap.Description);
					return true;
				}
			}
		}
		if (m_D3D12Device == nullptr)
		{
			MessageBox(NULL, L"找不到任何能支持 DX12 的显卡，请升级电脑上的硬件！", L"错误", MB_OK | MB_ICONERROR);
			return false;
		}


	}

	void CreateCommandComponents()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 直接命令列表，可以执行所有 GPU 命令
		m_D3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));	
		m_D3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));	
		m_D3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList));
		m_CommandList->Close(); // 创建出来的命令列表默认是打开的，我们需要先关闭，等到需要用的时候再打开


	}
	void CreateRenderTarget()
	{
		D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc{};
		RTVHeapDesc.NumDescriptors = 3; // 三个渲染目标
		RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RTV 描述符堆
		m_D3D12Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_RTVHeap));	

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = 3; // 三个缓冲区
		swapChainDesc.Width = WindowWidth; // 宽度
		swapChainDesc.Height = WindowHeight; // 高度
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32位颜色格式
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 交换链的缓冲区用作渲染目标
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 翻转丢弃模式
		swapChainDesc.SampleDesc.Count = 1; // 多重采样设置为 1，表示不使用多重采样
		ComPtr<IDXGISwapChain1> _temp_swapchain;

		m_DXGIFactory->CreateSwapChainForHwnd(
			m_CommandQueue.Get(),	// 交换链需要绑定到一个命令队列上
			m_hwnd,					// 交换链需要绑定到一个窗口上
			&swapChainDesc,			// 交换链描述符
			nullptr, nullptr, &_temp_swapchain
		);
		_temp_swapchain.As(&m_DXGISwapChain); // 查询 IDXGISwapChain4 接口
		RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart(); // 获取 RTV 描述符堆的起始句柄
		RTVDescriptorSize = m_D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // 获取 RTV 描述符的大小
		for (int i = 0;i < 3;i++)
		{
			m_DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i]));
			m_D3D12Device->CreateRenderTargetView(m_RenderTarget[i].Get(), nullptr, RTVHandle);
			//RTVHandle主要是起到一个指针的作用
			RTVHandle.ptr += RTVDescriptorSize; // 指向下一个描述符

		}
	}

	void CreateFenceAndBarrier()
	{
		RenderEvent = CreateEvent(nullptr, FALSE, TRUE, nullptr);
		m_D3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
		//资源屏障后续用到再处理
				// 设置资源屏障
		//// beg_barrier 起始屏障：Present 呈现状态 -> Render Target 渲染目标状态
		//beg_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;					// 指定类型为转换屏障		
		//beg_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		//beg_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		//// end_barrier 终止屏障：Render Target 渲染目标状态 -> Present 呈现状态
		//end_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		//end_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		//end_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	}

	bool LoadTextureFromFile()
	{
		if (m_WICFactory == nullptr) CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_WICFactory));

		// 创建图片解码器，并将图片读入到解码器中
		HRESULT hr = m_WICFactory->CreateDecoderFromFilename(TextureFilename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_WICBitmapDecoder);

		std::wostringstream output_str;		// 用于格式化字符串
		switch (hr)
		{
		case S_OK: break;	// 解码成功，直接 break 进入下一步即可

		case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):	// 文件找不到
			output_str << L"找不到文件 " << TextureFilename << L" ！请检查文件路径是否有误！";
			MessageBox(NULL, output_str.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
			return false;

		case HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT):	// 文件句柄正在被另一个应用进程占用
			output_str << L"文件 " << TextureFilename << L" 已经被另一个应用进程打开并占用了！请先关闭那个应用进程！";
			MessageBox(NULL, output_str.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
			return false;

		case WINCODEC_ERR_COMPONENTNOTFOUND:			// 找不到可解码的组件，说明这不是有效的图像文件
			output_str << L"文件 " << TextureFilename << L" 不是有效的图像文件，无法解码！请检查文件是否为图像文件！";
			MessageBox(NULL, output_str.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
			return false;

		default:			// 发生其他未知错误
			output_str << L"文件 " << TextureFilename << L" 解码失败！发生了其他错误，错误码：" << hr << L" ，请查阅微软官方文档。";
			MessageBox(NULL, output_str.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
			return false;
		}

		// 获取图片数据的第一帧，这个 GetFrame 可以用于 gif 这种多帧动图
		m_WICBitmapDecoder->GetFrame(0, &m_WICBitmapDecodeFrame);


		// 获取图片格式，并将它转化为 DX12 能接受的纹理格式
		// 如果碰到格式无法支持的错误，可以用微软提供的 画图3D 来转换，强力推荐!
		WICPixelFormatGUID SourceFormat = {};				// 源图格式
		GUID TargetFormat = {};								// 目标格式

		m_WICBitmapDecodeFrame->GetPixelFormat(&SourceFormat);						// 获取源图格式

		if (DX12TextureHelper::GetTargetPixelFormat(&SourceFormat, &TargetFormat))	// 获取目标格式
		{
			TextureFormat = DX12TextureHelper::GetDXGIFormatFromPixelFormat(&TargetFormat);	// 获取 DX12 支持的格式
		}
		else	// 如果没有可支持的目标格式
		{
			::MessageBox(NULL, L"此纹理不受支持!", L"提示", MB_OK);
			return false;
		}

		// 获取目标格式后，将纹理转换为目标格式，使其能被 DX12 使用
		m_WICFactory->CreateFormatConverter(&m_WICFormatConverter);		// 创建图片转换器
		// 初始化转换器，实际上是把位图进行了转换
		m_WICFormatConverter->Initialize(m_WICBitmapDecodeFrame.Get(), TargetFormat, WICBitmapDitherTypeNone,
			nullptr, 0.0f, WICBitmapPaletteTypeCustom);
		// 将位图数据继承到 WIC 位图资源，我们要在这个 WIC 位图资源上获取信息
		m_WICFormatConverter.As(&m_WICBitmapSource);



		m_WICBitmapSource->GetSize(&TextureWidth, &TextureHeight);		// 获取纹理宽高

		ComPtr<IWICComponentInfo> _temp_WICComponentInfo = {};			// 用于获取 BitsPerPixel 纹理图像深度
		ComPtr<IWICPixelFormatInfo> _temp_WICPixelInfo = {};			// 用于获取 BitsPerPixel 纹理图像深度
		m_WICFactory->CreateComponentInfo(TargetFormat, &_temp_WICComponentInfo);
		_temp_WICComponentInfo.As(&_temp_WICPixelInfo);
		_temp_WICPixelInfo->GetBitsPerPixel(&BitsPerPixel);				// 获取 BitsPerPixel 图像深度

		return true;
	}

	void CreateSRVHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc{};
		SRVHeapDesc.NumDescriptors = 1; // 一个纹理
		SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // SRV 描述符堆
		SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 这个描述符堆需要在着色器中可见
		m_D3D12Device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(&m_SRVHeap));

	}

	inline UINT Ceil(UINT A, UINT B)
	{
		return (A + B - 1) / B;
	}

	void CreateUploadAndDefaultResource()
	{
		BytePerRowSize = TextureWidth * BitsPerPixel / 8; // 计算出每行的字节数
		TextureSize = BytePerRowSize * TextureHeight; // 计算出总的纹理数据大小
		// 计算出上传堆资源每行的大小，必须是 256 字节对齐的
		UploadResourceRowSize = Ceil(BytePerRowSize, 256) * 256;

		UploadResourceSize = UploadResourceRowSize * (TextureHeight - 1) + BytePerRowSize;

		CD3DX12_RESOURCE_DESC UploadResourceDesc{};
		UploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // 资源维度是 Buffer
		UploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // 行主序
		UploadResourceDesc.Width = UploadResourceSize; // 资源大小
		UploadResourceDesc.Height = 1; // 高度为 1
		UploadResourceDesc.DepthOrArraySize = 1; // 深度或数组大小为 1
		UploadResourceDesc.MipLevels = 1; // Mip 级别为 1
		UploadResourceDesc.SampleDesc.Count = 1; // 多重采样为 1

		CD3DX12_HEAP_PROPERTIES UploadHeapDesc{ D3D12_HEAP_TYPE_UPLOAD }; // 上传堆属性
		m_D3D12Device->CreateCommittedResource(&UploadHeapDesc, D3D12_HEAP_FLAG_NONE, &UploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_UploadTextureResource));

		CD3DX12_RESOURCE_DESC DefaultResourceDesc{};
		DefaultResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 资源维度是 2D 纹理
		DefaultResourceDesc.Alignment = 0; // 默认对齐
		DefaultResourceDesc.Width = TextureWidth; // 纹理宽度
		DefaultResourceDesc.Height = TextureHeight; // 纹理高度
		DefaultResourceDesc.DepthOrArraySize = 1; // 深度或数组大小为 1
		DefaultResourceDesc.MipLevels = 1; // Mip 级别为 1
		DefaultResourceDesc.Format = TextureFormat; // 纹理格式
		DefaultResourceDesc.SampleDesc.Count = 1; // 多重采样为 1
		DefaultResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // 纹理布局未知

		D3D12_HEAP_PROPERTIES DefaultHeapDesc{ D3D12_HEAP_TYPE_DEFAULT };
		
		m_D3D12Device->CreateCommittedResource(&DefaultHeapDesc, D3D12_HEAP_FLAG_NONE, &DefaultResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_DefaultTextureResource));

	}
	void CopyTextureDataToDefaultResource()
	{
		// 用于暂时存储纹理数据的指针，这里要用 malloc 分配空间
		//TextureData = (BYTE*)malloc(TextureSize);
		//unique_ptr<BYTE> TextureData = nullptr; // 纹理数据
		TextureData = std::make_unique<BYTE[]>(TextureSize);
		// 将整块纹理数据读到 TextureData 中，方便后文的 memcpy 复制操作
		m_WICBitmapSource->CopyPixels(nullptr, BytePerRowSize, TextureSize, TextureData.get());
		BYTE* TextureDataPtr = TextureData.get(); // 保存初始指针，后面释放内存时要用
		// 用于传递资源的指针
		BYTE* TransferPointer = nullptr;

		D3D12_SUBRESOURCE_DATA textureData{};
		textureData.pData = TextureData.get();					// 指向纹理数据的指针
		textureData.RowPitch = BytePerRowSize;			//为对齐后每行的大小
		textureData.SlicePitch = TextureSize;					//童谣为对齐后的整个纹理数据大小，单位：字节

		m_CommandAllocator->Reset();
		m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_DefaultTextureResource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		m_CommandList->ResourceBarrier(1, &barrier);
		UpdateSubresources<1>(
			m_CommandList.Get(),
			m_DefaultTextureResource.Get(),
			m_UploadTextureResource.Get(),
			0, 0, 1, &textureData
		);//内部是先将数据复制到上传堆，然后再从上传堆复制到默认堆，自动调用CopyTextureRegion拷贝上上传堆到默认堆
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_DefaultTextureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		m_CommandList->ResourceBarrier(1, &barrier);
		m_CommandList->Close();
		// 用于传递命令用的临时 ID3D12CommandList 数组
		ID3D12CommandList* _temp_cmdlists[] = { m_CommandList.Get() };

		// 提交复制命令！GPU 开始复制！
		m_CommandQueue->ExecuteCommandLists(1, _temp_cmdlists);

		// 将围栏预定值设定为下一帧，注意复制资源也需要围栏等待，否则会发生资源冲突
		FenceValue++;
		// 在命令队列 (命令队列在 GPU 端) 设置围栏预定值，此命令会加入到命令队列中
		// 命令队列执行到这里会修改围栏值，表示复制已完成，"击中"围栏
		m_CommandQueue->Signal(m_Fence.Get(), FenceValue);
		// 设置围栏的预定事件，当复制完成时，围栏被"击中"，激发预定事件，将事件由无信号状态转换成有信号状态
		m_Fence->SetEventOnCompletion(FenceValue, RenderEvent);

	}

	void CreateSRV()
	{
		CD3DX12_SHADER_RESOURCE_VIEW_DESC SRVDescriptorDesc{};
		SRVDescriptorDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // SRV 视图维度是 2D 纹理
		SRVDescriptorDesc.Format = TextureFormat; // 纹理格式
		SRVDescriptorDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // 默认映射
		SRVDescriptorDesc.Texture2D.MipLevels = 1; // Mip 级别为 1
		SRV_CPUHandle = m_SRVHeap->GetCPUDescriptorHandleForHeapStart(); // 获取 SRV 描述符堆的起始句柄
		m_D3D12Device->CreateShaderResourceView(m_DefaultTextureResource.Get(), &SRVDescriptorDesc, SRV_CPUHandle);
		//设置跟参数时需要用到SRV的GPU句柄来绑定跟参数
		SRV_GPUHandle = m_SRVHeap->GetGPUDescriptorHandleForHeapStart(); // 获取 SRV 描述符堆的起始 GPU 句柄

	}

	// 创建 Constant Buffer Resource 常量缓冲资源，常量缓冲是一块预先分配的高速显存，用于存储每一帧都要变换的资源，这里我们要存储 MVP 矩阵
	void CreateCBVBufferResource()
	{

		// 常量资源宽度，这里填整个结构体的大小。注意！硬件要求，常量缓冲需要 256 字节对齐！所以这里要进行 Ceil 向上取整，进行内存对齐！
		UINT CBufferWidth = Ceil(sizeof(CBuffer), 256) * 256;//此处就传MVP矩阵
		//CBV通常是放在上传堆中，因为它需要频繁更新，所以不会上传到默认堆中
		CD3DX12_RESOURCE_DESC CBVResourceDesc{};
		CBVResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // 上传堆资源都是缓冲
		CBVResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; 	// 上传堆资源都是按行存储数据的 (一维线性存储)
		CBVResourceDesc.Width = CBufferWidth;				// 资源宽度
		CBVResourceDesc.Height = 1;							// 资源高度为 1
		CBVResourceDesc.Format = DXGI_FORMAT_UNKNOWN;		// 上传堆资源的格式必须为 DXGI_FORMAT_UNKNOWN
		CBVResourceDesc.DepthOrArraySize = 1;				// 资源深度或数组大小为 1
		CBVResourceDesc.MipLevels = 1;						// Mip 级别为 1
		CBVResourceDesc.SampleDesc.Count = 1;				// 多重采样为 1

		CD3DX12_HEAP_PROPERTIES CBVHeapDesc{ D3D12_HEAP_TYPE_UPLOAD }; // 上传堆属性
		m_D3D12Device->CreateCommittedResource(&CBVHeapDesc, D3D12_HEAP_FLAG_NONE, &CBVResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_CBVResource));	
		// 将常量缓冲资源映射到 CPU 可访问的内存地址
		m_CBVResource->Map(0, nullptr, reinterpret_cast<void**>(&MVPBuffer));
	}


	// 创建根签名
	void CreateRootSignature()
	{
		ComPtr<ID3DBlob> SignaatureBlob = nullptr; // 根签名的二进制代码
		ComPtr<ID3DBlob> ErrorBlob = nullptr; // 用于存储错误信息的 Blob
		CD3DX12_ROOT_PARAMETER RootParameters[2]{};

		CD3DX12_DESCRIPTOR_RANGE SRVDescriptorRangeDesc{}; // SRV 描述符表
		SRVDescriptorRangeDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // 描述符类型是 SRV
		SRVDescriptorRangeDesc.NumDescriptors = 1; // 一个纹理
		SRVDescriptorRangeDesc.BaseShaderRegister = 0; // 从 t0 寄存器开始
		SRVDescriptorRangeDesc.RegisterSpace = 0; // 寄存器空间 0
		SRVDescriptorRangeDesc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // 自动计算偏移量
		RootParameters[0].InitAsDescriptorTable(1, &SRVDescriptorRangeDesc, D3D12_SHADER_VISIBILITY_PIXEL); // 像素着色器可见
		//RootParameters[1].InitAsConstantBufferView(0); // b0 寄存器
		//CD3DX12_ROOT_DESCRIPTOR CBVRootDescriptorDesc{};
		//CBVRootDescriptorDesc.RegisterSpace = 0; // 寄存器空间 0
		//CBVRootDescriptorDesc.ShaderRegister = 0; // b0 寄存器
		//此处直接使用的是根描述符，将资源信息直接内联到了根参数里了，这样就不需要创建常量描述符&描述符堆了
		//还要说明的是，此处只是说明要常量缓冲区的数据内联到根参数里，但是还没有这么做，所以后续任然要使用
		//ID3D12GraphicsCommandList::SetDescriptorHeaps将元数据传入到根参数形成根描述符
		RootParameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // 顶点着色器可见

		CD3DX12_STATIC_SAMPLER_DESC StaticSamplerDesc{}; // 静态采样器
		StaticSamplerDesc.ShaderRegister = 0; // s0 寄存器
		StaticSamplerDesc.RegisterSpace = 0; // 寄存器空间 
		StaticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 像素着色器可见	
		StaticSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		StaticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		StaticSamplerDesc.MipLODBias = 0; // Mip 级别偏移为 0
		StaticSamplerDesc.MaxAnisotropy = 0; // 各向异性过滤等级为 0
		StaticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比较函数
		StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK; // 边界颜色为不透明黑色

		CD3DX12_ROOT_SIGNATURE_DESC rootsignatureDesc{};
		rootsignatureDesc.Init(_countof(RootParameters), RootParameters, 1, &StaticSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		D3D12SerializeRootSignature(&rootsignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &SignaatureBlob, &ErrorBlob);
		if (ErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
		}
		m_D3D12Device->CreateRootSignature(0, SignaatureBlob->GetBufferPointer(), SignaatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
	}

	// 创建渲染管线状态对象 (Pipeline State Object, PSO)
	void CreatePSO()
	{
		//输入布局是专门用来告诉 GPU 我们的顶点数据长什么样子的
		D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc{};
		D3D12_INPUT_LAYOUT_DESC InputLayoutDesc{}; // 输入布局描述
		D3D12_INPUT_ELEMENT_DESC InputElementDesc[2]{
			"POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0,
			"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		};

		InputLayoutDesc.NumElements = _countof(InputElementDesc); // 输入元素数量
		InputLayoutDesc.pInputElementDescs = InputElementDesc; // 输入元素描述数组
		PSODesc.InputLayout = InputLayoutDesc; // 输入布局



		ComPtr<ID3DBlob> VertexShaderBlob = nullptr; // 顶点着色器的二进制代码
		ComPtr<ID3DBlob> PixelShaderBlob = nullptr; // 像素着色器的二进制代码
		ComPtr<ID3DBlob> ErrorBlob = nullptr; // 用于存储错误信息的 Blob
		D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\Block/shader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", NULL, NULL, &VertexShaderBlob, &ErrorBlob);
		if (ErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
			OutputDebugStringA("\n");
		}
		// 编译像素着色器 Pixel Shader
		D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\Block/shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", NULL, NULL, &PixelShaderBlob, &ErrorBlob);
		if (ErrorBlob)		// 如果着色器编译出错，ErrorBlob 可以提供报错信息
		{
			OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
			OutputDebugStringA("\n");
		}

		PSODesc.VS.pShaderBytecode = VertexShaderBlob->GetBufferPointer(); // 顶点着色器代码
		PSODesc.VS.BytecodeLength = VertexShaderBlob->GetBufferSize(); // 顶点着色器代码大小
		PSODesc.PS.pShaderBytecode = PixelShaderBlob->GetBufferPointer(); // 像素着色器代码
		PSODesc.PS.BytecodeLength = PixelShaderBlob->GetBufferSize(); // 像素着色器代码大小

		// Rasterizer 光栅化
		PSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		PSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

		// 第一次设置根签名！本次设置是将根签名与 PSO 绑定，设置渲染管线的输入参数状态
		PSODesc.pRootSignature = m_RootSignature.Get();

		PSODesc.pRootSignature = m_RootSignature.Get(); // 根签名
		PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // 图元拓扑类型是三角形
		PSODesc.NumRenderTargets = 1; // 一个渲染目标
		PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 渲染目标格式
		PSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		PSODesc.SampleDesc.Count = 1; // 多重采样为 1
		PSODesc.SampleMask = UINT_MAX; // 采样掩码
		m_D3D12Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&m_PipelineStateObject));

	
	}


	void CreateVertexResource()
	{
#pragma region 定义顶点结构体
		// CPU 高速缓存上的顶点信息数组，注意 DirectX 使用的是左手坐标系，写顶点信息时，请比一比你的左手！
		VERTEX vertexs[24] =
		{
			// 正面
			{{0,2,0,1},{0,0}},
			{{2,2,0,1},{1,0}},
			{{2,0,0,1},{1,1}},
			{{0,0,0,1},{0,1}},

			// 背面
			{{2,2,2,1},{0,0}},
			{{0,2,2,1},{1,0}},
			{{0,0,2,1},{1,1}},
			{{2,0,2,1},{0,1}},

			// 左面
			{{0,2,2,1},{0,0}},
			{{0,2,0,1},{1,0}},
			{{0,0,0,1},{1,1}},
			{{0,0,2,1},{0,1}},

			// 右面
			{{2,2,0,1},{0,0}},
			{{2,2,2,1},{1,0}},
			{{2,0,2,1},{1,1}},
			{{2,0,0,1},{0,1}},

			// 上面
			{{0,2,2,1},{0,0}},
			{{2,2,2,1},{1,0}},
			{{2,2,0,1},{1,1}},
			{{0,2,0,1},{0,1}},

			// 下面
			{{0,0,0,1},{0,0}},
			{{2,0,0,1},{1,0}},
			{{2,0,2,1},{1,1}},
			{{0,0,2,1},{0,1}}
		};
#pragma endregion

		D3D12_RESOURCE_DESC VertexDesc{};
		VertexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // 资源维度是 Buffer
		VertexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // 行主序
		VertexDesc.Width = sizeof(vertexs); // 资源大小
		VertexDesc.Height = 1; // 高度为 1
		VertexDesc.Format = DXGI_FORMAT_UNKNOWN; // 格式未知
		VertexDesc.DepthOrArraySize = 1; // 深度或数组大小为 1
		VertexDesc.MipLevels = 1; // Mip 级别为 1
		VertexDesc.SampleDesc.Count = 1; // 多重采样设置为 1，表示不使用多重采样

		CD3DX12_HEAP_PROPERTIES UploadHeapDesc{ D3D12_HEAP_TYPE_UPLOAD }; // 上传堆属性

		// 创建资源，CreateCommittedResource 会为资源自动创建一个等大小的隐式堆，这个隐式堆的所有权由操作系统管理，开发者不可控制
		m_D3D12Device->CreateCommittedResource(&UploadHeapDesc, D3D12_HEAP_FLAG_NONE, &VertexDesc, D3D12_RESOURCE_STATE_GENERIC_READ
			, nullptr, IID_PPV_ARGS(&m_VertexResource));
		BYTE* TransferPointer = nullptr;
		m_VertexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransferPointer));

		memcpy(TransferPointer, vertexs, sizeof(vertexs)); // 复制顶点数据到资源中
		m_VertexResource->Unmap(0, nullptr); // 解除映射
		// 初始化顶点缓冲视图
		VertexBufferView.BufferLocation = m_VertexResource->GetGPUVirtualAddress(); // 顶点缓冲的 GPU 虚拟地址
		VertexBufferView.StrideInBytes = sizeof(VERTEX); // 每个顶点的大小
		VertexBufferView.SizeInBytes = sizeof(vertexs); // 顶点缓冲的整个大小
	}
	void CreateIndexResource()
	{
		// 顶点索引数组，注意这里的 UINT == UINT32，后面填的格式 (步长) 必须是 DXGI_FORMAT_R32_UINT，否则会出错
		UINT IndexArray[36] =
		{
			// 正面
			0,1,2,0,2,3,
			// 背面
			4,5,6,4,6,7,
			// 左面
			8,9,10,8,10,11,
			// 右面
			12,13,14,12,14,15,
			// 上面
			16,17,18,16,18,19,
			// 下面
			20,21,22,20,22,23
		};
		CD3DX12_RESOURCE_DESC IndexResDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(IndexArray), D3D12_RESOURCE_FLAG_NONE, 0);
		CD3DX12_HEAP_PROPERTIES UploadHeapDesc{ D3D12_HEAP_TYPE_UPLOAD };

		m_D3D12Device->CreateCommittedResource(&UploadHeapDesc, D3D12_HEAP_FLAG_NONE, &IndexResDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexResource));

		BYTE* TransferPointer = nullptr;
		m_IndexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransferPointer));
		memcpy(TransferPointer, IndexArray, sizeof(IndexArray));
		m_IndexResource->Unmap(0, nullptr);

		IndexBufferView.BufferLocation = m_IndexResource->GetGPUVirtualAddress();
		IndexBufferView.SizeInBytes = sizeof(IndexArray);
		IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	}

	void UpdateConstantBuffer()
	{
		ModelMatrix = XMMatrixRotationY(30.0f);
		ViewMatrix = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
		ProjectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, 4.0 / 3, 0.1, 1000);
		XMStoreFloat4x4(&MVPBuffer->MVPMatrix, ModelMatrix * ViewMatrix * ProjectionMatrix);
	}

	void Render()
	{
		UpdateConstantBuffer();

		RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
		FrameIndex = m_DXGISwapChain->GetCurrentBackBufferIndex();
		RTVHandle.ptr += FrameIndex * RTVDescriptorSize;

		m_CommandAllocator->Reset();
		m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_RenderTarget[FrameIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		m_CommandList->ResourceBarrier(1, &barrier);

		m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
		m_CommandList->SetPipelineState(m_PipelineStateObject.Get());
		// 设置视口 (光栅化阶段)，用于光栅化里的屏幕映射
		m_CommandList->RSSetViewports(1, &viewPort);
		// 设置裁剪矩形 (光栅化阶段)
		m_CommandList->RSSetScissorRects(1, &ScissorRect);

		m_CommandList->OMSetRenderTargets(1, &RTVHandle, false, nullptr);
		// 清空当前渲染目标的背景为天蓝色
		m_CommandList->ClearRenderTargetView(RTVHandle, DirectX::Colors::SkyBlue, 0, nullptr);

		ID3D12DescriptorHeap* _temp_DescriptorHeaps[]{ m_SRVHeap.Get() };
		m_CommandList->SetDescriptorHeaps(1, _temp_DescriptorHeaps);
		m_CommandList->SetGraphicsRootDescriptorTable(0, SRV_GPUHandle);

		m_CommandList->SetGraphicsRootConstantBufferView(1, m_CBVResource->GetGPUVirtualAddress());

		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 设置 VBV 顶点缓冲描述符 (输入装配阶段) 
		m_CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);

		// 设置 IBV 索引缓冲描述符 (输入装配阶段) 
		m_CommandList->IASetIndexBuffer(&IndexBufferView);

		// Draw Call! 绘制方块
		m_CommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_RenderTarget[FrameIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);
		m_CommandList->ResourceBarrier(1, &barrier);
		m_CommandList->Close();
		ID3D12CommandList* _temp_cmdlists[]{ m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(1, _temp_cmdlists);
		m_DXGISwapChain->Present(1, 0);//阻塞等待1次垂直同步

		FenceValue++;
		m_CommandQueue->Signal(m_Fence.Get(), FenceValue);
		m_Fence->SetEventOnCompletion(FenceValue, RenderEvent);
	}

	// 渲染循环
	void RenderLoop()
	{
		bool isExit = false;	// 是否退出
		MSG msg = {};			// 消息结构体

		while (isExit != true)
		{
			// MsgWaitForMultipleObjects 用于多个线程的无阻塞等待，返回值是激发事件 (线程) 的 ID
			// 经过该函数后 RenderEvent 也会自动重置为无信号状态，因为我们创建事件的时候指定了第二个参数为 false
			DWORD ActiveEvent = ::MsgWaitForMultipleObjects(1, &RenderEvent, false, INFINITE, QS_ALLINPUT);

			switch (ActiveEvent - WAIT_OBJECT_0)
			{
			case 0:				// ActiveEvent 是 0，说明渲染事件已经完成了，进行下一次渲染
				Render();
				break;

			case 1:				// ActiveEvent 是 1，说明渲染事件未完成，CPU 主线程同时处理窗口消息，防止界面假死
				// 查看消息队列是否有消息，如果有就获取。 PM_REMOVE 表示获取完消息，就立刻将该消息从消息队列中移除
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					// 如果程序没有收到退出消息，就向操作系统发出派发消息的命令
					if (msg.message != WM_QUIT)
					{
						TranslateMessage(&msg);					// 翻译消息，将虚拟按键值转换为对应的 ASCII 码 (后文会讲)
						DispatchMessage(&msg);					// 派发消息，通知操作系统调用回调函数处理消息
					}
					else
					{
						isExit = true;							// 收到退出消息，就退出消息循环
					}
				}
				break;

			case WAIT_TIMEOUT:	// 渲染超时
				break;
			}
		}
	}

	// 回调函数
	static LRESULT CALLBACK CallBackFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// 用 switch 将第二个参数分流，每个 case 分别对应一个窗口消息
		switch (msg)
		{
		case WM_DESTROY:			// 窗口被销毁 (当按下右上角 X 关闭窗口时)
			PostQuitMessage(0);		// 向操作系统发出退出请求 (WM_QUIT)，结束消息循环
			break;

			// 如果接收到其他消息，直接默认返回整个窗口
		default: return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		return 0;	// 注意这里！
	}
	// 运行窗口
	static void Run(HINSTANCE hins)
	{
		DX12Engine engine;
		engine.InitWindow(hins);
		engine.CreateDebugDevice();
		engine.CreateDevice();
		engine.CreateCommandComponents();
		engine.CreateRenderTarget();
		engine.CreateFenceAndBarrier();

		engine.LoadTextureFromFile();
		engine.CreateSRVHeap();
		engine.CreateUploadAndDefaultResource();
		engine.CopyTextureDataToDefaultResource();
		engine.CreateSRV();

		engine.CreateCBVBufferResource();

		engine.CreateRootSignature();
		engine.CreatePSO();

		engine.CreateVertexResource();
		engine.CreateIndexResource();

		engine.RenderLoop();
	}
};


// 主函数
int WINAPI WinMain(HINSTANCE hins, HINSTANCE hPrev, LPSTR cmdLine, int cmdShow)
{
	DX12Engine::Run(hins);
}