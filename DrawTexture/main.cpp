// (4) DrawTexture：用 DirectX 12 画一个钻石原矿

#include<Windows.h>			// Windows 窗口编程核心头文件
#include<d3d12.h>			// DX12 核心头文件
#include<dxgi1_6.h>			// DXGI 头文件，用于管理与 DX12 相关联的其他必要设备，如 DXGI 工厂和 交换链
#include<DirectXColors.h>	// DirectX 颜色库
#include<DirectXMath.h>		// DirectX 数学库
#include<d3dcompiler.h>		// DirectX Shader 着色器编译库
#include<wincodec.h>		// WIC 图像处理框架，用于解码编码转换图片文件
#include"directx/d3dx12.h"
#include<wrl.h>				// COM 组件模板库，方便写 DX12 和 DXGI 相关的接口
#include<string>			// C++ 标准 string 库
#include<sstream>			// C++ 字符串流处理库

#pragma comment(lib,"d3d12.lib")			// 链接 DX12 核心 DLL
#pragma comment(lib,"dxgi.lib")				// 链接 DXGI DLL
#pragma comment(lib,"dxguid.lib")			// 链接 DXGI 必要的设备 GUID
#pragma comment(lib,"d3dcompiler.lib")		// 链接 DX12 需要的着色器编译 DLL
#pragma comment(lib,"windowscodecs.lib")	// 链接 WIC DLL

using namespace Microsoft::WRL;
using namespace DirectX;

// 命名空间 DX12TextureHelper 包含了帮助我们转换纹理图片格式的结构体与函数
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


class DX12Engine
{
private:
#pragma region 类成员
	int WindowWidth = 640;		// 窗口宽度
	int WindowHeight = 640;		// 窗口高度
	HWND m_hwnd;				// 窗口句柄

	ComPtr<ID3D12Debug> m_D3D12DebugDevice;					// D3D12 调试层设备
	UINT m_DXGICreateFactoryFlag = NULL;					// 创建 DXGI 工厂时需要用到的标志

	ComPtr<IDXGIFactory5> m_DXGIFactory;					// DXGI 工厂
	ComPtr<IDXGIAdapter1> m_DXGIAdapter;					// 显示适配器 (显卡)
	ComPtr<ID3D12Device4> m_D3D12Device;					// D3D12 核心设备

	ComPtr<ID3D12CommandQueue> m_CommandQueue;				// 命令队列
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;		// 命令分配器
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;		// 命令列表

	ComPtr<IDXGISwapChain3> m_DXGISwapChain;				// DXGI 交换链
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap;					// RTV 描述符堆
	ComPtr<ID3D12Resource> m_RenderTarget[3];				// 渲染目标数组，每一副渲染目标对应一个窗口缓冲区
	CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle;					// RTV 描述符句柄
	UINT RTVDescriptorSize = 0;								// RTV 描述符的大小
	UINT FrameIndex = 0;									// 帧索引，表示当前渲染的第 i 帧 (第 i 个渲染目标)

	ComPtr<ID3D12Fence> m_Fence;							// 围栏
	UINT64 FenceValue = 0;									// 用于围栏等待的围栏值
	HANDLE RenderEvent = NULL;								// GPU 渲染事件
	CD3DX12_RESOURCE_BARRIER beg_barrier = {};				// 渲染开始的资源屏障，呈现 -> 渲染目标
	CD3DX12_RESOURCE_BARRIER end_barrier = {};				// 渲染结束的资源屏障，渲染目标 -> 呈现

