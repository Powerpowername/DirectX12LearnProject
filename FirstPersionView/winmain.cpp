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

// �����ռ� DX12TextureHelper �����˰�������ת������ͼƬ��ʽ�Ľṹ���뺯��
namespace DX12TextureHelper
{
	// ����ת���ã����� DX12 ��֧�ֵĸ�ʽ��DX12 û����

	// Standard GUID -> DXGI ��ʽת���ṹ��
	struct WICTranslate
	{
		GUID wic;
		DXGI_FORMAT format;
	};

	// WIC ��ʽ�� DXGI ���ظ�ʽ�Ķ�Ӧ���ñ��еĸ�ʽΪ��֧�ֵĸ�ʽ
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

	// GUID -> Standard GUID ��ʽת���ṹ��
	struct WICConvert
	{
		GUID source;
		GUID target;
	};

	// WIC ���ظ�ʽת����
	static WICConvert g_WICConvert[] =
	{
		// Ŀ���ʽһ������ӽ��ı�֧�ֵĸ�ʽ
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


	// ���ȷ�����ݵ���ӽ���ʽ���ĸ�
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
		return false;		// �Ҳ������ͷ��� false
	}

	// ���ȷ�����ն�Ӧ�� DXGI ��ʽ����һ��
	DXGI_FORMAT GetDXGIFormatFromPixelFormat(const GUID* pPixelFormat)
	{
		for (size_t i = 0; i < _countof(g_WICFormats); ++i)
		{
			if (InlineIsEqualGUID(g_WICFormats[i].wic, *pPixelFormat))
			{
				return g_WICFormats[i].format;
			}
		}
		return DXGI_FORMAT_UNKNOWN;		// �Ҳ������ͷ��� UNKNOWN
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
	XMVECTOR EyePosition = XMVectorSet(4, 3, 4, 1);			// �����������ռ��µ�λ��
	XMVECTOR FocusPosition = XMVectorSet(0, 1, 1, 1);		// �����������ռ��¹۲�Ľ���λ��
	XMVECTOR UpDirection = XMVectorSet(0, 1, 0, 0);			// ����ռ䴹ֱ���ϵ�����

	// ������۲췽��ĵ�λ����������ǰ���ƶ�
	XMVECTOR ViewDirection = XMVector3Normalize(FocusPosition - EyePosition);

	// ���࣬�����ԭ���뽹��ľ��룬XMVector3Length ��ʾ������ȡģ
	float FocalLength = XMVectorGetX(XMVector3Length(FocusPosition - EyePosition));

	XMVECTOR RightDirection = XMVector3Normalize(XMVector3Cross(ViewDirection, UpDirection));
	POINT LastCursorPoint{};
	float FovAngleY = XM_PIDIV4;							// ��ֱ�ӳ���
	float AspectRatio = 4.0 / 3.0;							// ͶӰ���ڿ�߱�
	float NearZ = 0.1;										// ��ƽ�浽ԭ��ľ���
	float FarZ = 1000;										// Զƽ�浽ԭ��ľ���

	XMMATRIX ModelMatrix = XMMatrixIdentity();
	XMMATRIX ViewMatrix = XMMatrixIdentity();
	XMMATRIX ProjectionMatrix = XMMatrixIdentity();
	XMMATRIX MVPMatrix = XMMatrixIdentity();
public:
	Camera()
	{
		// ģ�;�������������ģ����ת 30�� ���У�ע������ֻ��һ��ʾ�����������ǻὫ���Ƴ���ÿ��ģ�Ͷ�Ӧ��ӵ����Զ�����ģ�;���
		ModelMatrix = XMMatrixRotationY(30.0f);
		// �۲����ע��ǰ���������ǵ㣬������������������
		ViewMatrix = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
		// ͶӰ���� (ע���ƽ���Զƽ����벻�� <= 0!)
		ProjectionMatrix = XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ);
	}

