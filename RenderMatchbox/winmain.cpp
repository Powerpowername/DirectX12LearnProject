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
#include <unordered_map>
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

struct VERTEX
{
	XMFLOAT4 position;			// ������ģ������ϵ������
	XMFLOAT2 texcoordUV;		// �������� UV ����
};
// ģ���࣬���Ǹ������࣬���������麯������������Ҫʵ�������������麯�����ܴ���ʵ��
class Model
{ 
protected:
	XMMATRIX ModelMatrix = XMMatrixIdentity();
	ComPtr<ID3D12Resource> m_VertexResource;
	ComPtr<ID3D12Resource> m_IndexResource;
	ComPtr<ID3D12Resource> m_ModelMatrixResource;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView[2];
    D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	// ������ - GPU ���ӳ������������������ø�����
	std::unordered_map<std::string, D3D12_GPU_DESCRIPTOR_HANDLE> Texture_GPUHandle_Map;
	void AppendTextureKey(std::string&& TextureName)
	{
		Texture_GPUHandle_Map[TextureName] = {};
	}

public:
	// �����ȡģ����Ҫ����������ӳ����ֻ������
	const std::unordered_map<std::string, D3D12_GPU_DESCRIPTOR_HANDLE>& RequestForTextureMap()
	{
		return Texture_GPUHandle_Map;
	}

	// ģ�ͻ�ȡ�����Ѿ��������� SRV �������� SRVHandle
	void SetTextureGPUHandle(std::string TextureName, D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle)
	{
		Texture_GPUHandle_Map[TextureName] = GPUHandle;
	}

	// ��ȡģ�;���
	XMMATRIX GetModelMatrix()
	{
		return ModelMatrix;
	}

	// ����ģ�;���
	void SetModelMatrix(XMMATRIX Matrix)
	{
		ModelMatrix = Matrix;
	}

	// ������Դ��������������Ǵ��麯����ʵ������Ҫʵ��
	virtual void CreateResourceAndDescriptor(ComPtr<ID3D12Device4>& pD3D12Device) = 0;

	// ����ģ�ͣ����Ҳ�Ǵ��麯����ʵ������Ҫʵ��
	virtual void DrawModel(ComPtr<ID3D12GraphicsCommandList>& pCommandList) = 0;
};

class SoildBlock : public Model
{ 
	inline static VERTEX VertexArray[24] =
	{
		// ����
		{{0,1,0,1},{0,0}},
		{{1,1,0,1},{1,0}},
		{{1,0,0,1},{1,1}},
		{{0,0,0,1},{0,1}},

		// ����
		{{1,1,1,1},{0,0}},
		{{0,1,1,1},{1,0}},
		{{0,0,1,1},{1,1}},
		{{1,0,1,1},{0,1}},

		// ����
		{{0,1,1,1},{0,0}},
		{{0,1,0,1},{1,0}},
		{{0,0,0,1},{1,1}},
		{{0,0,1,1},{0,1}},

		// ����
		{{1,1,0,1},{0,0}},
		{{1,1,1,1},{1,0}},
		{{1,0,1,1},{1,1}},
		{{1,0,0,1},{0,1}},

		// ����
		{{0,1,1,1},{0,0}},
		{{1,1,1,1},{1,0}},
		{{1,1,0,1},{1,1}},
		{{0,1,0,1},{0,1}},

		// ����
		{{0,0,0,1},{0,0}},
		{{1,0,0,1},{1,0}},
		{{1,0,1,1},{1,1}},
		{{0,0,1,1},{0,1}}
	};
	// ע������� UINT == UINT32��������ĸ�ʽ (����) ������ DXGI_FORMAT_R32_UINT����������
	inline static UINT IndexArray[36] =
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


public:
	virtual void CreateResourceAndDescriptor(ComPtr<ID3D12Device4>& pD3D12Device) override
	{
		// ��ʱ���� XMFLOAT4X4 ���͵�ģ�;���XMFLOAT4X4 �ó��洢�봫�ݣ�XMMATRIX �ó���������
		XMFLOAT4X4 _temp_ModelMatrix = {};
		XMStoreFloat4x4(&_temp_ModelMatrix, ModelMatrix);

		// ������������ģ�;���� vector��vector �ĵײ���һ�������ڴ棬memcpy ���������ڴ��� CPU �Ż����ܿ�ܶ�
		std::vector<XMFLOAT4X4> _temp_ModelMatrixGroup;
		// ������� ModelMatrix �� ModelMatrixGroup
		_temp_ModelMatrixGroup.assign(24, _temp_ModelMatrix);

		//����ģ�;�����Դ
		CD3DX12_RESOURCE_DESC UploadResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(XMFLOAT4X4) * 24);
		CD3DX12_HEAP_PROPERTIES HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		pD3D12Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE,
			&UploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_ModelMatrixResource));
		//�޸���Դ��С������������Դ
		UploadResourceDesc.Width = sizeof(VERTEX) * 24;
		pD3D12Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE,
			&UploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexResource));
		//�޸���Դ��С������������Դ
		UploadResourceDesc.Width = sizeof(UINT) * 36;
		pD3D12Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE,
			&UploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexResource));

		//�洢������Դ
		BYTE* TransmitPointer = nullptr;
		m_VertexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransmitPointer));
		memcpy(TransmitPointer, _temp_ModelMatrixGroup.data(), sizeof(XMFLOAT4X4) * 24);
		m_VertexResource->Unmap(0, nullptr);
		//�洢������Դ
		m_IndexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransmitPointer));
        memcpy(TransmitPointer, IndexArray, sizeof(UINT) * 36);
        m_IndexResource->Unmap(0, nullptr);
		//�洢������Դ
		m_ModelMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&TransmitPointer));
		memcpy(TransmitPointer, &_temp_ModelMatrixGroup[0], 24 * sizeof(XMFLOAT4X4));
		m_ModelMatrixResource->Unmap(0, nullptr);

		VertexBufferView[0].BufferLocation = m_VertexResource->GetGPUVirtualAddress();
		VertexBufferView[0].SizeInBytes = 24 * sizeof(VERTEX);
		VertexBufferView[0].StrideInBytes = sizeof(VERTEX);

		VertexBufferView[1].BufferLocation = m_ModelMatrixResource->GetGPUVirtualAddress();
		VertexBufferView[1].SizeInBytes = 24 * sizeof(XMFLOAT4X4);
		VertexBufferView[1].StrideInBytes = sizeof(XMFLOAT4X4);

		IndexBufferView.BufferLocation = m_IndexResource->GetGPUVirtualAddress();
		IndexBufferView.SizeInBytes = 36 * sizeof(UINT);
		IndexBufferView.Format = DXGI_FORMAT_R32_UINT;


	}
};