	std::wstring TextureFilename = L"diamond_ore.png";		// 纹理文件名 (这里用的是相对路径)
	ComPtr<IWICImagingFactory> m_WICFactory;				// WIC 工厂
	ComPtr<IWICBitmapDecoder> m_WICBitmapDecoder;			// 位图解码器
	ComPtr<IWICBitmapFrameDecode> m_WICBitmapDecodeFrame;	// 由解码器得到的单个位图帧
	ComPtr<IWICFormatConverter> m_WICFormatConverter;		// 位图转换器
	ComPtr<IWICBitmapSource> m_WICBitmapSource;				// WIC 位图资源，用于获取位图数据
	UINT TextureWidth = 0;									// 纹理宽度
	UINT TextureHeight = 0;									// 纹理高度
	UINT BitsPerPixel = 0;									// 图像深度，图片每个像素占用的比特数
	UINT BytePerRowSize = 0;								// 纹理每行数据的真实字节大小，用于读取纹理数据、上传纹理资源
	DXGI_FORMAT TextureFormat = DXGI_FORMAT_UNKNOWN;		// 纹理格式

	ComPtr<ID3D12DescriptorHeap> m_SRVHeap;					// SRV 描述符堆
	CD3DX12_CPU_DESCRIPTOR_HANDLE SRV_CPUHandle;				// SRV 描述符 CPU 句柄
	CD3DX12_GPU_DESCRIPTOR_HANDLE SRV_GPUHandle;				// SRV 描述符 GPU 句柄

	ComPtr<ID3D12Resource> m_UploadTextureResource;			// 上传堆资源，位于共享内存，用于中转纹理资源
	ComPtr<ID3D12Resource> m_DefaultTextureResource;		// 默认堆资源，位于显存，用于放纹理
	UINT TextureSize = 0;									// 纹理的真实大小 (单位：字节)
	UINT UploadResourceRowSize = 0;							// 上传堆资源每行的大小 (单位：字节)
	UINT UploadResourceSize = 0;							// 上传堆资源的总大小 (单位：字节)

	ComPtr<ID3D12RootSignature> m_RootSignature;			// 根签名
	ComPtr<ID3D12PipelineState> m_PipelineStateObject;		// 渲染管线状态


	ComPtr<ID3D12Resource> m_VertexResource;				// 顶点资源
	struct VERTEX											// 顶点数据结构体
	{
		XMFLOAT4 position;									// 顶点位置
		XMFLOAT2 texcoordUV;								// 顶点纹理坐标
	};
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;				// 顶点缓冲描述符

