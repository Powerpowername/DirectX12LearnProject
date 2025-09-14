// (4) DrawTexture���� DirectX 12 ��һ����ʯԭ��

#include<Windows.h>			// Windows ���ڱ�̺���ͷ�ļ�
#include<d3d12.h>			// DX12 ����ͷ�ļ�
#include<dxgi1_6.h>			// DXGI ͷ�ļ������ڹ����� DX12 �������������Ҫ�豸���� DXGI ������ ������
#include<DirectXColors.h>	// DirectX ��ɫ��
#include<DirectXMath.h>		// DirectX ��ѧ��
#include<d3dcompiler.h>		// DirectX Shader ��ɫ�������
#include<wincodec.h>		// WIC ͼ�����ܣ����ڽ������ת��ͼƬ�ļ�
#include"d3dx12.h"
#include<wrl.h>				// COM ���ģ��⣬����д DX12 �� DXGI ��صĽӿ�
#include<string>			// C++ ��׼ string ��
#include<sstream>			// C++ �ַ����������

#pragma comment(lib,"d3d12.lib")			// ���� DX12 ���� DLL
#pragma comment(lib,"dxgi.lib")				// ���� DXGI DLL
#pragma comment(lib,"dxguid.lib")			// ���� DXGI ��Ҫ���豸 GUID
#pragma comment(lib,"d3dcompiler.lib")		// ���� DX12 ��Ҫ����ɫ������ DLL
#pragma comment(lib,"windowscodecs.lib")	// ���� WIC DLL

using namespace Microsoft::WRL;
using namespace DirectX;

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


class DX12Engine
{
private:
#pragma region ���Ա
	int WindowWidth = 640;		// ���ڿ��
	int WindowHeight = 640;		// ���ڸ߶�
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
	ComPtr<ID3D12Resource> m_RenderTarget[3];				// ��ȾĿ�����飬ÿһ����ȾĿ���Ӧһ�����ڻ�����
	CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle;					// RTV ���������
	UINT RTVDescriptorSize = 0;								// RTV �������Ĵ�С
	UINT FrameIndex = 0;									// ֡��������ʾ��ǰ��Ⱦ�ĵ� i ֡ (�� i ����ȾĿ��)

	ComPtr<ID3D12Fence> m_Fence;							// Χ��
	UINT64 FenceValue = 0;									// ����Χ���ȴ���Χ��ֵ
	HANDLE RenderEvent = NULL;								// GPU ��Ⱦ�¼�
	CD3DX12_RESOURCE_BARRIER barrier = {};				// ��Ⱦ��ʼ����Դ���ϣ����� -> ��ȾĿ��
	//CD3DX12_RESOURCE_BARRIER end_barrier = {};				// ��Ⱦ��������Դ���ϣ���ȾĿ�� -> ����

	std::wstring TextureFilename = L"./diamond_ore.png";		// �����ļ��� (�����õ������·��)
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
	CD3DX12_CPU_DESCRIPTOR_HANDLE SRV_CPUHandle;				// SRV ������ CPU ���
	CD3DX12_GPU_DESCRIPTOR_HANDLE SRV_GPUHandle;				// SRV ������ GPU ���

	ComPtr<ID3D12Resource> m_UploadTextureResource;			// �ϴ�����Դ��λ�ڹ����ڴ棬������ת������Դ
	ComPtr<ID3D12Resource> m_DefaultTextureResource;		// Ĭ�϶���Դ��λ���Դ棬���ڷ�����
	UINT TextureSize = 0;									// �������ʵ��С (��λ���ֽ�)
	UINT UploadResourceRowSize = 0;							// �ϴ�����Դÿ�еĴ�С (��λ���ֽ�)
	UINT UploadResourceSize = 0;							// �ϴ�����Դ���ܴ�С (��λ���ֽ�)

	ComPtr<ID3D12RootSignature> m_RootSignature;			// ��ǩ��
	ComPtr<ID3D12PipelineState> m_PipelineStateObject;		// ��Ⱦ����״̬


	ComPtr<ID3D12Resource> m_VertexResource;				// ������Դ
	struct VERTEX											// �������ݽṹ��
	{
		XMFLOAT4 position;									// ����λ��
		XMFLOAT2 texcoordUV;								// ������������
	};
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;				// ���㻺��������

	// �ӿ�
	CD3DX12_RECT ScissorRect = CD3DX12_RECT{ 0, 0, WindowWidth, WindowHeight };
	CD3DX12_VIEWPORT viewPort = CD3DX12_VIEWPORT{ 0.0f, 0.0f, float(WindowWidth), float(WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	// �ü�����
#pragma endregion
public:
	// ��ʼ������
	void InitWindow(HINSTANCE hins);
	// �������Բ�
	void CreateDebugDevice();
	// �����豸
	bool CreateDevice();
	// ��������������
	void CreateCommandComponents();
	// ������ȾĿ�꣬����ȾĿ������Ϊ����
	void CreateRenderTarget();
	// ����Χ������Դ���ϣ����� CPU-GPU ��ͬ��
	void CreateFenceAndBarrier();
	// ���������ڴ���
	bool LoadTextureFromFile();//Ŀǰ�����о��ص㣬�������о�
	// ���� SRV Descriptor Heap ��ɫ����Դ��������
	void CreateSRVHeap();
	// ��ȡ���㷨���� A ����ȡ�����ж�����Ҫ���ٸ�����Ϊ B �Ŀռ�������� A�������ڴ����
	UINT Ceil(UINT A, UINT B);
	// ���������ϴ��� UploadResource �����ڷ������ DefaultResource
	void CreateUploadAndDefaultResource();
	// ��������з���������������ݸ��Ƶ� DefaultResource
	void CopyTextureDataToDefaultResource();
	// ���մ��� SRV ��ɫ����Դ���������������� DefaultResource Ϊһ������
	void CreateSRV();
	// ������ǩ��
	void CreateRootSignature();
	// ������Ⱦ����״̬���� (Pipeline State Object, PSO)
	void CreatePSO();
	// ����������Դ
	void CreateVertexResource();
	// ��Ⱦ
	void Render();
	// ��Ⱦѭ��
	void RenderLoop();
	// �ص�����
	static LRESULT CALLBACK CallBackFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	// ���д���
	static void Run(HINSTANCE hins);

};

inline void DX12Engine::InitWindow(HINSTANCE hins)
{
	// ע�ᴰ����
	WNDCLASS wc = {};
	wc.lpfnWndProc = CallBackFunc;	// ���ڹ��̺�����������Ĭ�ϵ�
	wc.hInstance = hins;				// Ӧ�ó���ʵ�����
	wc.lpszClassName = L"DX12WndClass";	// ����������
	RegisterClass(&wc);
	// ��������
	m_hwnd = CreateWindowEx(
		0,								// ��չ��ʽ
		wc.lpszClassName,				// ����������
		L"DirectX 12 Draw Texture",		// ���ڱ���
		WS_OVERLAPPEDWINDOW,			// ���ڷ���ص����ڣ������������߿�ϵͳ�˵���
		CW_USEDEFAULT, CW_USEDEFAULT,	// ���ڳ�ʼλ�� (x, y)��ϵͳĬ��λ��
		WindowWidth, WindowHeight,		// ���ڿ�Ⱥ͸߶�
		NULL,							// �����ھ��
		NULL,							// �˵����
		hins,							// Ӧ�ó���ʵ�����
		NULL							// �������
	);
	if (!m_hwnd)
	{
		MessageBox(NULL, L"��������ʧ�ܣ�", L"����", MB_OK);
		return;
	}
	ShowWindow(m_hwnd, SW_SHOW); // ��ʾ����
}



inline void DX12Engine::CreateDebugDevice()
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
bool DX12Engine::CreateDevice()
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

void DX12Engine::CreateCommandComponents()
{
	D3D12_COMMAND_QUEUE_DESC  queueDesc{};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // ֱ��������У�֧���������͵�����
	//queueDesc.Flags = CD3DX12_COMMAND_QUEUE_FLAG_NONE; // սδ������ʱûʲô�ã�û�������־
	m_D3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
	//��������������������ǿ����ڴ�Ϊ�����б�����ڴ�
	m_D3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocator.GetAddressOf()));
	// ���������б�
	m_D3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
	//�����������б��Ĭ�ϴ��ڴ�״̬��������Ҫ�ر������ȴ�����ʹ��
	m_CommandList->Close();
}

void DX12Engine::CreateRenderTarget()
{
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
	RTVHeapDesc.NumDescriptors = 3; // ����������
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RTV ��������
	m_D3D12Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_RTVHeap));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 3; // ����������
	swapChainDesc.Width = WindowWidth; // ���ڿ��
	swapChainDesc.Height = WindowHeight; // ���ڸ߶�
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ��������ʽ��8 λ RGBA ��ʽ
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ������ȾĿ�����
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ��ת����ģʽ
	swapChainDesc.SampleDesc.Count = 1; // ���ز���������1 ��ʾ��ʹ�ö��ز���
	
	// ��ʱ�Ͱ汾�������ӿڣ����ڴ����߰汾����������Ϊ���ĵ� CreateSwapChainForHwnd ����ֱ�����ڴ����߰汾�ӿ�
	ComPtr<IDXGISwapChain1> _temp_swapchain;

	m_DXGIFactory->CreateSwapChainForHwnd(
		m_CommandQueue.Get(),	// ��������Ҫ���������
		m_hwnd,					// �󶨵Ĵ��ھ��
		&swapChainDesc,			// ������������
		nullptr,				// ȫ����ʾ������nullptr ��ʾ��Ĭ��ֵ
		nullptr,				// ���������豸��nullptr ��ʾ������
		_temp_swapchain.GetAddressOf() // ��� IDXGISwapChain1 �ӿ�
	);
	// ͨ�� As ���������Ͱ汾�ӿڵ���Ϣ���ݸ��߰汾�ӿ�
	_temp_swapchain.As(&m_DXGISwapChain);
	// �����꽻���������ǻ���Ҫ�� RTV ������ ָ�� ��ȾĿ��
	// ��Ϊ ID3D12Resource ������ֻ��һ�����ݣ�������û�ж������÷���˵��
	// ����Ҫ�ó���֪�����������һ����ȾĿ�꣬�͵ô�����ʹ�� RTV ������

	// ��ȡ RTV ��ָ�����������ľ��
	RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	// ��ȡ RTV �������Ĵ�С
	RTVDescriptorSize = m_D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (UINT i = 0;i < 3;i++)
	{
		// �ӽ������л�ȡ�� i �����ڻ��壬������ i �� RenderTarget ��ȾĿ��
		m_DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i]));
		m_D3D12Device->CreateRenderTargetView(m_RenderTarget[i].Get(), nullptr, RTVHandle);
		// ÿ��ƫ��һ����������С��ָ����һ��������
		RTVHandle.Offset(1, RTVDescriptorSize);
	}
}

