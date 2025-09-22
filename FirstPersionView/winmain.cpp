#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include <wrl.h>
#include <string>
#include <sstream>
#include <functional>
#include <d3dx12.h>
#include <memory>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "dxguid.lib")

using namespace DirectX;
using namespace Microsoft::WRL;
//using Microsoft;

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

class CallBackWrapper
{
public:
	inline static std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> Broker_Func;
	static LRESULT CALLBACK CallBackFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return Broker_Func(hWnd, uMsg, wParam, lParam);
	}
};

class Camera
{
private:
	XMVECTOR EyePosition = XMVectorSet(4, 3, 4, 1);			// 摄像机在世界空间下的位置
	XMVECTOR FocusPosition = XMVectorSet(0, 1, 1, 1);		// 摄像机在世界空间下观察的焦点位置
	XMVECTOR UpDirection = XMVectorSet(0, 1, 0, 0);			// 世界空间垂直向上的向量

	// 摄像机观察方向的单位向量，用于前后移动
	XMVECTOR ViewDirection = XMVector3Normalize(FocusPosition - EyePosition);

	// 焦距，摄像机原点与焦点的距离，XMVector3Length 表示对向量取模
	float FocalLength = XMVectorGetX(XMVector3Length(FocusPosition - EyePosition));

	XMVECTOR RightDirection = XMVector3Normalize(XMVector3Cross(ViewDirection, UpDirection));
	POINT LastCursorPoint{};
	float FovAngleY = XM_PIDIV4;							// 垂直视场角
	float AspectRatio = 4.0 / 3.0;							// 投影窗口宽高比
	float NearZ = 0.1;										// 近平面到原点的距离
	float FarZ = 1000;										// 远平面到原点的距离

	XMMATRIX ModelMatrix = XMMatrixIdentity();
	XMMATRIX ViewMatrix = XMMatrixIdentity();
	XMMATRIX ProjectionMatrix = XMMatrixIdentity();
	XMMATRIX MVPMatrix = XMMatrixIdentity();
public:
	Camera()
	{
		// 模型矩阵，这里我们让模型旋转 30° 就行，注意这里只是一个示例，后文我们会将它移除，每个模型都应该拥有相对独立的模型矩阵
		ModelMatrix = XMMatrixRotationY(30.0f);
		// 观察矩阵，注意前两个参数是点，第三个参数才是向量
		ViewMatrix = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
		// 投影矩阵 (注意近平面和远平面距离不能 <= 0!)
		ProjectionMatrix = XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ);
	}

	// 摄像机前后移动，参数 Stride 是移动速度 (步长)，正数向前移动，负数向后移动
	void Walk(float Stride)
	{
		EyePosition += Stride * ViewDirection;
		FocusPosition += Stride * ViewDirection;
	}

	// 摄像机左右移动，参数 Stride 是移动速度 (步长)，正数向左移动，负数向右移动
	void Strafe(float Stride)
	{
		EyePosition += Stride * RightDirection;
		FocusPosition += Stride * RightDirection;
	}

	// 鼠标在屏幕空间 y 轴上移动，相当于摄像机以向右的向量 RightDirection 向上向下旋转，人眼往上下看
	void RotateByY(float angleY)
	{
		// 以向右向量为轴构建旋转矩阵，旋转 ViewDirection 和 UpDirection
		XMMATRIX R = XMMatrixRotationAxis(RightDirection, -angleY);

		UpDirection = XMVector3TransformNormal(UpDirection, R);
		ViewDirection = XMVector3TransformNormal(ViewDirection, R);

		// 利用 ViewDirection 观察向量、FocalLength 焦距，更新焦点位置
		FocusPosition = EyePosition + ViewDirection * FocalLength;
	}

	// 鼠标在屏幕空间 x 轴上移动，相当于摄像机绕世界空间的 y 轴向左向右旋转，人眼往左右看
	void RotateByX(float angleX)
	{
		// 以世界坐标系下的 y 轴 (0,1,0,0) 构建旋转矩阵，三个向量 ViewDirection, UpDirection, RightDirection 都要旋转
		XMMATRIX R = XMMatrixRotationY(angleX);

		UpDirection = XMVector3TransformNormal(UpDirection, R);
		ViewDirection = XMVector3TransformNormal(ViewDirection, R);
		RightDirection = XMVector3TransformNormal(RightDirection, R);

		// 利用 ViewDirection 观察向量、FocalLength 焦距，更新焦点位置
		FocusPosition = EyePosition + ViewDirection * FocalLength;
	}

	void UpdateLastCursorPos()
	{
		GetCursorPos(&LastCursorPoint);
	}

	// 当鼠标左键长按并移动时，旋转摄像机视角
	void CameraRotate()
	{
		POINT CurrentCursorPoint = {};
		GetCursorPos(&CurrentCursorPoint);	// 获取当前鼠标位置

		// 根据鼠标在屏幕坐标系的 x,y 轴的偏移量，计算摄像机旋转角
		float AngleX = XMConvertToRadians(0.25 * static_cast<float>(CurrentCursorPoint.x - LastCursorPoint.x));
		float AngleY = XMConvertToRadians(0.25 * static_cast<float>(CurrentCursorPoint.y - LastCursorPoint.y));

		// 旋转摄像机
		RotateByY(AngleY);
		RotateByX(AngleX);

		UpdateLastCursorPos();		// 旋转完毕，更新上一次的鼠标位置
	}

	void UpdateMVPMatrix()
	{
		ViewMatrix = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
		MVPMatrix = ModelMatrix * ViewMatrix * ProjectionMatrix;
	}
	// 获取 MVP 矩阵
	XMMATRIX& GetMVPMatrix()
	{
		// 每次返回前，都更新一次
		UpdateMVPMatrix();
		return MVPMatrix;
	}
};