	// 视口
	CD3DX12_RECT ScissorRect = CD3DX12_RECT{ 0, 0, WindowWidth, WindowHeight };
	CD3DX12_VIEWPORT viewPort = CD3DX12_VIEWPORT{ 0.0f, 0.0f, float(WindowWidth), float(WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	// 裁剪矩形
#pragma endregion
public:
	// 初始化窗口
	void InitWindow(HINSTANCE hins);
	// 创建调试层
	void CreateDebugDevice();
	// 创建设备
	bool CreateDevice();
	// 创建命令三件套
	void CreateCommandComponents();
	// 创建渲染目标，将渲染目标设置为窗口
	void CreateRenderTarget();
	// 创建围栏和资源屏障，用于 CPU-GPU 的同步
	void CreateFenceAndBarrier();
	// 加载纹理到内存中
	bool LoadTextureFromFile();//目前不是研究重点，后续再研究
	// 创建 SRV Descriptor Heap 着色器资源描述符堆
	void CreateSRVHeap();
	// 上取整算法，对 A 向上取整，判断至少要多少个长度为 B 的空间才能容纳 A，用于内存对齐
	UINT Ceil(UINT A, UINT B);
	// 创建用于上传的 UploadResource 与用于放纹理的 DefaultResource
	void CreateUploadAndDefaultResource();
	// 向命令队列发出命令，将纹理数据复制到 DefaultResource
	void CopyTextureDataToDefaultResource();
	// 最终创建 SRV 着色器资源描述符，用于描述 DefaultResource 为一块纹理
	void CreateSRV();
};

inline void DX12Engine::InitWindow(HINSTANCE hins)
{
	// 注册窗口类
	WNDCLASS wc = {};
	wc.lpfnWndProc = CallBackFunc;	// 窗口过程函数，这里用默认的
	wc.hInstance = hins;				// 应用程序实例句柄
	wc.lpszClassName = L"DX12WndClass";	// 窗口类名称
	RegisterClass(&wc);
	// 创建窗口
	m_hwnd = CreateWindowEx(
		0,								// 扩展样式
		wc.lpszClassName,				// 窗口类名称
		L"DirectX 12 Draw Texture",		// 窗口标题
		WS_OVERLAPPEDWINDOW,			// 窗口风格，重叠窗口，带标题栏、边框、系统菜单等
		CW_USEDEFAULT, CW_USEDEFAULT,	// 窗口初始位置 (x, y)，系统默认位置
		WindowWidth, WindowHeight,		// 窗口宽度和高度
		NULL,							// 父窗口句柄
		NULL,							// 菜单句柄
		hins,							// 应用程序实例句柄
		NULL							// 额外参数
	);
	if (!m_hwnd)
	{
		MessageBox(NULL, L"创建窗口失败！", L"错误", MB_OK);
		return;
	}
	ShowWindow(m_hwnd, SW_SHOW); // 显示窗口
}



inline void DX12Engine::CreateDebugDevice()
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


// 创建设备
bool DX12Engine::CreateDevice()
{
	// 创建 DXGI 工厂
	CreateDXGIFactory2(m_DXGICreateFactoryFlag, IID_PPV_ARGS(&m_DXGIFactory));

	// DX12 支持的所有功能版本，你的显卡最低需要支持 11.0
	const D3D_FEATURE_LEVEL dx12SupportLevel[] =
	{
		D3D_FEATURE_LEVEL_12_2,		// 12.2
		D3D_FEATURE_LEVEL_12_1,		// 12.1
		D3D_FEATURE_LEVEL_12_0,		// 12.0
		D3D_FEATURE_LEVEL_11_1,		// 11.1
		D3D_FEATURE_LEVEL_11_0		// 11.0
	};


	// 用 EnumAdapters1 先遍历电脑上的每一块显卡
	// 每次调用 EnumAdapters1 找到显卡会自动创建 DXGIAdapter 接口，并返回 S_OK
	// 找不到显卡会返回 ERROR_NOT_FOUND

	for (UINT i = 0; m_DXGIFactory->EnumAdapters1(i, &m_DXGIAdapter) != ERROR_NOT_FOUND; i++)
	{
		// 找到显卡，就创建 D3D12 设备，从高到低遍历所有功能版本，创建成功就跳出
		for (const auto& level : dx12SupportLevel)
		{
			// 创建 D3D12 核心层设备，创建成功就返回 true
			if (SUCCEEDED(D3D12CreateDevice(m_DXGIAdapter.Get(), level, IID_PPV_ARGS(&m_D3D12Device))))
			{
				DXGI_ADAPTER_DESC1 adap = {};
				m_DXGIAdapter->GetDesc1(&adap);
				OutputDebugStringW(adap.Description);
				return true;
			}
		}
	}

	// 如果找不到任何能支持 DX12 的显卡，就退出程序
	if (m_D3D12Device == nullptr)
	{
		MessageBox(NULL, L"找不到任何能支持 DX12 的显卡，请升级电脑上的硬件！", L"错误", MB_OK | MB_ICONERROR);
		return false;
	}
}

void DX12Engine::CreateCommandComponents()
{
	D3D12_COMMAND_QUEUE_DESC  queueDesc{};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 直接命令队列，支持所有类型的命令
	//queueDesc.Flags = CD3DX12_COMMAND_QUEUE_FLAG_NONE; // 战未来，暂时没什么用，没有特殊标志
	m_D3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
	//创建命令分配器，作用是开辟内存为命令列表分配内存
	m_D3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf()));
	// 创建命令列表
	m_D3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
	//创建完命令列表后，默认处于打开状态，我们需要关闭它，等待后续使用
	m_CommandList->Close();
}