// DX12 ����
class DX12Engine
{
private:
	int WindowWidth = 640;		// ���ڿ��
	int WindowHeight = 480;		// ���ڸ߶�
	HWND m_hwnd;				// ���ھ��

	ComPtr<ID3D12Debug> m_D3D12DebugDevice;					// D3D12 ���Բ��豸
	UINT m_DXGICreateFactoryFlag = NULL;					// ���� DXGI ����ʱ��Ҫ�õ��ı�־

	ComPtr<IDXGIFactory5> m_DXGIFactory;					// DXGI ����
	ComPtr<IDXGIAdapter1> m_DXGIAdapter;					// ��ʾ������ (�Կ�)
	ComPtr<ID3D12Device4> m_D3D12Device;					// D3D12 �����豸

	ComPtr<ID3D12CommandQueue> m_CommandQueue;				// �������
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;		// ���������
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;		// �����б�

	ComPtr<IDXGISwapChain3> m_DXGISwapChain;				// DXGI ������
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap;					// RTV ��������
	ComPtr<ID3D12Resource> m_RenderTarget[3];				// ��ȾĿ�껺�������飬ÿһ����Ⱦ�����Ӧһ�����ڻ�����
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle;					// RTV ���������
	UINT RTVDescriptorSize = 0;								// RTV �������Ĵ�С
	UINT FrameIndex = 0;									// ֡��������ʾ��ǰ��Ⱦ�ĵ� i ֡ (�� i ����ȾĿ��)

	ComPtr<ID3D12Fence> m_Fence;							// Χ��
	UINT64 FenceValue = 0;									// ����Χ���ȴ���Χ��ֵ
	HANDLE RenderEvent = NULL;								// GPU ��Ⱦ�¼�
	D3D12_RESOURCE_BARRIER beg_barrier = {};				// ��Ⱦ��ʼ����Դ���ϣ����� -> ��ȾĿ��
	D3D12_RESOURCE_BARRIER end_barrier = {};				// ��Ⱦ��������Դ���ϣ���ȾĿ�� -> ����

	ComPtr<ID3D12DescriptorHeap> m_DSVHeap;					// DSV ��������
	D3D12_CPU_DESCRIPTOR_HANDLE DSVHandle;					// DSV ���������
	ComPtr<ID3D12Resource> m_DepthStencilBuffer;			// DSV ���ģ�建����Դ

