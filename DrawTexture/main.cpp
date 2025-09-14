// (4) DrawTexture：用 DirectX 12 画一个钻石原矿

#include<Windows.h>			// Windows 窗口编程核心头文件
#include<d3d12.h>			// DX12 核心头文件
#include<dxgi1_6.h>			// DXGI 头文件，用于管理与 DX12 相关联的其他必要设备，如 DXGI 工厂和 交换链
#include<DirectXColors.h>	// DirectX 颜色库
#include<DirectXMath.h>		// DirectX 数学库
#include<d3dcompiler.h>		// DirectX Shader 着色器编译库
#include<wincodec.h>		// WIC 图像处理框架，用于解码编码转换图片文件
#include"d3dx12.h"
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
	CD3DX12_RESOURCE_BARRIER barrier = {};				// 渲染开始的资源屏障，呈现 -> 渲染目标
	//CD3DX12_RESOURCE_BARRIER end_barrier = {};				// 渲染结束的资源屏障，渲染目标 -> 呈现

	std::wstring TextureFilename = L"./diamond_ore.png";		// 纹理文件名 (这里用的是相对路径)
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
	// 创建根签名
	void CreateRootSignature();
	// 创建渲染管线状态对象 (Pipeline State Object, PSO)
	void CreatePSO();
	// 创建顶点资源
	void CreateVertexResource();
	// 渲染
	void Render();
	// 渲染循环
	void RenderLoop();
	// 回调函数
	static LRESULT CALLBACK CallBackFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	// 运行窗口
	static void Run(HINSTANCE hins);

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


	//// 设置资源屏障
	//// beg_barrier 起始屏障：Present 呈现状态 -> Render Target 渲染目标状态
	//beg_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;					// 指定类型为转换屏障	
	//beg_barrier.Transition(m_RenderTarget[FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	//// end_barrier 结束屏障：Render Target 渲染目标状态 -> Present 呈现状态
	//end_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//end_barrier.Transition(m_RenderTarget[FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
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

	CD3DX12_HEAP_PROPERTIES UploadHeapDesc(D3D12_HEAP_TYPE_UPLOAD) ;
	//UploadHeapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;					// 上传堆类型
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
	// SRV 描述符类型，这里我们指定 Texture2D 2D纹理
	SRVDescriptorDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	// SRV 描述符的格式也要填纹理格式
	SRVDescriptorDesc.Format = TextureFormat;
	// 纹理采样后每个纹理像素 RGBA 分量的顺序，D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING 表示纹理采样后分量顺序不改变
	SRVDescriptorDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDescriptorDesc.Texture2D.MipLevels =1;	// 采样纹理的最高 Mipmap 层级，我们没有 Mipmap，所以填 0
	SRVDescriptorDesc.Texture2D.MostDetailedMip = 0; // 采样纹理的最详细 Mipmap 层级，我们没有 Mipmap，所以填 0

	//只能用CPU创建描述符，因为GPU没有这个能力（设计成这样的）,描述符在描述符堆中是连续存储的，这片存储空间在显存中
	SRV_CPUHandle = m_SRVHeap->GetCPUDescriptorHandleForHeapStart(); // 获取 SRV 描述符堆指向首描述符的 CPU 句柄
	// 创建 SRV 描述符，描述 DefaultResource 这块资源为一块纹理	
	m_D3D12Device->CreateShaderResourceView(m_DefaultTextureResource.Get(), &SRVDescriptorDesc, SRV_CPUHandle);

	// 获取 SRV 描述符的 GPU 映射句柄，用于命令列表设置 SRVHeap 描述符堆，着色器引用 SRV 描述符找纹理资源
	SRV_GPUHandle = m_SRVHeap->GetGPUDescriptorHandleForHeapStart();

}