void DX12Engine::CreateRenderTarget()
{
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
	RTVHeapDesc.NumDescriptors = 3; // 三个缓冲区
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RTV 描述符堆
	m_D3D12Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_RTVHeap));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 3; // 三个缓冲区
	swapChainDesc.Width = WindowWidth; // 窗口宽度
	swapChainDesc.Height = WindowHeight; // 窗口高度
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 交换链格式，8 位 RGBA 格式
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 用作渲染目标输出
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 翻转丢弃模式
	swapChainDesc.SampleDesc.Count = 1; // 多重采样数量，1 表示不使用多重采样
	
	// 临时低版本交换链接口，用于创建高版本交换链，因为下文的 CreateSwapChainForHwnd 不能直接用于创建高版本接口
	ComPtr<IDXGISwapChain1> _temp_swapchain;

	m_DXGIFactory->CreateSwapChainForHwnd(
		m_CommandQueue.Get(),	// 交换链需要绑定命令队列
		m_hwnd,					// 绑定的窗口句柄
		&swapChainDesc,			// 交换链描述符
		nullptr,				// 全屏显示参数，nullptr 表示用默认值
		nullptr,				// 允许的输出设备，nullptr 表示不限制
		_temp_swapchain.GetAddressOf() // 输出 IDXGISwapChain1 接口
	);
	// 通过 As 方法，将低版本接口的信息传递给高版本接口
	_temp_swapchain.As(&m_DXGISwapChain);
	// 创建完交换链后，我们还需要令 RTV 描述符 指向 渲染目标
	// 因为 ID3D12Resource 本质上只是一块数据，它本身没有对数据用法的说明
	// 我们要让程序知道这块数据是一个渲染目标，就得创建并使用 RTV 描述符

	// 获取 RTV 堆指向首描述符的句柄
	RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	// 获取 RTV 描述符的大小
	RTVDescriptorSize = m_D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (UINT i = 0;i < 3;i++)
	{
		// 从交换链中获取第 i 个窗口缓冲，创建第 i 个 RenderTarget 渲染目标
		m_DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i]));
		m_D3D12Device->CreateRenderTargetView(m_RenderTarget[i].Get(), nullptr, RTVHandle);
		// 每次偏移一个描述符大小，指向下一个描述符
		RTVHandle.Offset(1, RTVDescriptorSize);
	}
}