void DX12Engine::CreateFenceAndBarrier()
{
	// ���� CPU �ϵĵȴ��¼�
	RenderEvent = CreateEvent(nullptr, false, true, nullptr);
	// ����Χ�����趨��ʼֵΪ 0
	m_D3D12Device->CreateFence(FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));


	//// ������Դ����
	//// beg_barrier ��ʼ���ϣ�Present ����״̬ -> Render Target ��ȾĿ��״̬
	//beg_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;					// ָ������Ϊת������	
	//beg_barrier.Transition(m_RenderTarget[FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	//// end_barrier �������ϣ�Render Target ��ȾĿ��״̬ -> Present ����״̬
	//end_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//end_barrier.Transition(m_RenderTarget[FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

bool DX12Engine::LoadTextureFromFile()
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

void DX12Engine::CreateSRVHeap()
{
	// ���� SRV ��������
	D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc = {};
	SRVHeapDesc.NumDescriptors = 1;									// ����ֻ��һ������ֻ��Ҫ��һ�� SRV ������
	SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;		// �����������ͣ�CBV��SRV��UAV ���������������Է���ͬһ������������
	SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;	// �������ѱ�־��Shader-Visible ��ʾ����ɫ���ɼ�

	// ���� SRV ��������
	m_D3D12Device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(&m_SRVHeap));
}

// ��ȡ���㷨���� A ����ȡ�����ж�����Ҫ���ٸ�����Ϊ B �Ŀռ�������� A�������ڴ����
inline UINT DX12Engine::Ceil(UINT A, UINT B)
{
	return (A + B - 1) / B;
}