inline void DX12Engine::CreateRootSignature()
{
	ComPtr<ID3DBlob> SignatureBlob;			// 根签名字节码
	ComPtr<ID3DBlob> ErrorBlob;				// 错误字节码，根签名创建失败时用 OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer()); 可以获取报错信息

	CD3DX12_DESCRIPTOR_RANGE SRVDescriptorRangeDesc = {};	// Range 描述符范围结构体，一块 Range 表示一堆连续的同类型描述符
	SRVDescriptorRangeDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;		// Range 类型，这里指定 SRV 类型，CBV_SRV_UAV 在这里分流
	SRVDescriptorRangeDesc.NumDescriptors = 1;								// Range 里面的描述符数量 N，一次可以绑定多个描述符到多个寄存器槽上
	SRVDescriptorRangeDesc.BaseShaderRegister = 0;							// Range 要绑定的起始寄存器槽编号 i，绑定范围是 [s(i),s(i+N)]，我们绑定 s0
	SRVDescriptorRangeDesc.RegisterSpace = 0;								// Range 要绑定的寄存器空间，默认都是 0
	SRVDescriptorRangeDesc.OffsetInDescriptorsFromTableStart = 0;			// Range 到根描述表开头的偏移量 (单位：描述符)，根签名需要用它来寻找 Range 的地址，我们这填 0 就行

	CD3DX12_ROOT_DESCRIPTOR_TABLE RootDescriptorTableDesc{};                // RootDescriptorTable 根描述表信息结构体，一个 Table 可以有多个 Range
	RootDescriptorTableDesc.pDescriptorRanges = &SRVDescriptorRangeDesc;	// Range 描述符范围指针
	RootDescriptorTableDesc.NumDescriptorRanges = 1;						// 根描述表中 Range 的数量


	CD3DX12_ROOT_PARAMETER RootParameter{};
	RootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// 根参数对像素着色器可见
	RootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // 根参数类型，这里指定为描述符表
	RootParameter.DescriptorTable = RootDescriptorTableDesc; // 根参数的描述符表信息

	CD3DX12_STATIC_SAMPLER_DESC StaticSamplerDesc{};	// 静态采样器描述符结构体
	StaticSamplerDesc.ShaderRegister = 0;				// 静态采样器要绑定的寄存器槽编号 s0
	StaticSamplerDesc.RegisterSpace = 0;				// 静态采样器要绑定的寄存器空间，默认都是 0
	StaticSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;	// 纹理过滤类型，这里我们直接选 邻近点采样 就行
	StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;			// 在 U 方向上的纹理寻址方式
	StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;			// 在 V 方向上的纹理寻址方式
	StaticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;			// 在 W 方向上的纹理寻址方式 (3D 纹理会用到)
	StaticSamplerDesc.MinLOD = 0;											// 最小 LOD 细节层次，这里我们默认填 0 就行
	StaticSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;							// 最大 LOD 细节层次，这里我们默认填 D3D12_FLOAT32_MAX (没有 LOD 上限)
	StaticSamplerDesc.MipLODBias = 0;										// 基础 Mipmap 采样偏移量，我们这里我们直接填 0 就行
	StaticSamplerDesc.MaxAnisotropy = 1;									// 各向异性过滤等级，我们不使用各向异性过滤，需要默认填 1
	StaticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;			// 这个是用于阴影贴图的，我们不需要用它，所以填 D3D12_COMPARISON_FUNC_NEVER


	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};	// 根签名描述符结构体
	rootSignatureDesc.NumParameters = 1;				// 根参数数量，我们这里只有一个根参数
	rootSignatureDesc.pParameters = &RootParameter;	// 根参数指针
	rootSignatureDesc.NumStaticSamplers = 1;			// 静态采样器数量，我们这里只有一个静态采样器
	rootSignatureDesc.pStaticSamplers = &StaticSamplerDesc; // 静态采样器指针
	// 根签名标志，可以设置渲染管线不同阶段下的输入参数状态。注意这里！我们要从 IA 阶段输入顶点数据，所以要通过根签名，设置渲染管线允许从 IA 阶段读入数据
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// 编译根签名，让根签名先编译成 GPU 可读的二进制字节码
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &SignatureBlob, &ErrorBlob);
	if (ErrorBlob)		// 如果根签名编译出错，ErrorBlob 可以提供报错信息
	{
		OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
		OutputDebugStringA("\n");
	}


	// 用这个二进制字节码创建根签名对象
	m_D3D12Device->CreateRootSignature(0, SignatureBlob->GetBufferPointer(), SignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));


}