void DX12Engine::CreateFenceAndBarrier()
{
	// 创建 CPU 上的等待事件
	RenderEvent = CreateEvent(nullptr, false, true, nullptr);
	// 创建围栏，设定初始值为 0
	m_D3D12Device->CreateFence(FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));


	// 设置资源屏障
	// beg_barrier 起始屏障：Present 呈现状态 -> Render Target 渲染目标状态
	beg_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;					// 指定类型为转换屏障	
	beg_barrier.Transition(m_RenderTarget[FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	// end_barrier 结束屏障：Render Target 渲染目标状态 -> Present 呈现状态
	end_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	end_barrier.Transition(m_RenderTarget[FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

bool DX12Engine::LoadTextureFromFile()
{
	// 如果还没创建 WIC 工厂，就新建一个 WIC 工厂实例。注意！WIC 工厂不可以重复释放与创建！
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

void DX12Engine::CreateSRVHeap()
{
	// 创建 SRV 描述符堆
	D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc = {};
	SRVHeapDesc.NumDescriptors = 1;									// 我们只有一副纹理，只需要用一个 SRV 描述符
	SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;		// 描述符堆类型，CBV、SRV、UAV 这三种描述符可以放在同一种描述符堆上
	SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	// 描述符堆标志，Shader-Visible 表示对着色器可见

	// 创建 SRV 描述符堆
	m_D3D12Device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(&m_SRVHeap));
}

// 上取整算法，对 A 向上取整，判断至少要多少个长度为 B 的空间才能容纳 A，用于内存对齐
inline UINT DX12Engine::Ceil(UINT A, UINT B)
{
	return (A + B - 1) / B;
}

inline void DX12Engine::CreateUploadAndDefaultResource()
{
	// 计算纹理每行数据的真实数据大小 (单位：Byte 字节)，因为纹理图片在内存中是线性存储的
	// 想获取纹理的真实大小、正确读取纹理数据、上传到 GPU，必须先获取纹理的 BitsPerPixel 图像深度，因为不同位图深度可能不同
	// 然后再计算每行像素占用的字节，除以 8 是因为 1 Byte = 8 bits
	BytePerRowSize = TextureWidth * BitsPerPixel / 8;
	// 纹理的真实大小 (单位：字节)
	TextureSize = BytePerRowSize * TextureHeight;

	// 上传堆资源每行的大小 (单位：字节)，注意这里要进行 256 字节对齐！
	// 因为 GPU 与 CPU 架构不同，GPU 注重并行计算，注重结构化数据的快速读取，读取数据都是以 256 字节为一组来读的
	// 因此要先要对 BytePerRowSize 进行对齐，判断需要有多少组才能容纳纹理每行像素，不对齐的话数据会读错的。
	UploadResourceRowSize = Ceil(BytePerRowSize, 256) * 256;//这里的实现太麻烦了，可以考虑书上的实现
	// 上传堆资源的总大小 (单位：字节)，分配空间必须只多不少，否则会报 D3D12 MinimumAlloc Error 资源内存创建错误
	// 注意最后一行不用内存对齐 (因为后面没其他行了，不用内存对齐也能正确读取)，所以要 (TextureHeight - 1) 再加 BytePerRowSize
	UploadResourceSize = UploadResourceRowSize * (TextureHeight - 1) + BytePerRowSize;

	// 用于中转纹理的上传堆资源结构体
	CD3DX12_RESOURCE_DESC UploadResourceDesc{};
	UploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;		// 资源类型，上传堆的资源类型都是 buffer 缓冲
	UploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;			// 资源布局，指定资源的存储方式，上传堆的资源都是 row major 按行线性存储
	UploadResourceDesc.Width = UploadResourceSize;						// 资源宽度，上传堆的资源宽度是资源的总大小，注意资源大小必须只多不少
	UploadResourceDesc.Height = 1;										// 资源高度，上传堆仅仅是传递线性资源的，所以高度必须为 1
	UploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;					// 资源格式，上传堆资源的格式必须为 UNKNOWN
	UploadResourceDesc.DepthOrArraySize = 1;							// 资源深度，这个是用于纹理数组和 3D 纹理的，上传堆资源必须为 1
	UploadResourceDesc.MipLevels = 1;									// Mipmap 等级，这个是用于纹理的，上传堆资源必须为 1
	UploadResourceDesc.SampleDesc.Count = 1;							// 资源采样次数，上传堆资源都是填 1

	CD3DX12_HEAP_PROPERTIES UploadHeapDesc{ D3D12_HEAP_TYPE_UPLOAD };
	UploadHeapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;					// 上传堆类型
	// 创建上传堆资源
	m_D3D12Device->CreateCommittedResource(
		&UploadHeapDesc,					// 堆属性
		D3D12_HEAP_FLAG_NONE,
		&UploadResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_UploadTextureResource.GetAddressOf())
	);

	CD3DX12_RESOURCE_DESC DefaultResourceDesc{};
	DefaultResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// 资源类型，这里指定为 Texture2D 2D纹理
	DefaultResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;			// 纹理资源的布局都是 UNKNOWN
	DefaultResourceDesc.Width = TextureWidth;							// 资源宽度，这里填纹理宽度
	DefaultResourceDesc.Height = TextureHeight;							// 资源高度，这里填纹理高度
	DefaultResourceDesc.Format = TextureFormat;							// 资源格式，这里填纹理格式，要和纹理一样
	DefaultResourceDesc.DepthOrArraySize = 1;							// 资源深度，我们只有一副纹理，所以填 1
	DefaultResourceDesc.MipLevels = 1;									// Mipmap 等级，我们暂时不使用 Mipmap，所以填 1
	DefaultResourceDesc.SampleDesc.Count = 1;							// 资源采样次数，这里我们填 1 就行
	
	// 默认堆属性的结构体，默认堆位于显存
	D3D12_HEAP_PROPERTIES DefaultHeapDesc{ D3D12_HEAP_TYPE_DEFAULT };
	m_D3D12Device->CreateCommittedResource(
		&DefaultHeapDesc,
		D3D12_HEAP_FLAG_NONE,
		&DefaultResourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(m_DefaultTextureResource.GetAddressOf())
	);
}


inline void DX12Engine::CopyTextureDataToDefaultResource()
{
	BYTE* TextureData = (BYTE*)malloc(TextureSize);

	// 将整块纹理数据读到 TextureData 中，方便后文的 memcpy 复制操作
	m_WICBitmapSource->CopyPixels(nullptr, BytePerRowSize, TextureSize, TextureData);
	// 用于传递资源的指针
	BYTE* TransferPointer = nullptr;
	// Map 开始映射，Map 方法会得到上传堆资源的地址 (在共享内存上)，传递给指针，这样我们就能通过 memcpy 操作复制数据了
	m_UploadTextureResource->Map(0, nullptr, reinterpret_cast<void**>(&TransferPointer));

	// 逐行复制纹理数据到上传堆资源上
	for (UINT i = 0; i < TextureHeight;i++)
	{
		// 向上传堆资源逐行复制纹理数据 (CPU 高速缓存 -> 共享内存)
		memcpy(TransferPointer, TextureData, BytePerRowSize);
		// 纹理指针偏移到下一行
		TextureData += BytePerRowSize;
		// 上传堆资源指针偏移到下一行，注意这里要按 UploadResourceRowSize 偏移，因为上传堆资源每行是 256 字节对齐的
		TransferPointer += UploadResourceRowSize;
	}

	// Unmap 结束映射，因为我们无法直接读写默认堆资源，需要上传堆复制到那里，在复制之前，我们需要先结束映射，让上传堆处于只读状态
	m_UploadTextureResource->Unmap(0, nullptr);

	TextureData -= TextureSize;		// 纹理资源指针偏移回初始位置
	free(TextureData);				// 释放上文 malloc 分配的空间，后面我们用不到它，不要让它占内存
	
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint = {};						// 资源脚本，用来描述要复制的资源
	D3D12_RESOURCE_DESC DefaultResourceDesc = m_DefaultTextureResource->GetDesc();	// 默认堆资源结构体

	// 获取纹理复制脚本，用于下文的纹理复制
	m_D3D12Device->GetCopyableFootprints(&DefaultResourceDesc, 0, 1, 0, &PlacedFootprint, nullptr, nullptr, nullptr);
	D3D12_TEXTURE_COPY_LOCATION DstLocation = {};						// 复制目标位置 (默认堆资源) 结构体
	DstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;		// 纹理复制类型，这里必须指向纹理
	DstLocation.SubresourceIndex = 0;									// 指定要复制的子资源索引,其实就是MipMap的层级
	DstLocation.pResource = m_DefaultTextureResource.Get();				// 要复制到的资源

	D3D12_TEXTURE_COPY_LOCATION SrcLocation = {};						// 复制源位置 (上传堆资源) 结构体
	SrcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;		// 纹理复制类型，这里必须指向缓冲区
	SrcLocation.PlacedFootprint = PlacedFootprint;						// 指定要复制的资源脚本信息
	SrcLocation.pResource = m_UploadTextureResource.Get();				// 被复制数据的缓冲

	// 复制资源需要使用 GPU 的 CopyEngine 复制引擎，所以需要向命令队列发出复制命令
	m_CommandAllocator->Reset();								// 先重置命令分配器
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);	// 再重置命令列表，复制命令不需要 PSO 状态，所以第二个参数填 nullptr

	// 记录复制资源到默认堆的命令 (共享内存 -> 显存) 
	m_CommandList->CopyTextureRegion(&DstLocation, 0, 0, 0, &SrcLocation, nullptr);
	// 关闭命令列表
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

inline void DX12Engine::CreateSRV()
{
	// SRV 描述符信息结构体
	CD3DX12_SHADER_RESOURCE_VIEW_DESC SRVDescriptorDesc = {};





	//只能用CPU创建描述符，因为GPU没有这个能力（设计成这样的）




}