inline void DX12Engine::CreateUploadAndDefaultResource()
{
	// ��������ÿ�����ݵ���ʵ���ݴ�С (��λ��Byte �ֽ�)����Ϊ����ͼƬ���ڴ��������Դ洢��
	// ���ȡ�������ʵ��С����ȷ��ȡ�������ݡ��ϴ��� GPU�������Ȼ�ȡ����� BitsPerPixel ͼ����ȣ���Ϊ��ͬλͼ��ȿ��ܲ�ͬ
	// Ȼ���ټ���ÿ������ռ�õ��ֽڣ����� 8 ����Ϊ 1 Byte = 8 bits
	BytePerRowSize = TextureWidth * BitsPerPixel / 8;
	// �������ʵ��С (��λ���ֽ�)
	TextureSize = BytePerRowSize * TextureHeight;

	// �ϴ�����Դÿ�еĴ�С (��λ���ֽ�)��ע������Ҫ���� 256 �ֽڶ��룡
	// ��Ϊ GPU �� CPU �ܹ���ͬ��GPU ע�ز��м��㣬ע�ؽṹ�����ݵĿ��ٶ�ȡ����ȡ���ݶ����� 256 �ֽ�Ϊһ��������
	// ���Ҫ��Ҫ�� BytePerRowSize ���ж��룬�ж���Ҫ�ж����������������ÿ�����أ�������Ļ����ݻ����ġ�
	UploadResourceRowSize = Ceil(BytePerRowSize, 256) * 256;//�����ʵ��̫�鷳�ˣ����Կ������ϵ�ʵ��
	// �ϴ�����Դ���ܴ�С (��λ���ֽ�)������ռ����ֻ�಻�٣�����ᱨ D3D12 MinimumAlloc Error ��Դ�ڴ洴������
	// ע�����һ�в����ڴ���� (��Ϊ����û�������ˣ������ڴ����Ҳ����ȷ��ȡ)������Ҫ (TextureHeight - 1) �ټ� BytePerRowSize
	UploadResourceSize = UploadResourceRowSize * (TextureHeight - 1) + BytePerRowSize;

	// ������ת������ϴ�����Դ�ṹ��
	CD3DX12_RESOURCE_DESC UploadResourceDesc{};
	UploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;		// ��Դ���ͣ��ϴ��ѵ���Դ���Ͷ��� buffer ����
	UploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;			// ��Դ���֣�ָ����Դ�Ĵ洢��ʽ���ϴ��ѵ���Դ���� row major �������Դ洢
	UploadResourceDesc.Width = UploadResourceSize;						// ��Դ��ȣ��ϴ��ѵ���Դ�������Դ���ܴ�С��ע����Դ��С����ֻ�಻��
	UploadResourceDesc.Height = 1;										// ��Դ�߶ȣ��ϴ��ѽ����Ǵ���������Դ�ģ����Ը߶ȱ���Ϊ 1
	UploadResourceDesc.Format = DXGI_FORMAT_UNKNOWN;					// ��Դ��ʽ���ϴ�����Դ�ĸ�ʽ����Ϊ UNKNOWN
	UploadResourceDesc.DepthOrArraySize = 1;							// ��Դ��ȣ������������������� 3D ����ģ��ϴ�����Դ����Ϊ 1
	UploadResourceDesc.MipLevels = 1;									// Mipmap �ȼ����������������ģ��ϴ�����Դ����Ϊ 1
	UploadResourceDesc.SampleDesc.Count = 1;							// ��Դ�����������ϴ�����Դ������ 1

	CD3DX12_HEAP_PROPERTIES UploadHeapDesc(D3D12_HEAP_TYPE_UPLOAD) ;
	//UploadHeapDesc.Type = D3D12_HEAP_TYPE_UPLOAD;					// �ϴ�������
	// �����ϴ�����Դ
	m_D3D12Device->CreateCommittedResource(
		&UploadHeapDesc,					// ������
		D3D12_HEAP_FLAG_NONE,
		&UploadResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_UploadTextureResource.GetAddressOf())
	);

	CD3DX12_RESOURCE_DESC DefaultResourceDesc{};
	DefaultResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	// ��Դ���ͣ�����ָ��Ϊ Texture2D 2D����
	DefaultResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;			// ������Դ�Ĳ��ֶ��� UNKNOWN
	DefaultResourceDesc.Width = TextureWidth;							// ��Դ��ȣ�������������
	DefaultResourceDesc.Height = TextureHeight;							// ��Դ�߶ȣ�����������߶�
	DefaultResourceDesc.Format = TextureFormat;							// ��Դ��ʽ�������������ʽ��Ҫ������һ��
	DefaultResourceDesc.DepthOrArraySize = 1;							// ��Դ��ȣ�����ֻ��һ������������ 1
	DefaultResourceDesc.MipLevels = 1;									// Mipmap �ȼ���������ʱ��ʹ�� Mipmap�������� 1
	DefaultResourceDesc.SampleDesc.Count = 1;							// ��Դ�������������������� 1 ����
	
	// Ĭ�϶����ԵĽṹ�壬Ĭ�϶�λ���Դ�
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

	// �������������ݶ��� TextureData �У�������ĵ� memcpy ���Ʋ���
	m_WICBitmapSource->CopyPixels(nullptr, BytePerRowSize, TextureSize, TextureData);
	// ���ڴ�����Դ��ָ��
	BYTE* TransferPointer = nullptr;
	// Map ��ʼӳ�䣬Map ������õ��ϴ�����Դ�ĵ�ַ (�ڹ����ڴ���)�����ݸ�ָ�룬�������Ǿ���ͨ�� memcpy ��������������
	m_UploadTextureResource->Map(0, nullptr, reinterpret_cast<void**>(&TransferPointer));

	// ���и����������ݵ��ϴ�����Դ��
	for (UINT i = 0; i < TextureHeight;i++)
	{
		// ���ϴ�����Դ���и����������� (CPU ���ٻ��� -> �����ڴ�)
		memcpy(TransferPointer, TextureData, BytePerRowSize);
		// ����ָ��ƫ�Ƶ���һ��
		TextureData += BytePerRowSize;
		// �ϴ�����Դָ��ƫ�Ƶ���һ�У�ע������Ҫ�� UploadResourceRowSize ƫ�ƣ���Ϊ�ϴ�����Դÿ���� 256 �ֽڶ����
		TransferPointer += UploadResourceRowSize;
	}

	// Unmap ����ӳ�䣬��Ϊ�����޷�ֱ�Ӷ�дĬ�϶���Դ����Ҫ�ϴ��Ѹ��Ƶ�����ڸ���֮ǰ��������Ҫ�Ƚ���ӳ�䣬���ϴ��Ѵ���ֻ��״̬
	m_UploadTextureResource->Unmap(0, nullptr);

	TextureData -= TextureSize;		// ������Դָ��ƫ�ƻس�ʼλ��
	free(TextureData);				// �ͷ����� malloc ����Ŀռ䣬���������ò���������Ҫ����ռ�ڴ�
	
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT PlacedFootprint = {};						// ��Դ�ű�����������Ҫ���Ƶ���Դ
	D3D12_RESOURCE_DESC DefaultResourceDesc = m_DefaultTextureResource->GetDesc();	// Ĭ�϶���Դ�ṹ��

	// ��ȡ�����ƽű����������ĵ�������
	m_D3D12Device->GetCopyableFootprints(&DefaultResourceDesc, 0, 1, 0, &PlacedFootprint, nullptr, nullptr, nullptr);
	D3D12_TEXTURE_COPY_LOCATION DstLocation = {};						// ����Ŀ��λ�� (Ĭ�϶���Դ) �ṹ��
	DstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;		// ���������ͣ��������ָ������
	DstLocation.SubresourceIndex = 0;									// ָ��Ҫ���Ƶ�����Դ����,��ʵ����MipMap�Ĳ㼶
	DstLocation.pResource = m_DefaultTextureResource.Get();				// Ҫ���Ƶ�����Դ

	D3D12_TEXTURE_COPY_LOCATION SrcLocation = {};						// ����Դλ�� (�ϴ�����Դ) �ṹ��
	SrcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;		// ���������ͣ��������ָ�򻺳���
	SrcLocation.PlacedFootprint = PlacedFootprint;						// ָ��Ҫ���Ƶ���Դ�ű���Ϣ
	SrcLocation.pResource = m_UploadTextureResource.Get();				// ���������ݵĻ���

	// ������Դ��Ҫʹ�� GPU �� CopyEngine �������棬������Ҫ��������з�����������
	m_CommandAllocator->Reset();								// ���������������
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);	// �����������б����������Ҫ PSO ״̬�����Եڶ��������� nullptr

	// ��¼������Դ��Ĭ�϶ѵ����� (�����ڴ� -> �Դ�) 
	m_CommandList->CopyTextureRegion(&DstLocation, 0, 0, 0, &SrcLocation, nullptr);
	// �ر������б�
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