inline void DX12Engine::CreatePSO()
{
	// PSO 描述符结构体
	D3D12_GRAPHICS_PIPELINE_STATE_DESC  PSODesc{};

	// Input Assembler 输入装配阶段
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc{};	// 输入布局描述符结构体
	D3D12_INPUT_ELEMENT_DESC InputElementDesc[]{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	InputLayoutDesc.NumElements = _countof(InputElementDesc); // 输入元素数量
	InputLayoutDesc.pInputElementDescs = InputElementDesc; // 输入元素指针
	PSODesc.InputLayout = InputLayoutDesc; // 将输入布局描述符赋值给 PSO 描述符

	ComPtr<ID3DBlob> VertexShaderBlob;		// 顶点着色器二进制字节码
	ComPtr<ID3DBlob> PixelShaderBlob;		// 像素着色器二进制字节码
	ComPtr<ID3DBlob> ErrorBlob;				// 错误字节码，根签名创建失败时用 OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer()); 可以获取报错信息

	// 编译顶点着色器 Vertex Shader
	D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\DrawTexture\\shader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", NULL, NULL, &VertexShaderBlob, &ErrorBlob);
	if (ErrorBlob)		// 如果着色器编译出错，ErrorBlob 可以提供报错信息
	{
		OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
		OutputDebugStringA("\n");
	}

	// 编译像素着色器 Pixel Shader
	D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\DrawTexture\\shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", NULL, NULL, &PixelShaderBlob, &ErrorBlob);
	if (ErrorBlob)		// 如果着色器编译出错，ErrorBlob 可以提供报错信息
	{
		OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
		OutputDebugStringA("\n");
	}

	PSODesc.VS.pShaderBytecode = VertexShaderBlob->GetBufferPointer(); // 顶点着色器字节码指针	
	PSODesc.VS.BytecodeLength = VertexShaderBlob->GetBufferSize();	// 顶点着色器字节码大小
	PSODesc.PS.pShaderBytecode = PixelShaderBlob->GetBufferPointer(); // 像素着色器字节码指针
	PSODesc.PS.BytecodeLength = PixelShaderBlob->GetBufferSize();	// 像素着色器字节码大小
	// 光栅化阶段
	PSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;			// 指定背面剔除	
	PSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;			// 指定实心填充
	
	// 第一次设置根签名！本次设置是将根签名与 PSO 绑定，设置渲染管线的输入参数状态
	PSODesc.pRootSignature = m_RootSignature.Get();

	PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // 指定图元类型，这里我们指定三角形	
	PSODesc.NumRenderTargets = 1;												// 指定渲染目标数量，我们只有一个渲染目标	
	PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;						// 指定渲染目标格式，我们的交换链格式是 DXGI_FORMAT_R8G8B8A8_UNORM
	PSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;				// 指定不启用混合
	// 设置采样次数，我们这里填 1 就行
	PSODesc.SampleDesc.Count = 1;
	// 设置采样掩码，这个是用于多重采样的，我们直接填全采样 (UINT_MAX，就是将 UINT 所有的比特位全部填充为 1) 就行
	PSODesc.SampleMask = UINT_MAX;
	// 创建 PSO 对象
	m_D3D12Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(m_PipelineStateObject.GetAddressOf()));

}