	// �����ǰ���ƶ������� Stride ���ƶ��ٶ� (����)��������ǰ�ƶ�����������ƶ�
	void Walk(float Stride)
	{
		EyePosition += Stride * ViewDirection;
		FocusPosition += Stride * ViewDirection;
	}

	// ����������ƶ������� Stride ���ƶ��ٶ� (����)�����������ƶ������������ƶ�
	void Strafe(float Stride)
	{
		EyePosition += Stride * RightDirection;
		FocusPosition += Stride * RightDirection;
	}

	// �������Ļ�ռ� y �����ƶ����൱������������ҵ����� RightDirection ����������ת�����������¿�
	void RotateByY(float angleY)
	{
		// ����������Ϊ�ṹ����ת������ת ViewDirection �� UpDirection
		XMMATRIX R = XMMatrixRotationAxis(RightDirection, -angleY);

		UpDirection = XMVector3TransformNormal(UpDirection, R);
		ViewDirection = XMVector3TransformNormal(ViewDirection, R);

		// ���� ViewDirection �۲�������FocalLength ���࣬���½���λ��
		FocusPosition = EyePosition + ViewDirection * FocalLength;
	}

	// �������Ļ�ռ� x �����ƶ����൱�������������ռ�� y ������������ת�����������ҿ�
	void RotateByX(float angleX)
	{
		// ����������ϵ�µ� y �� (0,1,0,0) ������ת������������ ViewDirection, UpDirection, RightDirection ��Ҫ��ת
		XMMATRIX R = XMMatrixRotationY(angleX);

		UpDirection = XMVector3TransformNormal(UpDirection, R);
		ViewDirection = XMVector3TransformNormal(ViewDirection, R);
		RightDirection = XMVector3TransformNormal(RightDirection, R);

		// ���� ViewDirection �۲�������FocalLength ���࣬���½���λ��
		FocusPosition = EyePosition + ViewDirection * FocalLength;
	}

	void UpdateLastCursorPos()
	{
		GetCursorPos(&LastCursorPoint);
	}

	// ���������������ƶ�ʱ����ת������ӽ�
	void CameraRotate()
	{
		POINT CurrentCursorPoint = {};
		GetCursorPos(&CurrentCursorPoint);	// ��ȡ��ǰ���λ��

		// �����������Ļ����ϵ�� x,y ���ƫ�����������������ת��
		float AngleX = XMConvertToRadians(0.25 * static_cast<float>(CurrentCursorPoint.x - LastCursorPoint.x));
		float AngleY = XMConvertToRadians(0.25 * static_cast<float>(CurrentCursorPoint.y - LastCursorPoint.y));

		// ��ת�����
		RotateByY(AngleY);
		RotateByX(AngleX);

		UpdateLastCursorPos();		// ��ת��ϣ�������һ�ε����λ��
	}

