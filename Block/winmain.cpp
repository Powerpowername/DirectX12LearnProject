#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include "directx/d3dx12.h"
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

	std::wstring TextureFilename = L"../diamond_ore.jpg"; // 纹理文件路径
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
	ComPtr<ID3D12PipelineState> m_PipelineState; // 管线状态对象

	ComPtr<ID3D12Resource> m_VertexBuffer; // 顶点缓冲区资源
	struct VERTEX											// 顶点数据结构体
	{
		XMFLOAT4 position;									// 顶点位置
		XMFLOAT2 texcoordUV;								// 顶点纹理坐标
	};

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView; // 顶点缓冲区视图
	ComPtr<ID3D12Resource> m_IndexBuffer; // 索引缓冲区资源
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

		//m_UploadTextureResource->Map(0, nullptr, reinterpret_cast<void**>(&TransferPointer));

		//for (int i = 0;i < TextureHeight;i++)
		//{
		//	memcpy(TransferPointer, TextureDataPtr, BytePerRowSize); // 复制一行数据

		//	TextureDataPtr += BytePerRowSize; // 源数据指针下移一行
		//	TransferPointer += UploadResourceRowSize; // 目标数据指针下移一行
		//}

		//m_UploadTextureResource->Unmap(0, nullptr); // 解除映射

		//TextureData -= TextureSize; // 将 TextureData 指针复位
		//free(TextureData); // 释放内存

		//D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint{};
		//D3D12_RESOURCE_DESC DefaultResourceDesc = m_DefaultTextureResource->GetDesc();

		//m_D3D12Device->GetCopyableFootprints(&DefaultResourceDesc, 0, 1, 0, &PlacedFootprint, nullptr, nullptr, nullptr);
		//D3D12_TEXTURE_COPY_LOCATION DstLocation = {};						// 复制目标位置 (默认堆资源) 结构体
		//DstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;		// 纹理复制类型，这里必须指向纹理
		//DstLocation.SubresourceIndex = 0;									// 指定要复制的子资源索引
		//DstLocation.pResource = m_DefaultTextureResource.Get();				// 要复制到的资源
		
		D3D12_SUBRESOURCE_DATA textureData{};
		textureData.pData = TextureData.get();					// 指向纹理数据的指针
		textureData.RowPitch = UploadResourceRowSize;			//为对齐后每行的大小
		textureData.SlicePitch = UploadResourceSize;					//童谣为对齐后的整个纹理数据大小，单位：字节

		m_CommandAllocator->Reset();
		m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DefaultTextureResource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
		UpdateSubresources<1>(
			m_CommandList.Get(),
			m_DefaultTextureResource.Get(),
			m_UploadTextureResource.Get(),
			0, 0, 1, &textureData
		);//内部是先将数据复制到上传堆，然后再从上传堆复制到默认堆，自动调用CopyTextureRegion拷贝上上传堆到默认堆
		m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_DefaultTextureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
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

	void CreateRootSignature()
	{
		ComPtr<ID3DBlob> SignaatureBlob = nullptr; // 根签名的二进制代码

	}
	
	
};