inline void DX12Engine::CreateVertexResource()
{
	// CPU 高速缓存上的顶点信息数组，注意这里的顶点坐标都是 NDC 空间坐标
	VERTEX vertexs[6] =
	{
		{{-0.75f, 0.75f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.75f, 0.75f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.75f, -0.75f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.75f, 0.75f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.75f, -0.75f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.75f, -0.75f, 0.0f, 1.0f}, {0.0f, 1.0f}}
	};

	CD3DX12_RESOURCE_DESC VertexDesc{};	// 顶点缓冲区资源描述符结构体
	VertexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;		// 资源类型，上传堆的资源类型都是 buffer 缓冲
	VertexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;			// 资源布局，指定资源的存储方式，上传堆的资源都是 row major 按行线性存储
	VertexDesc.Width = sizeof(vertexs);							// 资源宽度，上传堆的资源宽度是资源的总大小
	VertexDesc.Height = 1;										// 资源高度，上传堆仅仅是传递线性资源的，所以高度必须为 1
	VertexDesc.Format = DXGI_FORMAT_UNKNOWN;					// 资源格式，上传堆资源的格式必须为 UNKNOWN
	VertexDesc.DepthOrArraySize = 1;							// 资源深度，这个是用于纹理数组和 3D 纹理的，上传堆资源必须为 1
	VertexDesc.MipLevels = 1;									// Mipmap 等级，这个是用于纹理的，上传堆资源必须为 1
	VertexDesc.SampleDesc.Count = 1;							// 资源采样次数，上传堆资源都是填 1

	// 上传堆属性的结构体，上传堆位于 CPU 和 GPU 的共享内存
	CD3DX12_HEAP_PROPERTIES UploadHeapDesc{ D3D12_HEAP_TYPE_UPLOAD };
	// 创建顶点缓冲区资源
	m_D3D12Device->CreateCommittedResource(
		&UploadHeapDesc,					// 堆属性
		D3D12_HEAP_FLAG_NONE,
		&VertexDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_VertexResource.GetAddressOf())
	);

	BYTE* TransferPointer = nullptr;	// 用于传递资源的指针
	// Map 开始映射，Map 方法会得到上传堆资源的地址 (在共享内存上)，传递给指针，这样我们就能通过 memcpy 操作复制数据了
	m_VertexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransferPointer));
	// 复制顶点数据到顶点缓冲区资源上 (CPU 高速缓存 -> 共享内存)
	memcpy(TransferPointer, vertexs, sizeof(vertexs));
	// Unmap 结束映射，因为我们无法直接读写默认堆资源，需要上传堆复制到那里，在复制之前，我们需要先结束映射，让上传堆处于只读状态
	m_VertexResource->Unmap(0, nullptr);

	// 创建顶点缓冲区视图
	// 顶点缓冲区资源的 GPU 虚拟地址
	VertexBufferView.BufferLocation = m_VertexResource->GetGPUVirtualAddress();
	// 每个顶点的大小 (单位：字节)
	VertexBufferView.StrideInBytes = sizeof(VERTEX);
	// 顶点缓冲区的总大小 (单位：字节)
	VertexBufferView.SizeInBytes = sizeof(vertexs);


}