	void UpdateMVPMatrix()
	{
		ViewMatrix = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
		MVPMatrix = ModelMatrix * ViewMatrix * ProjectionMatrix;
	}
	// ��ȡ MVP ����
	XMMATRIX& GetMVPMatrix()
	{
		// ÿ�η���ǰ��������һ��
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
	ComPtr<ID3D12Device4> m_D3D12Device;					// D3D12 �����豸


	//�������
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

	std::wstring TextureFilename = L"E:\\DirectX12Project\\DirectX12Pro\\FirstPersionView\\diamond_ore.png";		// �����ļ��� (�����õ������·��)
	ComPtr<IWICImagingFactory> m_WICFactory;				// WIC ����
	ComPtr<IWICBitmapDecoder> m_WICBitmapDecoder;			// λͼ������
	ComPtr<IWICBitmapFrameDecode> m_WICBitmapDecodeFrame;	// �ɽ������õ��ĵ���λͼ֡
	ComPtr<IWICFormatConverter> m_WICFormatConverter;		// λͼת����
	ComPtr<IWICBitmapSource> m_WICBitmapSource;				// WIC λͼ��Դ�����ڻ�ȡλͼ����
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
	struct CBuffer								// ��������ṹ��
	{
		XMFLOAT4X4 MVPMatrix;		// MVP �������ڽ��������ݴӶ���ռ�任����βü��ռ�
	};
	CBuffer* MVPBuffer = nullptr;	// ��������ṹ��ָ�룬����洢���� MVP ������Ϣ������ Map ��ָ���ָ�� CBVResource �ĵ�ַ

	Camera m_FirstCamera;			// ��һ�˳������

	ComPtr<ID3D12RootSignature> m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineStateObject;

	ComPtr<ID3D12Resource> m_VertexResource;
	struct VERTEX											// �������ݽṹ��
	{
		XMFLOAT4 position;									// ����λ��
		XMFLOAT2 texcoordUV;								// ������������
	};
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	ComPtr<ID3D12Resource> m_IndexResource;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;

	D3D12_VIEWPORT viewPort = D3D12_VIEWPORT{ 0, 0, float(WindowWidth), float(WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	// �ü�����
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
	// �������Բ�
	void CreateDebugDevice()
	{
		::CoInitialize(nullptr);	// ע�����DX12 �������豸�ӿڶ��ǻ��� COM �ӿڵģ�������Ҫ��ȫ����ʼ��Ϊ nullptr

#if defined(_DEBUG)		// ����� Debug ģʽ�±��룬��ִ������Ĵ���

		// ��ȡ���Բ��豸�ӿ�
		D3D12GetDebugInterface(IID_PPV_ARGS(&m_D3D12DebugDevice));
		// �������Բ�
		m_D3D12DebugDevice->EnableDebugLayer();
		// �������Բ�󣬴��� DXGI ����Ҳ��Ҫ Debug Flag
		m_DXGICreateFactoryFlag = DXGI_CREATE_FACTORY_DEBUG;

#endif
	}

	// �����豸
	bool CreateDevice()
	{
		// ���� DXGI ����
		CreateDXGIFactory2(m_DXGICreateFactoryFlag, IID_PPV_ARGS(&m_DXGIFactory));

		// DX12 ֧�ֵ����й��ܰ汾������Կ������Ҫ֧�� 11.0
		const D3D_FEATURE_LEVEL dx12SupportLevel[] =
		{
			D3D_FEATURE_LEVEL_12_2,		// 12.2
			D3D_FEATURE_LEVEL_12_1,		// 12.1
			D3D_FEATURE_LEVEL_12_0,		// 12.0
			D3D_FEATURE_LEVEL_11_1,		// 11.1
			D3D_FEATURE_LEVEL_11_0		// 11.0
		};


		// �� EnumAdapters1 �ȱ��������ϵ�ÿһ���Կ�
		// ÿ�ε��� EnumAdapters1 �ҵ��Կ����Զ����� DXGIAdapter �ӿڣ������� S_OK
		// �Ҳ����Կ��᷵�� ERROR_NOT_FOUND

		for (UINT i = 0; m_DXGIFactory->EnumAdapters1(i, &m_DXGIAdapter) != ERROR_NOT_FOUND; i++)
		{
			// �ҵ��Կ����ʹ��� D3D12 �豸���Ӹߵ��ͱ������й��ܰ汾�������ɹ�������
			for (const auto& level : dx12SupportLevel)
			{
				// ���� D3D12 ���Ĳ��豸�������ɹ��ͷ��� true
				if (SUCCEEDED(D3D12CreateDevice(m_DXGIAdapter.Get(), level, IID_PPV_ARGS(&m_D3D12Device))))
				{
					DXGI_ADAPTER_DESC1 adap = {};
					m_DXGIAdapter->GetDesc1(&adap);
					OutputDebugStringW(adap.Description);
					return true;
				}
			}
		}

		// ����Ҳ����κ���֧�� DX12 ���Կ������˳�����
		if (m_D3D12Device == nullptr)
		{
			MessageBox(NULL, L"�Ҳ����κ���֧�� DX12 ���Կ��������������ϵ�Ӳ����", L"����", MB_OK | MB_ICONERROR);
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
		//_1:�Ƿ���Ա��ӽ��̷��ʣ�_2:�¼��������Ƿ��ֶ���λ��δ����״̬��_3:�¼��ĳ�ʼ״̬��_4:�¼�����
		RenderEvent = CreateEvent(nullptr, false, true, nullptr);
		m_D3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));

	}
	// ���������ڴ���
	bool LoadTextureFromFile()
	{
		// �����û���� WIC ���������½�һ�� WIC ����ʵ����ע�⣡WIC �����������ظ��ͷ��봴����
		if (m_WICFactory == nullptr) CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_WICFactory));

		// ����ͼƬ������������ͼƬ���뵽��������
		HRESULT hr = m_WICFactory->CreateDecoderFromFilename(TextureFilename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_WICBitmapDecoder);

		std::wostringstream output_str;		// ���ڸ�ʽ���ַ���
		switch (hr)
		{
		case S_OK: break;	// ����ɹ���ֱ�� break ������һ������

		case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):	// �ļ��Ҳ���
			output_str << L"�Ҳ����ļ� " << TextureFilename << L" �������ļ�·���Ƿ�����";
			MessageBox(NULL, output_str.str().c_str(), L"����", MB_OK | MB_ICONERROR);
			return false;

		case HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT):	// �ļ�������ڱ���һ��Ӧ�ý���ռ��
			output_str << L"�ļ� " << TextureFilename << L" �Ѿ�����һ��Ӧ�ý��̴򿪲�ռ���ˣ����ȹر��Ǹ�Ӧ�ý��̣�";
			MessageBox(NULL, output_str.str().c_str(), L"����", MB_OK | MB_ICONERROR);
			return false;