inline void DX12Engine::CreateSRV()
{
	// SRV ��������Ϣ�ṹ��
	CD3DX12_SHADER_RESOURCE_VIEW_DESC SRVDescriptorDesc = {};
	// SRV ���������ͣ���������ָ�� Texture2D 2D����
	SRVDescriptorDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	// SRV �������ĸ�ʽҲҪ�������ʽ
	SRVDescriptorDesc.Format = TextureFormat;
	// ���������ÿ���������� RGBA ������˳��D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING ��ʾ������������˳�򲻸ı�
	SRVDescriptorDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDescriptorDesc.Texture2D.MipLevels =1;	// ������������ Mipmap �㼶������û�� Mipmap�������� 0
	SRVDescriptorDesc.Texture2D.MostDetailedMip = 0; // �������������ϸ Mipmap �㼶������û�� Mipmap�������� 0

	//ֻ����CPU��������������ΪGPUû�������������Ƴ������ģ�,�������������������������洢�ģ���Ƭ�洢�ռ����Դ���
	SRV_CPUHandle = m_SRVHeap->GetCPUDescriptorHandleForHeapStart(); // ��ȡ SRV ��������ָ������������ CPU ���
	// ���� SRV ������������ DefaultResource �����ԴΪһ������	
	m_D3D12Device->CreateShaderResourceView(m_DefaultTextureResource.Get(), &SRVDescriptorDesc, SRV_CPUHandle);

	// ��ȡ SRV �������� GPU ӳ���������������б����� SRVHeap �������ѣ���ɫ������ SRV ��������������Դ
	SRV_GPUHandle = m_SRVHeap->GetGPUDescriptorHandleForHeapStart();

}

inline void DX12Engine::CreateRootSignature()
{
	ComPtr<ID3DBlob> SignatureBlob;			// ��ǩ���ֽ���
	ComPtr<ID3DBlob> ErrorBlob;				// �����ֽ��룬��ǩ������ʧ��ʱ�� OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer()); ���Ի�ȡ������Ϣ

	CD3DX12_DESCRIPTOR_RANGE SRVDescriptorRangeDesc = {};	// Range ��������Χ�ṹ�壬һ�� Range ��ʾһ��������ͬ����������
	SRVDescriptorRangeDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;		// Range ���ͣ�����ָ�� SRV ���ͣ�CBV_SRV_UAV ���������
	SRVDescriptorRangeDesc.NumDescriptors = 1;								// Range ��������������� N��һ�ο��԰󶨶��������������Ĵ�������
	SRVDescriptorRangeDesc.BaseShaderRegister = 0;							// Range Ҫ�󶨵���ʼ�Ĵ����۱�� i���󶨷�Χ�� [s(i),s(i+N)]�����ǰ� s0
	SRVDescriptorRangeDesc.RegisterSpace = 0;								// Range Ҫ�󶨵ļĴ����ռ䣬Ĭ�϶��� 0
	SRVDescriptorRangeDesc.OffsetInDescriptorsFromTableStart = 0;			// Range ����������ͷ��ƫ���� (��λ��������)����ǩ����Ҫ������Ѱ�� Range �ĵ�ַ���������� 0 ����

	CD3DX12_ROOT_DESCRIPTOR_TABLE RootDescriptorTableDesc{};                // RootDescriptorTable ����������Ϣ�ṹ�壬һ�� Table �����ж�� Range
	RootDescriptorTableDesc.pDescriptorRanges = &SRVDescriptorRangeDesc;	// Range ��������Χָ��
	RootDescriptorTableDesc.NumDescriptorRanges = 1;						// ���������� Range ������


	CD3DX12_ROOT_PARAMETER RootParameter{};
	RootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;	// ��������������ɫ���ɼ�
	RootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // ���������ͣ�����ָ��Ϊ��������
	RootParameter.DescriptorTable = RootDescriptorTableDesc; // ������������������Ϣ

	CD3DX12_STATIC_SAMPLER_DESC StaticSamplerDesc{};	// ��̬�������������ṹ��
	StaticSamplerDesc.ShaderRegister = 0;				// ��̬������Ҫ�󶨵ļĴ����۱�� s0
	StaticSamplerDesc.RegisterSpace = 0;				// ��̬������Ҫ�󶨵ļĴ����ռ䣬Ĭ�϶��� 0
	StaticSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;	// ����������ͣ���������ֱ��ѡ �ڽ������ ����
	StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;			// �� U �����ϵ�����Ѱַ��ʽ
	StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;			// �� V �����ϵ�����Ѱַ��ʽ
	StaticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;			// �� W �����ϵ�����Ѱַ��ʽ (3D ������õ�)
	StaticSamplerDesc.MinLOD = 0;											// ��С LOD ϸ�ڲ�Σ���������Ĭ���� 0 ����
	StaticSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;							// ��� LOD ϸ�ڲ�Σ���������Ĭ���� D3D12_FLOAT32_MAX (û�� LOD ����)
	StaticSamplerDesc.MipLODBias = 0;										// ���� Mipmap ����ƫ������������������ֱ���� 0 ����
	StaticSamplerDesc.MaxAnisotropy = 1;									// �������Թ��˵ȼ������ǲ�ʹ�ø������Թ��ˣ���ҪĬ���� 1
	StaticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;			// �����������Ӱ��ͼ�ģ����ǲ���Ҫ������������ D3D12_COMPARISON_FUNC_NEVER


	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};	// ��ǩ���������ṹ��
	rootSignatureDesc.NumParameters = 1;				// ��������������������ֻ��һ��������
	rootSignatureDesc.pParameters = &RootParameter;	// ������ָ��
	rootSignatureDesc.NumStaticSamplers = 1;			// ��̬��������������������ֻ��һ����̬������
	rootSignatureDesc.pStaticSamplers = &StaticSamplerDesc; // ��̬������ָ��
	// ��ǩ����־������������Ⱦ���߲�ͬ�׶��µ��������״̬��ע���������Ҫ�� IA �׶����붥�����ݣ�����Ҫͨ����ǩ����������Ⱦ��������� IA �׶ζ�������
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	// �����ǩ�����ø�ǩ���ȱ���� GPU �ɶ��Ķ������ֽ���
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &SignatureBlob, &ErrorBlob);
	if (ErrorBlob)		// �����ǩ���������ErrorBlob �����ṩ������Ϣ
	{
		OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
		OutputDebugStringA("\n");
	}


	// ������������ֽ��봴����ǩ������
	m_D3D12Device->CreateRootSignature(0, SignatureBlob->GetBufferPointer(), SignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));


}