	// DSV ��Դ�ĸ�ʽ
	// ���ģ�建��ֻ֧�����ָ�ʽ:
	// DXGI_FORMAT_D24_UNORM_S8_UINT	(ÿ������ռ���ĸ��ֽ� 32 λ��24 λ�޷��Ź�һ���������������ֵ��8 λ��������ģ��ֵ)
	// DXGI_FORMAT_D32_FLOAT_S8X24_UINT	(ÿ������ռ�ð˸��ֽ� 64 λ��32 λ�������������ֵ��8 λ��������ģ��ֵ������ 24 λ������ʹ��)
	// DXGI_FORMAT_D16_UNORM			(ÿ������ռ�������ֽ� 16 λ��16 λ�޷��Ź�һ���������������ֵ����Χ [0,1]����ʹ��ģ��)
	// DXGI_FORMAT_D32_FLOAT			(ÿ������ռ���ĸ��ֽ� 32 λ��32 λ�������������ֵ����ʹ��ģ��)
	// ��������ѡ����õĸ�ʽ DXGI_FORMAT_D24_UNORM_S8_UINT
	DXGI_FORMAT DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


	ModelManager m_ModelManager;							// ģ�͹�����������������Ⱦģ��

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

	ComPtr<ID3D12DescriptorHeap> m_SRVHeap;					// SRV ��������

	UINT TextureSize = 0;									// �������ʵ��С (��λ���ֽ�)
	UINT UploadResourceRowSize = 0;							// �ϴ�����Դÿ�еĴ�С (��λ���ֽ�)
	UINT UploadResourceSize = 0;							// �ϴ�����Դ���ܴ�С (��λ���ֽ�)



	ComPtr<ID3D12Resource> m_CBVResource;		// ����������Դ�����ڴ�� MVP ����MVP ����ÿ֡��Ҫ���£�������Ҫ�洢�ڳ�����������
	struct CBuffer								// ��������ṹ��
	{
		XMFLOAT4X4 MVPMatrix;		// MVP �������ڽ��������ݴӶ���ռ�任����βü��ռ�
	};
	CBuffer* MVPBuffer = nullptr;	// ��������ṹ��ָ�룬����洢���� MVP ������Ϣ������ Map ��ָ���ָ�� CBVResource �ĵ�ַ

	Camera m_FirstCamera;			// ��һ�˳������

	ComPtr<ID3D12RootSignature> m_RootSignature;			// ��ǩ��
	ComPtr<ID3D12PipelineState> m_PipelineStateObject;		// ��Ⱦ����״̬