class DX12Engine
{
private:
	int WindowWidth = 640;
	int WindowHeight = 480;
	HWND m_hwnd = nullptr;

	ComPtr<ID3D12Debug> m_D3D12DebugDevice;
	UINT m_DXGICreateFactoryFlag = 0;
	ComPtr<IDXGIFactory5> m_DXGIFactory;
	ComPtr<IDXGIAdapter1> m_DXGIAdapter;
	ComPtr<ID3D12Device4> m_D3D12Device;					// D3D12 核心设备


	//命令队列
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

	ComPtr<IDXGISwapChain3> m_DXGISwapChain;
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap;
	ComPtr<ID3D12Resource> m_RenderTarget[3];
	CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle;
	UINT RTVDescriptorSize;
	UINT FrameIndex;

	ComPtr<ID3D12Fence> m_Fence;
	UINT64 FenceValue = 0;
	HANDLE RenderEvent;
	CD3DX12_RESOURCE_BARRIER barrier;

	std::wstring TextureFilename = L"E:\\DirectX12Project\\DirectX12Pro\\FirstPersionView\\diamond_ore.png";		// 纹理文件名 (这里用的是相对路径)
	ComPtr<IWICImagingFactory> m_WICFactory;				// WIC 工厂
	ComPtr<IWICBitmapDecoder> m_WICBitmapDecoder;			// 位图解码器
	ComPtr<IWICBitmapFrameDecode> m_WICBitmapDecodeFrame;	// 由解码器得到的单个位图帧
	ComPtr<IWICFormatConverter> m_WICFormatConverter;		// 位图转换器
	ComPtr<IWICBitmapSource> m_WICBitmapSource;				// WIC 位图资源，用于获取位图数据
	UINT TextureWidth = 0;
	UINT TextureHeight = 0;
	UINT BitsPerPixel = 0;
	UINT BytesPerRowSize = 0;
	DXGI_FORMAT TextureFormat = DXGI_FORMAT_UNKNOWN;

	ComPtr<ID3D12DescriptorHeap> m_SRVHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE SRV_CPUHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE SRV_GPUHandle;

	ComPtr<ID3D12Resource> m_UploadTextureResource;
	ComPtr<ID3D12Resource> m_DefaultTextureResource;

	UINT TextureSize = 0;
	UINT UploadResourceRowSize = 0;
	UINT UploadResourceSize = 0;

	ComPtr<ID3D12Resource> m_CBVResource;
	struct CBuffer								// 常量缓冲结构体
	{
		XMFLOAT4X4 MVPMatrix;		// MVP 矩阵，用于将顶点数据从顶点空间变换到齐次裁剪空间
	};
	CBuffer* MVPBuffer = nullptr;	// 常量缓冲结构体指针，里面存储的是 MVP 矩阵信息，下文 Map 后指针会指向 CBVResource 的地址