inline void DX12Engine::CreatePSO()
{
	// PSO �������ṹ��
	D3D12_GRAPHICS_PIPELINE_STATE_DESC  PSODesc{};

	// Input Assembler ����װ��׶�
	D3D12_INPUT_LAYOUT_DESC InputLayoutDesc{};	// ���벼���������ṹ��
	D3D12_INPUT_ELEMENT_DESC InputElementDesc[]{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};
	InputLayoutDesc.NumElements = _countof(InputElementDesc); // ����Ԫ������
	InputLayoutDesc.pInputElementDescs = InputElementDesc; // ����Ԫ��ָ��
	PSODesc.InputLayout = InputLayoutDesc; // �����벼����������ֵ�� PSO ������

	ComPtr<ID3DBlob> VertexShaderBlob;		// ������ɫ���������ֽ���
	ComPtr<ID3DBlob> PixelShaderBlob;		// ������ɫ���������ֽ���
	ComPtr<ID3DBlob> ErrorBlob;				// �����ֽ��룬��ǩ������ʧ��ʱ�� OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer()); ���Ի�ȡ������Ϣ

	// ���붥����ɫ�� Vertex Shader
	D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\DrawTexture\\shader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", NULL, NULL, &VertexShaderBlob, &ErrorBlob);
	if (ErrorBlob)		// �����ɫ���������ErrorBlob �����ṩ������Ϣ
	{
		OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
		OutputDebugStringA("\n");
	}

	// ����������ɫ�� Pixel Shader
	D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\DrawTexture\\shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", NULL, NULL, &PixelShaderBlob, &ErrorBlob);
	if (ErrorBlob)		// �����ɫ���������ErrorBlob �����ṩ������Ϣ
	{
		OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
		OutputDebugStringA("\n");
	}

	PSODesc.VS.pShaderBytecode = VertexShaderBlob->GetBufferPointer(); // ������ɫ���ֽ���ָ��	
	PSODesc.VS.BytecodeLength = VertexShaderBlob->GetBufferSize();	// ������ɫ���ֽ����С
	PSODesc.PS.pShaderBytecode = PixelShaderBlob->GetBufferPointer(); // ������ɫ���ֽ���ָ��
	PSODesc.PS.BytecodeLength = PixelShaderBlob->GetBufferSize();	// ������ɫ���ֽ����С
	// ��դ���׶�
	PSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;			// ָ�������޳�	
	PSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;			// ָ��ʵ�����
	
	// ��һ�����ø�ǩ�������������ǽ���ǩ���� PSO �󶨣�������Ⱦ���ߵ��������״̬
	PSODesc.pRootSignature = m_RootSignature.Get();

	PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // ָ��ͼԪ���ͣ���������ָ��������	
	PSODesc.NumRenderTargets = 1;												// ָ����ȾĿ������������ֻ��һ����ȾĿ��	
	PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;						// ָ����ȾĿ���ʽ�����ǵĽ�������ʽ�� DXGI_FORMAT_R8G8B8A8_UNORM
	PSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;				// ָ�������û��
	// ���ò������������������� 1 ����
	PSODesc.SampleDesc.Count = 1;
	// ���ò������룬��������ڶ��ز����ģ�����ֱ����ȫ���� (UINT_MAX�����ǽ� UINT ���еı���λȫ�����Ϊ 1) ����
	PSODesc.SampleMask = UINT_MAX;
	// ���� PSO ����
	m_D3D12Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(m_PipelineStateObject.GetAddressOf()));

}