	// �ӿ�
	D3D12_VIEWPORT viewPort = D3D12_VIEWPORT{ 0, 0, float(WindowWidth), float(WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	// �ü�����
	D3D12_RECT ScissorRect = D3D12_RECT{ 0, 0, WindowWidth, WindowHeight };
public:
	void InitWindow(HINSTANCE hins)
	{
		WNDCLASS  wc = {};
		wc.hInstance = hins;
		CallBackWrapper::Broker_Func = std::bind(&DX12Engine::CallBackFunc, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		wc.lpfnWndProc = CallBackWrapper::CallBackFunc;
		wc.lpszClassName = L"DX12Window";
		RegisterClass(&wc);

		m_hwnd = CreateWindow(wc.lpszClassName, L"Minecraft", WS_SYSMENU | WS_OVERLAPPED,
			10, 10, WindowWidth, WindowHeight,
			NULL, NULL, hins, NULL);

		ShowWindow(m_hwnd, SW_SHOW);
	}

	void CreateDebugDevice()
	{
		::CoInitialize(nullptr);	// ע�����DX12 �������豸�ӿڶ��ǻ��� COM �ӿڵģ�������Ҫ��ȫ����ʼ��Ϊ nullptr

#if defined(_DEBUG)
		// ��ȡ���Բ��豸�ӿ�
		D3D12GetDebugInterface(IID_PPV_ARGS(&m_D3D12DebugDevice));
		// �������Բ�
		m_D3D12DebugDevice->EnableDebugLayer();
		// �������Բ�󣬴��� DXGI ����Ҳ��Ҫ Debug Flag
		m_DXGICreateFactoryFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif
	}

	bool CreateDevice()
	{
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
					OutputDebugStringW(adap.Description);		// ������������������ D3D12 �豸���õ��Կ�����
					return true;
				}
			}
		}
		if (m_D3D12Device == nullptr)
		{
			MessageBox(NULL, L"�Ҳ����κ���֧�� DX12 ���Կ��������������ϵ�Ӳ����", L"����", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	void CreateCommandComponent()
	{ 
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		m_D3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));
        m_D3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
		m_D3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList));
		m_CommandList->Close();
	}

	void CreateRenderTarget()
	{
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = 3;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_D3D12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RTVHeap));
		
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = 3;
        swapChainDesc.Width = WindowWidth;
        swapChainDesc.Height = WindowHeight;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
	    
		ComPtr<IDXGISwapChain1> _temp_swapchain;
		m_DXGIFactory->CreateSwapChainForHwnd(m_CommandQueue.Get(), m_hwnd, &swapChainDesc, nullptr, nullptr, &_temp_swapchain);
        _temp_swapchain.As(&m_DXGISwapChain);

		RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
		RTVDescriptorSize = m_D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		for (int i = 0; i < 3; i++)
		{
			m_DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i]));
			m_D3D12Device->CreateRenderTargetView(m_RenderTarget[i].Get(), nullptr, RTVHandle);
            RTVHandle.ptr += RTVDescriptorSize;
		}
	}
	void CreateFence()
	{
		RenderEvent = CreateEvent(nullptr, false, true, nullptr);
		m_D3D12Device->CreateFence(FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
	}

	void CreatDSVHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc{};
		DSVHeapDesc.NumDescriptors = 1;
		DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		m_D3D12Device->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&m_DSVHeap));

		DSVHandle = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

	}

	void CreateDepthStencilBuffer()
	{
		CD3DX12_RESOURCE_DESC DSVResourceDesc{};
		DSVResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DSVFormat,
			WindowWidth,
			WindowHeight,
			1, 1, 1, 0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
			D3D12_TEXTURE_LAYOUT_UNKNOWN
		);

		CD3DX12_CLEAR_VALUE DepthStencilBufferClearValue(
			DSVFormat,
			1.0f,
			0.0f
		);

		CD3DX12_HEAP_PROPERTIES DefaultProperties(D3D12_HEAP_TYPE_DEFAULT);

		m_D3D12Device->CreateCommittedResource(&DefaultProperties, D3D12_HEAP_FLAG_NONE,
			&DSVResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &DepthStencilBufferClearValue, IID_PPV_ARGS(&m_DepthStencilBuffer));
	}

	void CreatDSV()
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc{};
        DSVDesc.Format = DSVFormat;
        DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        DSVDesc.Flags = D3D12_DSV_FLAG_NONE;
		m_D3D12Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &DSVDesc, DSVHandle);
	}

	void CreateSRVHeap()
	{
		D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc{};
		SRVHeapDesc.NumDescriptors = m_ModelManager.Texture_SRV_Map.size();
        SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		m_D3D12Device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(&m_SRVHeap));

	}

	void StartCommandRecord()
	{
		m_CommandAllocator->Reset();
        m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
	}

	// ���������ڴ���
	bool LoadTextureFromFile(std::wstring TextureFilename)
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

	// ��ȡ���㷨���� A ����ȡ�����ж�����Ҫ���ٸ�����Ϊ B �Ŀռ�������� A�������ڴ����
	inline UINT Ceil(UINT A, UINT B)
	{
		return (A + B - 1) / B;
	}

	void CreateUploadAndDefaultResource(ModelManager::TEXTURE_MAP_INFO& Info)
	{
		BytePerRowSize = TextureWidth * BitsPerPixel / 8;
		TextureSize = BytePerRowSize * TextureHeight;
		UploadResourceRowSize = Ceil(BytePerRowSize, 256) * 256;
		UploadResourceSize = UploadResourceRowSize * (TextureHeight - 1) + BytePerRowSize;

		CD3DX12_HEAP_PROPERTIES HeapProps(D3D12_HEAP_TYPE_DEFAULT);
        CD3DX12_RESOURCE_DESC UploadHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(UploadResourceSize);

		m_D3D12Device->CreateCommittedResource(
            &HeapProps,
            D3D12_HEAP_FLAG_NONE,
            &UploadHeapDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&Info.UploadHeapTextureResource));

		CD3DX12_HEAP_PROPERTIES HeapProps1(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC DefaultHeapDesc = CD3DX12_RESOURCE_DESC::Buffer(UploadResourceSize);

		m_D3D12Device->CreateCommittedResource(
			&HeapProps1,
			D3D12_HEAP_FLAG_NONE,
			&DefaultHeapDesc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&Info.DefaultHeapTextureResource)
		);

	}





};