	Camera m_FirstCamera;			// 第一人称摄像机

	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineStateObject;

	ComPtr<ID3D12Resource> m_VertexResource;
	struct VERTEX											// 顶点数据结构体
	{
		XMFLOAT4 position;									// 顶点位置
		XMFLOAT2 texcoordUV;								// 顶点纹理坐标
	};
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	ComPtr<ID3D12Resource> m_IndexResource;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;

	D3D12_VIEWPORT viewPort = D3D12_VIEWPORT{ 0, 0, float(WindowWidth), float(WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	// 裁剪矩形
	D3D12_RECT ScissorRect = D3D12_RECT{ 0, 0, WindowWidth, WindowHeight };
	std::unique_ptr<BYTE[]> TextureData;

public:
	void InitWindow(HINSTANCE hins)
	{
		WNDCLASS wc{};
		wc.hInstance = hins;
		CallBackWrapper::Broker_Func = std::bind(&DX12Engine::CallBackFunc, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		wc.lpfnWndProc = CallBackWrapper::CallBackFunc;

		wc.lpszClassName = L"DX12Window";
		RegisterClass(&wc);
		m_hwnd = CreateWindow(
			L"DX12Window",
			L"DX12Window",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			WindowWidth,
			WindowHeight,
			nullptr,
			nullptr,
			hins,
			nullptr
		);

		ShowWindow(m_hwnd, SW_SHOW);


	}
	// 创建调试层
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

	// 创建设备
	bool CreateDevice()
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

	void CreateCommandComponents()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		m_D3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
		m_D3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
		m_D3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList));
		m_CommandList->Close();
	}