inline void DX12Engine::CreateVertexResource()
{
	// CPU ���ٻ����ϵĶ�����Ϣ���飬ע������Ķ������궼�� NDC �ռ�����
	VERTEX vertexs[6] =
	{
		{{-0.75f, 0.75f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.75f, 0.75f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.75f, -0.75f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.75f, 0.75f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.75f, -0.75f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.75f, -0.75f, 0.0f, 1.0f}, {0.0f, 1.0f}}
	};

	CD3DX12_RESOURCE_DESC VertexDesc{};	// ���㻺������Դ�������ṹ��
	VertexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;		// ��Դ���ͣ��ϴ��ѵ���Դ���Ͷ��� buffer ����
	VertexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;			// ��Դ���֣�ָ����Դ�Ĵ洢��ʽ���ϴ��ѵ���Դ���� row major �������Դ洢
	VertexDesc.Width = sizeof(vertexs);							// ��Դ��ȣ��ϴ��ѵ���Դ�������Դ���ܴ�С
	VertexDesc.Height = 1;										// ��Դ�߶ȣ��ϴ��ѽ����Ǵ���������Դ�ģ����Ը߶ȱ���Ϊ 1
	VertexDesc.Format = DXGI_FORMAT_UNKNOWN;					// ��Դ��ʽ���ϴ�����Դ�ĸ�ʽ����Ϊ UNKNOWN
	VertexDesc.DepthOrArraySize = 1;							// ��Դ��ȣ������������������� 3D ����ģ��ϴ�����Դ����Ϊ 1
	VertexDesc.MipLevels = 1;									// Mipmap �ȼ����������������ģ��ϴ�����Դ����Ϊ 1
	VertexDesc.SampleDesc.Count = 1;							// ��Դ�����������ϴ�����Դ������ 1

	// �ϴ������ԵĽṹ�壬�ϴ���λ�� CPU �� GPU �Ĺ����ڴ�
	CD3DX12_HEAP_PROPERTIES UploadHeapDesc{ D3D12_HEAP_TYPE_UPLOAD };
	// �������㻺������Դ
	m_D3D12Device->CreateCommittedResource(
		&UploadHeapDesc,					// ������
		D3D12_HEAP_FLAG_NONE,
		&VertexDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_VertexResource.GetAddressOf())
	);

	BYTE* TransferPointer = nullptr;	// ���ڴ�����Դ��ָ��
	// Map ��ʼӳ�䣬Map ������õ��ϴ�����Դ�ĵ�ַ (�ڹ����ڴ���)�����ݸ�ָ�룬�������Ǿ���ͨ�� memcpy ��������������
	m_VertexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransferPointer));
	// ���ƶ������ݵ����㻺������Դ�� (CPU ���ٻ��� -> �����ڴ�)
	memcpy(TransferPointer, vertexs, sizeof(vertexs));
	// Unmap ����ӳ�䣬��Ϊ�����޷�ֱ�Ӷ�дĬ�϶���Դ����Ҫ�ϴ��Ѹ��Ƶ�����ڸ���֮ǰ��������Ҫ�Ƚ���ӳ�䣬���ϴ��Ѵ���ֻ��״̬
	m_VertexResource->Unmap(0, nullptr);

	// �������㻺������ͼ
	// ���㻺������Դ�� GPU �����ַ
	VertexBufferView.BufferLocation = m_VertexResource->GetGPUVirtualAddress();
	// ÿ������Ĵ�С (��λ���ֽ�)
	VertexBufferView.StrideInBytes = sizeof(VERTEX);
	// ���㻺�������ܴ�С (��λ���ֽ�)
	VertexBufferView.SizeInBytes = sizeof(vertexs);


}