inline void DX12Engine::Render()
{
	// 重置命令分配器
	m_CommandAllocator->Reset();
	// 重置命令列表，设置初始状态 PSO
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
	//获取RTV堆首句柄
	RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	FrameIndex = m_DXGISwapChain->GetCurrentBackBufferIndex(); // 获取当前帧索引,其实是后台缓冲中第一个可用帧的索引
	// 根据当前帧索引偏移 RTV 堆句柄
	RTVHandle.ptr += FrameIndex * RTVDescriptorSize;

	// 资源屏障，将当前帧的渲染目标从 PRESENT 状态转换为 RENDER_TARGET 状态
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_RenderTarget[FrameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,//当前状态
		D3D12_RESOURCE_STATE_RENDER_TARGET //要转换的状态
	);

	m_CommandList->ResourceBarrier(1, &barrier); // 设置资源屏障
	// 第二次设置根签名！本次设置将会检查 渲染管线绑定的根签名 与 这里的根签名 是否匹配
	// 以及根签名指定的资源是否被正确绑定，检查完毕后会进行简单的映射
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	// 设置渲染管线状态，可以在上面 m_CommandList->Reset() 的时候直接在第二个参数设置 PSO
	m_CommandList->SetPipelineState(m_PipelineStateObject.Get());
	// 设置视口 (光栅化阶段)，用于光栅化里的屏幕映射
	m_CommandList->RSSetViewports(1, &viewPort);
	// 设置裁剪矩形 (光栅化阶段)
	m_CommandList->RSSetScissorRects(1, &ScissorRect);
	// 设置渲染目标 (输出合并阶段)
	m_CommandList->OMSetRenderTargets(1, &RTVHandle, false, nullptr);
	// 清空当前渲染目标的背景为天蓝色
	m_CommandList->ClearRenderTargetView(RTVHandle, DirectX::Colors::SkyBlue, 0, nullptr);
	// 用于设置描述符堆用的临时 ID3D12DescriptorHeap 数组
	ID3D12DescriptorHeap* _temp_DescriptorHeaps[] = { m_SRVHeap.Get() };
	// 设置描述符堆，着色器才能引用描述符
	m_CommandList->SetDescriptorHeaps(1, _temp_DescriptorHeaps);
	// 设置根参数 0 的描述符表，根参数 0 是一个描述符表，描述符表里有一个 Range，Range 里有一个 SRV 描述符
	m_CommandList->SetGraphicsRootDescriptorTable(0, SRV_GPUHandle);
	// 设置图元拓扑 (输入装配阶段)，我们这里设置三角形列表
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// 设置顶点缓冲区 (输入装配阶段)
	m_CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
	// 绘制图元
	m_CommandList->DrawInstanced(6, 1, 0, 0);
	// 资源屏障，将当前帧的渲染目标从 RENDER_TARGET 状态转换为 PRESENT 状态
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_RenderTarget[FrameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	m_CommandList->ResourceBarrier(1, &barrier); // 设置资源屏障

	// 关闭命令列表，Record 录制状态 -> Close 关闭状态，命令列表只有关闭才可以提交
	m_CommandList->Close();

	// 用于传递命令用的临时 ID3D12CommandList 数组
	ID3D12CommandList* _temp_cmdlists[] = { m_CommandList.Get() };

	m_CommandQueue->ExecuteCommandLists(1, _temp_cmdlists); // 提交命令，GPU 开始执行命令

	// 向命令队列发出交换缓冲的命令，此命令会加入到命令队列中，命令队列执行到该命令时，会通知交换链交换缓冲
	m_DXGISwapChain->Present(1, NULL);
	// 将围栏预定值设定为下一帧
	FenceValue++;
	// 在命令队列 (命令队列在 GPU 端) 设置围栏预定值，此命令会加入到命令队列中
	// 命令队列执行到这里会修改围栏值，表示渲染已完成，"击中"围栏
	m_CommandQueue->Signal(m_Fence.Get(), FenceValue);
	// 设置围栏的预定事件，当渲染完成时，围栏被"击中"，激发预定事件，将事件由无信号状态转换成有信号状态
	m_Fence->SetEventOnCompletion(FenceValue, RenderEvent);
}

inline void DX12Engine::RenderLoop()
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
inline LRESULT CALLBACK DX12Engine::CallBackFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

inline void DX12Engine::Run(HINSTANCE hins)
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

	engine.CreateRootSignature();
	engine.CreatePSO();
	engine.CreateVertexResource();

	engine.RenderLoop();
}

// 主函数
int WINAPI WinMain(HINSTANCE hins, HINSTANCE hPrev, LPSTR cmdLine, int cmdShow)
{
	DX12Engine::Run(hins);
	return 0;
}