	void CreateRenderTargets()
	{
		D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
		RTVHeapDesc.NumDescriptors = 3;
		RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_D3D12Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_RTVHeap));
		RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
		RTVDescriptorSize = m_D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
		swapChainDesc.BufferCount = 3;
		swapChainDesc.Width = WindowWidth;
		swapChainDesc.Height = WindowHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;
		ComPtr<IDXGISwapChain1> _temp_swapchain;
		m_DXGIFactory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_hwnd, &swapChainDesc, nullptr, nullptr, &_temp_swapchain);
		_temp_swapchain.As(&m_DXGISwapChain);

		for (int i = 0; i < 3; i++)
		{
			m_DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i]));
			m_D3D12Device->CreateRenderTargetView(m_RenderTarget[i].Get(), nullptr, RTVHandle);
			RTVHandle.ptr += RTVDescriptorSize;
		}


	}

	void CreateFence()
	{
		//_1:是否可以被子进程访问，_2:事件出发后是否手动复位到未触发状态，_3:事件的初始状态，_4:事件名称
		RenderEvent = CreateEvent(nullptr, false, true, nullptr);
		m_D3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));

	}
	// 加载纹理到内存中
	bool LoadTextureFromFile()
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

	void CreateSRVHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc{};
		SRVHeapDesc.NumDescriptors = 1;
		SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_D3D12Device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(&m_SRVHeap));

	}

	// 上取整算法，对 A 向上取整，判断至少要多少个长度为 B 的空间才能容纳 A，用于内存对齐
	inline UINT Ceil(UINT A, UINT B)
	{
		return (A + B - 1) / B;
	}

	void CreateUploadAndDefaultResource()
	{
		BytesPerRowSize = TextureWidth * BitsPerPixel / 8;
		TextureSize = BytesPerRowSize * TextureHeight;

		UploadResourceRowSize = Ceil(BytesPerRowSize, 256) * 256;
		UploadResourceSize = UploadResourceRowSize * (TextureHeight - 1) + BytesPerRowSize;
		//UploadResourceSize = UploadResourceRowSize * TextureHeight;

		CD3DX12_RESOURCE_DESC UploadResourceDesc{};
		UploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		UploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		UploadResourceDesc.Width = UploadResourceSize;
		UploadResourceDesc.Height = 1;
		UploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		UploadResourceDesc.DepthOrArraySize = 1;
		UploadResourceDesc.MipLevels = 1;//只创建一个 Mip 层级
		UploadResourceDesc.SampleDesc.Count = 1;

		CD3DX12_HEAP_PROPERTIES HeapProperties0{ D3D12_HEAP_TYPE_UPLOAD };

		m_D3D12Device->CreateCommittedResource(
			&HeapProperties0,
			D3D12_HEAP_FLAG_NONE,
			&UploadResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_UploadTextureResource)
		);


		// 创建默认资源
		CD3DX12_RESOURCE_DESC DefaultResourceDesc{};
		DefaultResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		DefaultResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		DefaultResourceDesc.Width = TextureWidth;
		DefaultResourceDesc.Height = TextureHeight;
		DefaultResourceDesc.Format = TextureFormat;
		DefaultResourceDesc.DepthOrArraySize = 1;
		DefaultResourceDesc.MipLevels = 1;
		DefaultResourceDesc.SampleDesc.Count = 1;

		CD3DX12_HEAP_PROPERTIES HeapProperties{ D3D12_HEAP_TYPE_DEFAULT };
		m_D3D12Device->CreateCommittedResource(
			&HeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&DefaultResourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&m_DefaultTextureResource)
		);


	}

	void CopyTextureDataToDefaultResource()
	{
		// 用于暂时存储纹理数据的指针，这里要用 malloc 分配空间
		//TextureData = (BYTE*)malloc(TextureSize);
		//unique_ptr<BYTE> TextureData = nullptr; // 纹理数据
		TextureData = std::make_unique<BYTE[]>(TextureSize);
		// 将整块纹理数据读到 TextureData 中，方便后文的 memcpy 复制操作
		m_WICBitmapSource->CopyPixels(nullptr, BytesPerRowSize, TextureSize, TextureData.get());
		BYTE* TextureDataPtr = TextureData.get(); // 保存初始指针，后面释放内存时要用
		// 用于传递资源的指针
		BYTE* TransferPointer = nullptr;

		D3D12_SUBRESOURCE_DATA textureData{};
		textureData.pData = TextureData.get();					// 指向纹理数据的指针
		textureData.RowPitch = BytesPerRowSize;			//为对齐后每行的大小
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
		SRVDescriptorDesc.Format = TextureFormat;
		SRVDescriptorDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDescriptorDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDescriptorDesc.Texture2D.MipLevels = 1;//能访问的 Mip 层级数

		SRV_CPUHandle = m_SRVHeap->GetCPUDescriptorHandleForHeapStart();
		m_D3D12Device->CreateShaderResourceView(m_DefaultTextureResource.Get(), &SRVDescriptorDesc, SRV_CPUHandle);
		//CPU 描述符句柄用来创建描述符，GPU 描述符句柄让GPU来使用资源
		SRV_GPUHandle = m_SRVHeap->GetGPUDescriptorHandleForHeapStart();


	}

	void CreateCBVResource()
	{
		UINT CBufferWidth = Ceil(sizeof(CBuffer), 256) * 256;

		CD3DX12_RESOURCE_DESC CBVResourceDesc{};
		CBVResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		//D3D12_TEXTURE_LAYOUT_UNKNOWN让驱动自己挑选最优布局
		CBVResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;//此处和D3D12_TEXTURE_LAYOUT_UNKNOWN是一样的，意思是线性布局
		CBVResourceDesc.Width = CBufferWidth;
		CBVResourceDesc.Height = 1;
		CBVResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		CBVResourceDesc.DepthOrArraySize = 1;
		CBVResourceDesc.MipLevels = 1;
		CBVResourceDesc.SampleDesc.Count = 1;

		CD3DX12_HEAP_PROPERTIES UploadHeapDesc{ D3D12_HEAP_TYPE_UPLOAD };
		m_D3D12Device->CreateCommittedResource(
			&UploadHeapDesc,
			D3D12_HEAP_FLAG_NONE,
			&CBVResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_CBVResource)
		);
		m_CBVResource->Map(0, nullptr, reinterpret_cast<void**>(&MVPBuffer));//拿到CB的映射地址


	}
	void CreateRootSignature()
	{
		ComPtr<ID3DBlob> SignatureBlob;
		ComPtr<ID3DBlob> ErrorBlob;

		CD3DX12_ROOT_PARAMETER RootParameters[2]{};

		D3D12_DESCRIPTOR_RANGE SRVDescriptorRangeDesc{};
		SRVDescriptorRangeDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		SRVDescriptorRangeDesc.NumDescriptors = 1;
		SRVDescriptorRangeDesc.BaseShaderRegister = 0;
		SRVDescriptorRangeDesc.RegisterSpace = 0;
		SRVDescriptorRangeDesc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		RootParameters[0].InitAsDescriptorTable(1, &SRVDescriptorRangeDesc, D3D12_SHADER_VISIBILITY_PIXEL);

		RootParameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_STATIC_SAMPLER_DESC StaticSamplerDesc{};
		StaticSamplerDesc.ShaderRegister = 0;
		StaticSamplerDesc.RegisterSpace = 0;
		StaticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		StaticSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		StaticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		StaticSamplerDesc.MinLOD = 0.0f;
		StaticSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		StaticSamplerDesc.MipLODBias = 0.0f;
		StaticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		rootSignatureDesc.Init(_countof(RootParameters), RootParameters, 1, &StaticSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &SignatureBlob, &ErrorBlob);
		if (ErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
		}
		m_D3D12Device->CreateRootSignature(0, SignatureBlob->GetBufferPointer(), SignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
	}

	void CreatePSO()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc{};
		D3D12_INPUT_LAYOUT_DESC InputLayoutDesc{};
		D3D12_INPUT_ELEMENT_DESC InputElementDesc[2]{
			"POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0,
			"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		};

		InputLayoutDesc.NumElements = _countof(InputElementDesc);
		InputLayoutDesc.pInputElementDescs = InputElementDesc;

		PsoDesc.InputLayout = InputLayoutDesc;


		ComPtr<ID3DBlob> VertexShaderBlob;
		ComPtr<ID3DBlob> PixelShaderBlob;
		ComPtr<ID3DBlob> ErrorBlob;
		D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\FirstPersionView\\shader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", 0, 0, &VertexShaderBlob, nullptr);
		if (ErrorBlob)		// 如果着色器编译出错，ErrorBlob 可以提供报错信息
		{
			OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
			OutputDebugStringA("\n");
		}
		// 编译像素着色器 Pixel Shader
		D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\FirstPersionView\\shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", NULL, NULL, &PixelShaderBlob, &ErrorBlob);
		if (ErrorBlob)		// 如果着色器编译出错，ErrorBlob 可以提供报错信息
		{
			OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
			OutputDebugStringA("\n");
		}

		PsoDesc.VS.pShaderBytecode = VertexShaderBlob->GetBufferPointer();
		PsoDesc.VS.BytecodeLength = VertexShaderBlob->GetBufferSize();
		PsoDesc.PS.pShaderBytecode = PixelShaderBlob->GetBufferPointer();
		PsoDesc.PS.BytecodeLength = PixelShaderBlob->GetBufferSize();
		PsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		PsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

		PsoDesc.pRootSignature = m_RootSignature.Get();
		PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		PsoDesc.NumRenderTargets = 1;
		PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		PsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		PsoDesc.SampleDesc.Count = 1;
		PsoDesc.SampleMask = UINT_MAX;

		HRESULT hr = m_D3D12Device->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&m_PipelineStateObject));
		if (FAILED(hr)) {
			WCHAR errorMsg[1024] = { 0 };
			// 用 FormatMessage 解析 HRESULT 为可读字符串
			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,        // 无模块句柄（从系统错误表取）
				hr,             // 要解析的 HRESULT
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 语言（中性）
				errorMsg,       // 输出缓冲区
				_countof(errorMsg), // 缓冲区大小
				nullptr
			);

			// 拼接并显示错误信息
			std::wstringstream msg;
			msg << L"创建资源失败！错误码: 0x" << std::hex << hr << L"\n" << errorMsg;
			MessageBox(nullptr, msg.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
			return;
		}
	}

	void CreateVertexResource()
	{
#pragma region 顶点信息
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

		CD3DX12_RESOURCE_DESC VertexDesc{};
		VertexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		VertexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		VertexDesc.Width = sizeof(vertexs);
		VertexDesc.Height = 1;
		VertexDesc.Format = DXGI_FORMAT_UNKNOWN;
		VertexDesc.DepthOrArraySize = 1;
		VertexDesc.MipLevels = 1;
		VertexDesc.SampleDesc.Count = 1;

		D3D12_HEAP_PROPERTIES UploadHeapProperties{ D3D12_HEAP_TYPE_UPLOAD };
		HRESULT hr =m_D3D12Device->CreateCommittedResource(&UploadHeapProperties, D3D12_HEAP_FLAG_NONE, &VertexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexResource));
		if (FAILED(hr)) {
			WCHAR errorMsg[1024] = { 0 };
			// 用 FormatMessage 解析 HRESULT 为可读字符串
			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,        // 无模块句柄（从系统错误表取）
				hr,             // 要解析的 HRESULT
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // 语言（中性）
				errorMsg,       // 输出缓冲区
				_countof(errorMsg), // 缓冲区大小
				nullptr
			);

			// 拼接并显示错误信息
			std::wstringstream msg;
			msg << L"创建顶点资源失败！错误码: 0x" << std::hex << hr << L"\n" << errorMsg;
			MessageBox(nullptr, msg.str().c_str(), L"错误", MB_OK | MB_ICONERROR);
			return;
		}
		BYTE* m_VertexData = nullptr;
		m_VertexResource->Map(0, nullptr, reinterpret_cast<void**>(&m_VertexData));
		memcpy(m_VertexData, vertexs, sizeof(vertexs));
		m_VertexResource->Unmap(0, nullptr);

		VertexBufferView.BufferLocation = m_VertexResource->GetGPUVirtualAddress();
		VertexBufferView.StrideInBytes = sizeof(VERTEX);
		VertexBufferView.SizeInBytes = sizeof(vertexs);

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

		CD3DX12_RESOURCE_DESC IndexDesc{};
		IndexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		IndexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; //缓冲区就是一维的，只有纹理资源可以使用D3D12_TEXTURE_LAYOUT_UNKNOWN;
		IndexDesc.Width = sizeof(IndexArray);
		IndexDesc.Height = 1;
		IndexDesc.Format = DXGI_FORMAT_UNKNOWN;
		IndexDesc.DepthOrArraySize = 1;
		IndexDesc.MipLevels = 1;
		IndexDesc.SampleDesc.Count = 1;

		CD3DX12_HEAP_PROPERTIES UploadHeapProperties{ D3D12_HEAP_TYPE_UPLOAD };
		m_D3D12Device->CreateCommittedResource(&UploadHeapProperties, D3D12_HEAP_FLAG_NONE, &IndexDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexResource));

		BYTE* m_IndexData = nullptr;
		m_IndexResource->Map(0, nullptr, (void**)&m_IndexData);
		memcpy(m_IndexData, IndexArray, sizeof(IndexArray));
		m_IndexResource->Unmap(0, nullptr);
		IndexBufferView.BufferLocation = m_IndexResource->GetGPUVirtualAddress();
		IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		IndexBufferView.SizeInBytes = sizeof(IndexArray);
	}

	void UpdateConstantBuffer()
	{
		XMStoreFloat4x4(&MVPBuffer->MVPMatrix, m_FirstCamera.GetMVPMatrix());
	}

	void Render()
	{
		UpdateConstantBuffer();
		RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
		FrameIndex = m_DXGISwapChain->GetCurrentBackBufferIndex();
		RTVHandle.ptr += FrameIndex * RTVDescriptorSize;
		
		m_CommandAllocator->Reset();
        m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
		//设置资源屏障
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_RenderTarget[FrameIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		m_CommandList->ResourceBarrier(1, &barrier);
		//设置输出对象
		m_CommandList->OMSetRenderTargets(1, &RTVHandle, false, nullptr);
		m_CommandList->ClearRenderTargetView(RTVHandle, DirectX::Colors::SkyBlue, 0, nullptr);
		//设置根签名
		m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
		m_CommandList->SetPipelineState(m_PipelineStateObject.Get());

		m_CommandList->RSSetViewports(1, &viewPort);
		m_CommandList->RSSetScissorRects(1, &ScissorRect);

		ID3D12DescriptorHeap* Heaps[] = { m_SRVHeap.Get() };
		m_CommandList->SetDescriptorHeaps(_countof(Heaps), Heaps);
		m_CommandList->SetGraphicsRootDescriptorTable(0, SRV_GPUHandle);
		m_CommandList->SetGraphicsRootConstantBufferView(1, m_CBVResource->GetGPUVirtualAddress());

		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
        m_CommandList->IASetIndexBuffer(&IndexBufferView);
        m_CommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_RenderTarget[FrameIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);
		m_CommandList->ResourceBarrier(1, &barrier);
		m_CommandList->Close();

		ID3D12CommandList* _temp_cmdlists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(_temp_cmdlists), _temp_cmdlists);
        m_DXGISwapChain->Present(1, 0);

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
			{
				Render();
				Sleep(10);
			}
			break;


			case 1:				// ActiveEvent 是 1，说明渲染事件未完成，CPU 主线程同时处理窗口消息，防止界面假死
			{
				// 查看消息队列是否有消息，如果有就获取。 PM_REMOVE 表示获取完消息，就立刻将该消息从消息队列中移除
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					// 如果程序没有收到退出消息，就向操作系统发出派发消息的命令
					if (msg.message != WM_QUIT)
					{
						TranslateMessage(&msg);		// 翻译消息，当键盘按键发出信号 (WM_KEYDOWN)，将虚拟按键值转换为对应的 ASCII 码，同时产生 WM_CHAR 消息
						DispatchMessage(&msg);		// 派发消息，通知操作系统调用回调函数处理消息
					}
					else
					{
						isExit = true;							// 收到退出消息，就退出消息循环
					}
				}
			}
			break;


			case WAIT_TIMEOUT:	// 渲染超时
			{

			}
			break;

			}
		}
	}

	// 回调函数，处理窗口产生的消息
	// WASD 键 —— WM_CHAR 字符消息 —— 摄像机前后左右移动
	// 鼠标长按左键移动 —— WM_MOUSEMOVE 鼠标移动消息 —— 摄像机视角旋转
	// 关闭窗口 —— WM_DESTROY 窗口销毁消息 —— 窗口关闭，程序进程退出
	LRESULT CALLBACK CallBackFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// 用 switch 将第二个参数分流，每个 case 分别对应一个窗口消息
		switch (msg)
		{
		case WM_DESTROY:			// 窗口被销毁 (当按下右上角 X 关闭窗口时)
		{
			PostQuitMessage(0);		// 向操作系统发出退出请求 (WM_QUIT)，结束消息循环
		}
		break;


		case WM_CHAR:	// 获取键盘产生的字符消息，TranslateMessage 会将虚拟键码翻译成字符码，同时产生 WM_CHAR 消息
		{
			switch (wParam)		// wParam 是按键对应的字符 ASCII 码
			{
			case 'w':
			case 'W':	// 向前移动
				m_FirstCamera.Walk(0.1);
				break;

			case 's':
			case 'S':	// 向后移动
				m_FirstCamera.Walk(-0.1);
				break;

			case 'a':
			case 'A':	// 向左移动
				m_FirstCamera.Strafe(0.1);
				break;

			case 'd':
			case 'D':	// 向右移动
				m_FirstCamera.Strafe(-0.1);
				break;
			}
		}
		break;


		case WM_MOUSEMOVE:	// 获取鼠标移动消息
		{
			switch (wParam)	// wParam 是鼠标按键的状态
			{
			case MK_LBUTTON:	// 当用户长按鼠标左键的同时移动鼠标，摄像机旋转
				m_FirstCamera.CameraRotate();
				break;

				// 按键没按，鼠标只是移动也要更新，否则就会发生摄像机视角瞬移
			default: m_FirstCamera.UpdateLastCursorPos();
			}
		}
		break;


		// 如果接收到其他消息，直接默认返回整个窗口
		default: return DefWindowProc(hwnd, msg, wParam, lParam);

		}

		return 0;	// 注意这里！default 除外的分支都会运行到这里，因此需要 return 0，否则就会返回系统随机值，导致窗口无法正常显示
	}

	// 运行窗口
	static void Run(HINSTANCE hins)
	{
		DX12Engine engine;
		engine.InitWindow(hins);
		engine.CreateDebugDevice();
		engine.CreateDevice();
		engine.CreateCommandComponents();
		engine.CreateRenderTargets();
		engine.CreateFence();

		engine.LoadTextureFromFile();
		engine.CreateSRVHeap();
		engine.CreateUploadAndDefaultResource();
		engine.CopyTextureDataToDefaultResource();
		engine.CreateSRV();

		engine.CreateCBVResource();

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