inline void DX12Engine::Render()
{
	// �������������
	m_CommandAllocator->Reset();
	// ���������б����ó�ʼ״̬ PSO
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
	//��ȡRTV���׾��
	RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();
	FrameIndex = m_DXGISwapChain->GetCurrentBackBufferIndex(); // ��ȡ��ǰ֡����,��ʵ�Ǻ�̨�����е�һ������֡������
	// ���ݵ�ǰ֡����ƫ�� RTV �Ѿ��
	RTVHandle.ptr += FrameIndex * RTVDescriptorSize;

	// ��Դ���ϣ�����ǰ֡����ȾĿ��� PRESENT ״̬ת��Ϊ RENDER_TARGET ״̬
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_RenderTarget[FrameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,//��ǰ״̬
		D3D12_RESOURCE_STATE_RENDER_TARGET //Ҫת����״̬
	);

	m_CommandList->ResourceBarrier(1, &barrier); // ������Դ����
	// �ڶ������ø�ǩ�����������ý����� ��Ⱦ���߰󶨵ĸ�ǩ�� �� ����ĸ�ǩ�� �Ƿ�ƥ��
	// �Լ���ǩ��ָ������Դ�Ƿ���ȷ�󶨣������Ϻ����м򵥵�ӳ��
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	// ������Ⱦ����״̬������������ m_CommandList->Reset() ��ʱ��ֱ���ڵڶ����������� PSO
	m_CommandList->SetPipelineState(m_PipelineStateObject.Get());
	// �����ӿ� (��դ���׶�)�����ڹ�դ�������Ļӳ��
	m_CommandList->RSSetViewports(1, &viewPort);
	// ���òü����� (��դ���׶�)
	m_CommandList->RSSetScissorRects(1, &ScissorRect);
	// ������ȾĿ�� (����ϲ��׶�)
	m_CommandList->OMSetRenderTargets(1, &RTVHandle, false, nullptr);
	// ��յ�ǰ��ȾĿ��ı���Ϊ����ɫ
	m_CommandList->ClearRenderTargetView(RTVHandle, DirectX::Colors::SkyBlue, 0, nullptr);
	// �����������������õ���ʱ ID3D12DescriptorHeap ����
	ID3D12DescriptorHeap* _temp_DescriptorHeaps[] = { m_SRVHeap.Get() };
	// �����������ѣ���ɫ����������������
	m_CommandList->SetDescriptorHeaps(1, _temp_DescriptorHeaps);
	// ���ø����� 0 ���������������� 0 ��һ����������������������һ�� Range��Range ����һ�� SRV ������
	m_CommandList->SetGraphicsRootDescriptorTable(0, SRV_GPUHandle);
	// ����ͼԪ���� (����װ��׶�)���������������������б�
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// ���ö��㻺���� (����װ��׶�)
	m_CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);
	// ����ͼԪ
	m_CommandList->DrawInstanced(6, 1, 0, 0);
	// ��Դ���ϣ�����ǰ֡����ȾĿ��� RENDER_TARGET ״̬ת��Ϊ PRESENT ״̬
	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_RenderTarget[FrameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	m_CommandList->ResourceBarrier(1, &barrier); // ������Դ����

	// �ر������б�Record ¼��״̬ -> Close �ر�״̬�������б�ֻ�йرղſ����ύ
	m_CommandList->Close();

	// ���ڴ��������õ���ʱ ID3D12CommandList ����
	ID3D12CommandList* _temp_cmdlists[] = { m_CommandList.Get() };

	m_CommandQueue->ExecuteCommandLists(1, _temp_cmdlists); // �ύ���GPU ��ʼִ������

	// ��������з������������������������뵽��������У��������ִ�е�������ʱ����֪ͨ��������������
	m_DXGISwapChain->Present(1, NULL);
	// ��Χ��Ԥ��ֵ�趨Ϊ��һ֡
	FenceValue++;
	// ��������� (��������� GPU ��) ����Χ��Ԥ��ֵ�����������뵽���������
	// �������ִ�е�������޸�Χ��ֵ����ʾ��Ⱦ����ɣ�"����"Χ��
	m_CommandQueue->Signal(m_Fence.Get(), FenceValue);
	// ����Χ����Ԥ���¼�������Ⱦ���ʱ��Χ����"����"������Ԥ���¼������¼������ź�״̬ת�������ź�״̬
	m_Fence->SetEventOnCompletion(FenceValue, RenderEvent);
}

inline void DX12Engine::RenderLoop()
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
			Render();
			break;

		case 1:				// ActiveEvent �� 1��˵����Ⱦ�¼�δ��ɣ�CPU ���߳�ͬʱ��������Ϣ����ֹ�������
			// �鿴��Ϣ�����Ƿ�����Ϣ������оͻ�ȡ�� PM_REMOVE ��ʾ��ȡ����Ϣ�������̽�����Ϣ����Ϣ�������Ƴ�
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				// �������û���յ��˳���Ϣ���������ϵͳ�����ɷ���Ϣ������
				if (msg.message != WM_QUIT)
				{
					TranslateMessage(&msg);					// ������Ϣ�������ⰴ��ֵת��Ϊ��Ӧ�� ASCII �� (���Ļὲ)
					DispatchMessage(&msg);					// �ɷ���Ϣ��֪ͨ����ϵͳ���ûص�����������Ϣ
				}
				else
				{
					isExit = true;							// �յ��˳���Ϣ�����˳���Ϣѭ��
				}
			}
			break;

		case WAIT_TIMEOUT:	// ��Ⱦ��ʱ
			break;
		}
	}
}

// �ص�����
inline LRESULT CALLBACK DX12Engine::CallBackFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// �� switch ���ڶ�������������ÿ�� case �ֱ��Ӧһ��������Ϣ
	switch (msg)
	{
	case WM_DESTROY:			// ���ڱ����� (���������Ͻ� X �رմ���ʱ)
		PostQuitMessage(0);		// �����ϵͳ�����˳����� (WM_QUIT)��������Ϣѭ��
		break;

		// ������յ�������Ϣ��ֱ��Ĭ�Ϸ�����������
	default: return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;	// ע�����
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

// ������
int WINAPI WinMain(HINSTANCE hins, HINSTANCE hPrev, LPSTR cmdLine, int cmdShow)
{
	DX12Engine::Run(hins);
	return 0;
}