		case WINCODEC_ERR_COMPONENTNOTFOUND:			// �Ҳ����ɽ���������˵���ⲻ����Ч��ͼ���ļ�
			output_str << L"�ļ� " << TextureFilename << L" ������Ч��ͼ���ļ����޷����룡�����ļ��Ƿ�Ϊͼ���ļ���";
			MessageBox(NULL, output_str.str().c_str(), L"����", MB_OK | MB_ICONERROR);
			return false;

		default:			// ��������δ֪����
			output_str << L"�ļ� " << TextureFilename << L" ����ʧ�ܣ��������������󣬴����룺" << hr << L" �������΢��ٷ��ĵ���";
			MessageBox(NULL, output_str.str().c_str(), L"����", MB_OK | MB_ICONERROR);
			return false;
		}

		// ��ȡͼƬ���ݵĵ�һ֡����� GetFrame �������� gif ���ֶ�֡��ͼ
		m_WICBitmapDecoder->GetFrame(0, &m_WICBitmapDecodeFrame);


		// ��ȡͼƬ��ʽ��������ת��Ϊ DX12 �ܽ��ܵ������ʽ
		// ���������ʽ�޷�֧�ֵĴ��󣬿�����΢���ṩ�� ��ͼ3D ��ת����ǿ���Ƽ�!
		WICPixelFormatGUID SourceFormat = {};				// Դͼ��ʽ
		GUID TargetFormat = {};								// Ŀ���ʽ

		m_WICBitmapDecodeFrame->GetPixelFormat(&SourceFormat);						// ��ȡԴͼ��ʽ

		if (DX12TextureHelper::GetTargetPixelFormat(&SourceFormat, &TargetFormat))	// ��ȡĿ���ʽ
		{
			TextureFormat = DX12TextureHelper::GetDXGIFormatFromPixelFormat(&TargetFormat);	// ��ȡ DX12 ֧�ֵĸ�ʽ
		}
		else	// ���û�п�֧�ֵ�Ŀ���ʽ
		{
			::MessageBox(NULL, L"��������֧��!", L"��ʾ", MB_OK);
			return false;
		}


		// ��ȡĿ���ʽ�󣬽�����ת��ΪĿ���ʽ��ʹ���ܱ� DX12 ʹ��
		m_WICFactory->CreateFormatConverter(&m_WICFormatConverter);		// ����ͼƬת����
		// ��ʼ��ת������ʵ�����ǰ�λͼ������ת��
		m_WICFormatConverter->Initialize(m_WICBitmapDecodeFrame.Get(), TargetFormat, WICBitmapDitherTypeNone,
			nullptr, 0.0f, WICBitmapPaletteTypeCustom);
		// ��λͼ���ݼ̳е� WIC λͼ��Դ������Ҫ����� WIC λͼ��Դ�ϻ�ȡ��Ϣ
		m_WICFormatConverter.As(&m_WICBitmapSource);



		m_WICBitmapSource->GetSize(&TextureWidth, &TextureHeight);		// ��ȡ������

		ComPtr<IWICComponentInfo> _temp_WICComponentInfo = {};			// ���ڻ�ȡ BitsPerPixel ����ͼ�����
		ComPtr<IWICPixelFormatInfo> _temp_WICPixelInfo = {};			// ���ڻ�ȡ BitsPerPixel ����ͼ�����
		m_WICFactory->CreateComponentInfo(TargetFormat, &_temp_WICComponentInfo);
		_temp_WICComponentInfo.As(&_temp_WICPixelInfo);
		_temp_WICPixelInfo->GetBitsPerPixel(&BitsPerPixel);				// ��ȡ BitsPerPixel ͼ�����

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

	// ��ȡ���㷨���� A ����ȡ�����ж�����Ҫ���ٸ�����Ϊ B �Ŀռ�������� A�������ڴ����
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
		UploadResourceDesc.MipLevels = 1;//ֻ����һ�� Mip �㼶
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


		// ����Ĭ����Դ
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
		// ������ʱ�洢�������ݵ�ָ�룬����Ҫ�� malloc ����ռ�
		//TextureData = (BYTE*)malloc(TextureSize);
		//unique_ptr<BYTE> TextureData = nullptr; // ��������
		TextureData = std::make_unique<BYTE[]>(TextureSize);
		// �������������ݶ��� TextureData �У�������ĵ� memcpy ���Ʋ���
		m_WICBitmapSource->CopyPixels(nullptr, BytesPerRowSize, TextureSize, TextureData.get());
		BYTE* TextureDataPtr = TextureData.get(); // �����ʼָ�룬�����ͷ��ڴ�ʱҪ��
		// ���ڴ�����Դ��ָ��
		BYTE* TransferPointer = nullptr;

		D3D12_SUBRESOURCE_DATA textureData{};
		textureData.pData = TextureData.get();					// ָ���������ݵ�ָ��
		textureData.RowPitch = BytesPerRowSize;			//Ϊ�����ÿ�еĴ�С
		textureData.SlicePitch = TextureSize;					//ͯҥΪ�����������������ݴ�С����λ���ֽ�

		m_CommandAllocator->Reset();
		m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_DefaultTextureResource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
		m_CommandList->ResourceBarrier(1, &barrier);
		UpdateSubresources<1>(
			m_CommandList.Get(),
			m_DefaultTextureResource.Get(),
			m_UploadTextureResource.Get(),
			0, 0, 1, &textureData
		);//�ڲ����Ƚ����ݸ��Ƶ��ϴ��ѣ�Ȼ���ٴ��ϴ��Ѹ��Ƶ�Ĭ�϶ѣ��Զ�����CopyTextureRegion�������ϴ��ѵ�Ĭ�϶�
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_DefaultTextureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
		m_CommandList->ResourceBarrier(1, &barrier);
		m_CommandList->Close();
		// ���ڴ��������õ���ʱ ID3D12CommandList ����
		ID3D12CommandList* _temp_cmdlists[] = { m_CommandList.Get() };

		// �ύ�������GPU ��ʼ���ƣ�
		m_CommandQueue->ExecuteCommandLists(1, _temp_cmdlists);

		// ��Χ��Ԥ��ֵ�趨Ϊ��һ֡��ע�⸴����ԴҲ��ҪΧ���ȴ�������ᷢ����Դ��ͻ
		FenceValue++;
		// ��������� (��������� GPU ��) ����Χ��Ԥ��ֵ�����������뵽���������
		// �������ִ�е�������޸�Χ��ֵ����ʾ��������ɣ�"����"Χ��
		m_CommandQueue->Signal(m_Fence.Get(), FenceValue);
		// ����Χ����Ԥ���¼������������ʱ��Χ����"����"������Ԥ���¼������¼������ź�״̬ת�������ź�״̬
		m_Fence->SetEventOnCompletion(FenceValue, RenderEvent);

	}


	void CreateSRV()
	{
		CD3DX12_SHADER_RESOURCE_VIEW_DESC SRVDescriptorDesc{};
		SRVDescriptorDesc.Format = TextureFormat;
		SRVDescriptorDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDescriptorDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDescriptorDesc.Texture2D.MipLevels = 1;//�ܷ��ʵ� Mip �㼶��

		SRV_CPUHandle = m_SRVHeap->GetCPUDescriptorHandleForHeapStart();
		m_D3D12Device->CreateShaderResourceView(m_DefaultTextureResource.Get(), &SRVDescriptorDesc, SRV_CPUHandle);
		//CPU �������������������������GPU �����������GPU��ʹ����Դ
		SRV_GPUHandle = m_SRVHeap->GetGPUDescriptorHandleForHeapStart();


	}

	void CreateCBVResource()
	{
		UINT CBufferWidth = Ceil(sizeof(CBuffer), 256) * 256;

		CD3DX12_RESOURCE_DESC CBVResourceDesc{};
		CBVResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		//D3D12_TEXTURE_LAYOUT_UNKNOWN�������Լ���ѡ���Ų���
		CBVResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;//�˴���D3D12_TEXTURE_LAYOUT_UNKNOWN��һ���ģ���˼�����Բ���
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
		m_CBVResource->Map(0, nullptr, reinterpret_cast<void**>(&MVPBuffer));//�õ�CB��ӳ���ַ


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
		if (ErrorBlob)		// �����ɫ���������ErrorBlob �����ṩ������Ϣ
		{
			OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
			OutputDebugStringA("\n");
		}
		// ����������ɫ�� Pixel Shader
		D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\FirstPersionView\\shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", NULL, NULL, &PixelShaderBlob, &ErrorBlob);
		if (ErrorBlob)		// �����ɫ���������ErrorBlob �����ṩ������Ϣ
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
			// �� FormatMessage ���� HRESULT Ϊ�ɶ��ַ���
			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,        // ��ģ��������ϵͳ�����ȡ��
				hr,             // Ҫ������ HRESULT
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // ���ԣ����ԣ�
				errorMsg,       // ���������
				_countof(errorMsg), // ��������С
				nullptr
			);

			// ƴ�Ӳ���ʾ������Ϣ
			std::wstringstream msg;
			msg << L"������Դʧ�ܣ�������: 0x" << std::hex << hr << L"\n" << errorMsg;
			MessageBox(nullptr, msg.str().c_str(), L"����", MB_OK | MB_ICONERROR);
			return;
		}
	}

	void CreateVertexResource()
	{
#pragma region ������Ϣ
		// CPU ���ٻ����ϵĶ�����Ϣ���飬ע�� DirectX ʹ�õ�����������ϵ��д������Ϣʱ�����һ��������֣�
		VERTEX vertexs[24] =
		{
			// ����
			{{0,2,0,1},{0,0}},
			{{2,2,0,1},{1,0}},
			{{2,0,0,1},{1,1}},
			{{0,0,0,1},{0,1}},

			// ����
			{{2,2,2,1},{0,0}},
			{{0,2,2,1},{1,0}},
			{{0,0,2,1},{1,1}},
			{{2,0,2,1},{0,1}},

			// ����
			{{0,2,2,1},{0,0}},
			{{0,2,0,1},{1,0}},
			{{0,0,0,1},{1,1}},
			{{0,0,2,1},{0,1}},

			// ����
			{{2,2,0,1},{0,0}},
			{{2,2,2,1},{1,0}},
			{{2,0,2,1},{1,1}},
			{{2,0,0,1},{0,1}},

			// ����
			{{0,2,2,1},{0,0}},
			{{2,2,2,1},{1,0}},
			{{2,2,0,1},{1,1}},
			{{0,2,0,1},{0,1}},

			// ����
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
			// �� FormatMessage ���� HRESULT Ϊ�ɶ��ַ���
			FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,        // ��ģ��������ϵͳ�����ȡ��
				hr,             // Ҫ������ HRESULT
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // ���ԣ����ԣ�
				errorMsg,       // ���������
				_countof(errorMsg), // ��������С
				nullptr
			);

			// ƴ�Ӳ���ʾ������Ϣ
			std::wstringstream msg;
			msg << L"����������Դʧ�ܣ�������: 0x" << std::hex << hr << L"\n" << errorMsg;
			MessageBox(nullptr, msg.str().c_str(), L"����", MB_OK | MB_ICONERROR);
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
		// �����������飬ע������� UINT == UINT32��������ĸ�ʽ (����) ������ DXGI_FORMAT_R32_UINT����������
		UINT IndexArray[36] =
		{
			// ����
			0,1,2,0,2,3,
			// ����
			4,5,6,4,6,7,
			// ����
			8,9,10,8,10,11,
			// ����
			12,13,14,12,14,15,
			// ����
			16,17,18,16,18,19,
			// ����
			20,21,22,20,22,23
		};

		CD3DX12_RESOURCE_DESC IndexDesc{};
		IndexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		IndexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; //����������һά�ģ�ֻ��������Դ����ʹ��D3D12_TEXTURE_LAYOUT_UNKNOWN;
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
		//������Դ����
		barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_RenderTarget[FrameIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);
		m_CommandList->ResourceBarrier(1, &barrier);
		//�����������
		m_CommandList->OMSetRenderTargets(1, &RTVHandle, false, nullptr);
		m_CommandList->ClearRenderTargetView(RTVHandle, DirectX::Colors::SkyBlue, 0, nullptr);
		//���ø�ǩ��
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
	// ��Ⱦѭ��
	void RenderLoop()
	{
		bool isExit = false;	// �Ƿ��˳�
		MSG msg = {};			// ��Ϣ�ṹ��

		while (isExit != true)
		{
			// MsgWaitForMultipleObjects ���ڶ���̵߳��������ȴ�������ֵ�Ǽ����¼� (�߳�) �� ID
			// �����ú����� RenderEvent Ҳ���Զ�����Ϊ���ź�״̬����Ϊ���Ǵ����¼���ʱ��ָ���˵ڶ�������Ϊ false
			DWORD ActiveEvent = ::MsgWaitForMultipleObjects(1, &RenderEvent, false, INFINITE, QS_ALLINPUT);

			switch (ActiveEvent - WAIT_OBJECT_0)
			{
			case 0:				// ActiveEvent �� 0��˵����Ⱦ�¼��Ѿ�����ˣ�������һ����Ⱦ
			{
				Render();
				Sleep(10);
			}
			break;


			case 1:				// ActiveEvent �� 1��˵����Ⱦ�¼�δ��ɣ�CPU ���߳�ͬʱ��������Ϣ����ֹ�������
			{
				// �鿴��Ϣ�����Ƿ�����Ϣ������оͻ�ȡ�� PM_REMOVE ��ʾ��ȡ����Ϣ�������̽�����Ϣ����Ϣ�������Ƴ�
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					// �������û���յ��˳���Ϣ���������ϵͳ�����ɷ���Ϣ������
					if (msg.message != WM_QUIT)
					{
						TranslateMessage(&msg);		// ������Ϣ�������̰��������ź� (WM_KEYDOWN)�������ⰴ��ֵת��Ϊ��Ӧ�� ASCII �룬ͬʱ���� WM_CHAR ��Ϣ
						DispatchMessage(&msg);		// �ɷ���Ϣ��֪ͨ����ϵͳ���ûص�����������Ϣ
					}
					else
					{
						isExit = true;							// �յ��˳���Ϣ�����˳���Ϣѭ��
					}
				}
			}
			break;


			case WAIT_TIMEOUT:	// ��Ⱦ��ʱ
			{

			}
			break;

			}
		}
	}

	// �ص������������ڲ�������Ϣ
	// WASD �� ���� WM_CHAR �ַ���Ϣ ���� �����ǰ�������ƶ�
	// ��곤������ƶ� ���� WM_MOUSEMOVE ����ƶ���Ϣ ���� ������ӽ���ת
	// �رմ��� ���� WM_DESTROY ����������Ϣ ���� ���ڹرգ���������˳�
	LRESULT CALLBACK CallBackFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// �� switch ���ڶ�������������ÿ�� case �ֱ��Ӧһ��������Ϣ
		switch (msg)
		{
		case WM_DESTROY:			// ���ڱ����� (���������Ͻ� X �رմ���ʱ)
		{
			PostQuitMessage(0);		// �����ϵͳ�����˳����� (WM_QUIT)��������Ϣѭ��
		}
		break;


		case WM_CHAR:	// ��ȡ���̲������ַ���Ϣ��TranslateMessage �Ὣ������뷭����ַ��룬ͬʱ���� WM_CHAR ��Ϣ
		{
			switch (wParam)		// wParam �ǰ�����Ӧ���ַ� ASCII ��
			{
			case 'w':
			case 'W':	// ��ǰ�ƶ�
				m_FirstCamera.Walk(0.1);
				break;

			case 's':
			case 'S':	// ����ƶ�
				m_FirstCamera.Walk(-0.1);
				break;

			case 'a':
			case 'A':	// �����ƶ�
				m_FirstCamera.Strafe(0.1);
				break;

			case 'd':
			case 'D':	// �����ƶ�
				m_FirstCamera.Strafe(-0.1);
				break;
			}
		}
		break;


		case WM_MOUSEMOVE:	// ��ȡ����ƶ���Ϣ
		{
			switch (wParam)	// wParam ����갴����״̬
			{
			case MK_LBUTTON:	// ���û�������������ͬʱ�ƶ���꣬�������ת
				m_FirstCamera.CameraRotate();
				break;

				// ����û�������ֻ���ƶ�ҲҪ���£�����ͻᷢ��������ӽ�˲��
			default: m_FirstCamera.UpdateLastCursorPos();
			}
		}
		break;


		// ������յ�������Ϣ��ֱ��Ĭ�Ϸ�����������
		default: return DefWindowProc(hwnd, msg, wParam, lParam);

		}

		return 0;	// ע�����default ����ķ�֧�������е���������Ҫ return 0������ͻ᷵��ϵͳ���ֵ�����´����޷�������ʾ
	}

	// ���д���
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


// ������
int WINAPI WinMain(HINSTANCE hins, HINSTANCE hPrev, LPSTR cmdLine, int cmdShow)
{
	DX12Engine::Run(hins);
}


