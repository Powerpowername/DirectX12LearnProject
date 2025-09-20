#include <Windows.h>
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
#include "directx/d3dx12.h"
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")				// ���� DXGI DLL
#pragma comment(lib,"dxguid.lib")			// ���� DXGI ��Ҫ���豸 GUID
#pragma comment(lib,"d3dcompiler.lib")		// ���� DX12 ��Ҫ����ɫ������ DLL
#pragma comment(lib,"windowscodecs.lib")	// ���� WIC DLL

using namespace Microsoft;
using namespace Microsoft::WRL;		// ʹ�� wrl.h ����������ռ䣬������Ҫ�õ������ Microsoft::WRL::ComPtr COM����ָ��
using namespace DirectX;			// DirectX �����ռ�
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
	static LRESULT CALLBACK CallBackFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return Broker_Func(hwnd, msg, wParam, lParam);
	}
};

// �������
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

	// ��������ҷ���ĵ�λ���������������ƶ���XMVector3Cross �����������
	XMVECTOR RightDirection = XMVector3Normalize(XMVector3Cross(ViewDirection, UpDirection));

	POINT LastCursorPoint = {};								// ��һ������λ��

	float FovAngleY = XM_PIDIV4;							// ��ֱ�ӳ���
	float AspectRatio = 4.0 / 3.0;							// ͶӰ���ڿ�߱�
	float NearZ = 0.1;										// ��ƽ�浽ԭ��ľ���
	float FarZ = 1000;										// Զƽ�浽ԭ��ľ���

	XMMATRIX ModelMatrix;									// ģ�;���ģ�Ϳռ� -> ����ռ�
	XMMATRIX ViewMatrix;									// �۲��������ռ� -> �۲�ռ�
	XMMATRIX ProjectionMatrix;								// ͶӰ���󣬹۲�ռ� -> ��βü��ռ�

	XMMATRIX MVPMatrix;										// MVP ����������Ҫ�ù��з��� GetMVPMatrix ��ȡ

public:

	Camera()	// ������Ĺ��캯��
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

	// ������һ�ε����λ��
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

	// ���� MVP ����
	void UpdateMVPMatrix()
	{
		// ��Ҫ�Ǹ��¹۲����
		ViewMatrix = XMMatrixLookAtLH(EyePosition, FocusPosition, UpDirection);
		MVPMatrix = ModelMatrix * ViewMatrix * ProjectionMatrix;
	}

	// ��ȡ MVP ����
	inline XMMATRIX& GetMVPMatrix()
	{
		// ÿ�η���ǰ��������һ��
		UpdateMVPMatrix();
		return MVPMatrix;
	}
};

// DX12 ����
class DX12Engine
{
private:
	int WindowWidth = 640;		// ���ڿ��
	int WindowHeight = 480;		// ���ڸ߶�
	HWND m_hwnd;				// ���ھ��

	ComPtr<ID3D12Debug> m_D3D12DebugDevice;
	UINT m_DXGICreateFactoryFlag = 0;
	
	ComPtr<IDXGIFactory5> m_DXGIFactory;
	ComPtr<IDXGIAdapter1> m_DXGIAdapter;
	ComPtr<ID3D12Device4> m_D3D12Device;

	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_CommadAllocator;
	ComPtr<ID3D12CommandList> m_CommadList;


	ComPtr<IDXGISwapChain3> m_DXGISwapChain;
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap;
	ComPtr<ID3D12Resource> m_RenderTarget[3];
	CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle;
	UINT RTVDescriptorSize = 0;
	UINT FrameIndex = 0;
	
	ComPtr<ID3D12Fence> m_Fence;
	UINT64 FenceValue = 0;
	HANDLE RenderEvent;
	CD3DX12_RESOURCE_BARRIER barrier;

	std::wstring TextureFilename = L"diamond_ore.png";		// �����ļ��� (�����õ������·��)
	ComPtr<IWICImagingFactory> m_WICFactory;				// WIC ����
	ComPtr<IWICBitmapDecoder> m_WICBitmapDecoder;			// λͼ������
	ComPtr<IWICBitmapFrameDecode> m_WICBitmapDecodeFrame;	// �ɽ������õ��ĵ���λͼ֡
	ComPtr<IWICFormatConverter> m_WICFormatConverter;		// λͼת����
	ComPtr<IWICBitmapSource> m_WICBitmapSource;				// WIC λͼ��Դ�����ڻ�ȡλͼ����
	UINT TextureWidth = 0;									// ������
	UINT TextureHeight = 0;									// ����߶�
	UINT BitsPerPixel = 0;									// ͼ����ȣ�ͼƬÿ������ռ�õı�����
	UINT BytePerRowSize = 0;								// ����ÿ�����ݵ���ʵ�ֽڴ�С�����ڶ�ȡ�������ݡ��ϴ�������Դ
	DXGI_FORMAT TextureFormat = DXGI_FORMAT_UNKNOWN;		// �����ʽ
		
	ComPtr<ID3D12Heap> SRVHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE SRV_CPUHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE SRV_GPUHandle;

	ComPtr<ID3D12Resource> m_UploadTextureResource;
	ComPtr<ID3D12Resource> m_DefaultTextureResource;

	UINT TextureSize = 0;									// �������ʵ��С (��λ���ֽ�)
	UINT UploadResourceRowSize = 0;							// �ϴ�����Դÿ�еĴ�С (��λ���ֽ�)
	UINT UploadResourceSize = 0;							// �ϴ�����Դ���ܴ�С (��λ���ֽ�)

	ComPtr<ID3D12Resource> m_CBVResource;
	struct CBuffer								// ��������ṹ��
	{
		XMFLOAT4X4 MVPMatrix;		// MVP �������ڽ��������ݴӶ���ռ�任����βü��ռ�
	};
	CBuffer* MVPBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView{};

	ComPtr<ID3D12Resource> m_VertexResource;
	struct VERTEX											// �������ݽṹ��
	{
		XMFLOAT4 position;									// ����λ��
		XMFLOAT2 texcoordUV;								// ������������
	};
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView{};
	

	// �ӿ�
	D3D12_VIEWPORT viewPort = D3D12_VIEWPORT{ 0, 0, float(WindowWidth), float(WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	// �ü�����
	D3D12_RECT ScissorRect = D3D12_RECT{ 0, 0, WindowWidth, WindowHeight };

public:

};