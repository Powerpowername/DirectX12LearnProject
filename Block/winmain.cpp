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
// DX12 ����
class DX12Engine
{
private:
	////test
	//D3D12_RESOURCE_BARRIER beg_barrier = {};				// ��Ⱦ��ʼ����Դ���ϣ����� -> ��ȾĿ��
	//D3D12_RESOURCE_BARRIER end_barrier = {};				// ��Ⱦ��������Դ���ϣ���ȾĿ�� -> ����

	int WindowWidth = 800;		// ���ڿ��
	int WindowHeight = 600;	// ���ڸ߶�
	HWND m_hwnd = nullptr;	// ���ھ��
	ComPtr<ID3D12Debug> m_D3D12DebugDevice; // D3D12 ���Խӿ�
	UINT m_DXGICreateFactoryFlag = NULL; // ���� DXGI ����ʱ��Ҫ�õ��ı�־
	ComPtr<IDXGIFactory6> m_DXGIFactory; // DXGI ����
	ComPtr<IDXGIAdapter1> m_DXGIAdapter; // �Կ�������
	ComPtr<ID3D12Device> m_D3D12Device; // D3D12 �豸

	ComPtr<ID3D12CommandQueue> m_CommandQueue; // �������
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator; // ���������
	ComPtr<ID3D12GraphicsCommandList> m_CommandList; // �����б�

	ComPtr<IDXGISwapChain4> m_DXGISwapChain; // DXGI ������
	ComPtr<ID3D12DescriptorHeap> m_RTVHeap; // RTV ��������
	ComPtr<ID3D12Resource> m_RenderTarget[3]; // ������ȾĿ��
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle = {}; // RTV ���������

	UINT RTVDescriptorSize = 0; // RTV �������Ĵ�С
	UINT FrameIndex = 0; // ֡��������ʾ��ǰ��Ⱦ�ĵ� i ֡ (�� i ����ȾĿ��)

	ComPtr<ID3D12Fence> m_Fence; // GPU ͬ���õ�դ��
	UINT64 FenceValue = 0; // դ��ֵ
	HANDLE RenderEvent = nullptr; // GPU ��Ⱦ�¼�
	CD3DX12_RESOURCE_BARRIER barrier{};//  ��Ⱦ��ʼ����Դ���ϣ����� -> ��ȾĿ��

	std::wstring TextureFilename = L"E:\\DirectX12Project\\DirectX12Pro\\Block/diamond_ore.png"; // �����ļ�·��
	ComPtr<IWICImagingFactory> m_WICFactory;				// WIC ����
	ComPtr<IWICBitmapDecoder> m_WICBitmapDecoder;			// λͼ������
	ComPtr<IWICBitmapFrameDecode> m_WICBitmapDecodeFrame;	// �ɽ������õ��ĵ���λͼ֡
	ComPtr<IWICFormatConverter> m_WICFormatConverter;		// λͼת����
	ComPtr<IWICBitmapSource> m_WICBitmapSource;				// WIC λͼ��Դ�����ڻ�ȡλͼ����

	UINT TextureWidth = 0;		// ������
	UINT TextureHeight = 0;	// ����߶�
	UINT BitsPerPixel = 0;	// ����ÿ����λ��
	UINT BytePerRowSize = 0;	// ����ÿ�����ֽ���
	DXGI_FORMAT TextureFormat = DXGI_FORMAT_UNKNOWN; // �����ʽ
	unique_ptr<BYTE[]> TextureData = nullptr; // ��������

	ComPtr<ID3D12DescriptorHeap> m_SRVHeap; // SRV ��������
	D3D12_CPU_DESCRIPTOR_HANDLE SRV_CPUHandle{}; // SRV CPU ���������,����������������
	D3D12_GPU_DESCRIPTOR_HANDLE SRV_GPUHandle{}; // SRV GPU ����������������󶨵�����

	ComPtr<ID3D12Resource> m_UploadTextureResource;			// �ϴ�����Դ��λ�ڹ����ڴ棬������ת������Դ
	ComPtr<ID3D12Resource> m_DefaultTextureResource;		// Ĭ�϶���Դ��λ���Դ棬���ڷ�����

	UINT TextureSize = 0; // �������ݴ�С
	UINT UploadResourceRowSize = 0;							// �ϴ�����Դÿ�еĴ�С (��λ���ֽ�)
	UINT UploadResourceSize = 0;							// �ϴ�����Դ���ܴ�С (��λ���ֽ�)

	ComPtr<ID3D12Resource> m_CBVResource;				// ������������Դ
	struct CBuffer								// ��������ṹ��
	{
		XMFLOAT4X4 MVPMatrix;		// MVP �������ڽ��������ݴӶ���ռ�任����βü��ռ�
	};

	CBuffer* MVPBuffer = nullptr;	// ��������ṹ��ָ�룬����洢���� MVP ������Ϣ������ Map ��ָ���ָ�� CBVResource �ĵ�ַ
	
	XMVECTOR EyePosition = XMVectorSet(4, 3, 4, 1);					// �����������ռ��µ�λ��
	XMVECTOR FocusPosition = XMVectorSet(0, 1, 1, 1);				// �����������ռ��¹۲�Ľ���λ��
	XMVECTOR UpDirection = XMVectorSet(0, 1, 0, 0);					// ����ռ䴹ֱ���ϵ�����
	XMMATRIX ModelMatrix;											// ģ�;���ģ�Ϳռ� -> ����ռ�
	XMMATRIX ViewMatrix;											// �۲��������ռ� -> �۲�ռ�
	XMMATRIX ProjectionMatrix;										// ͶӰ���󣬹۲�ռ� -> ��βü��ռ�

	ComPtr<ID3D12RootSignature> m_RootSignature; // ��ǩ��
	ComPtr<ID3D12PipelineState> m_PipelineStateObject; // ����״̬����

	ComPtr<ID3D12Resource> m_VertexResource; // ���㻺������Դ
	struct VERTEX											// �������ݽṹ��
	{
		XMFLOAT4 position;									// ����λ��
		XMFLOAT2 texcoordUV;								// ������������
	};

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView; // ���㻺������ͼ
	ComPtr<ID3D12Resource> m_IndexResource; // ������������Դ
	D3D12_INDEX_BUFFER_VIEW IndexBufferView{}; // ������������ͼ

	// �ӿ�
	D3D12_VIEWPORT viewPort = D3D12_VIEWPORT{ 0, 0, float(WindowWidth), float(WindowHeight), D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
	// �ü�����
	D3D12_RECT ScissorRect = D3D12_RECT{ 0, 0, WindowWidth, WindowHeight };


public:
	
	void InitWindow(HINSTANCE hins)
	{
		
		WNDCLASS wc = {};					// ���ڼ�¼��������Ϣ�Ľṹ��
		wc.hInstance = hins;				// ��������Ҫһ��Ӧ�ó����ʵ����� hinstance
		wc.lpfnWndProc = CallBackFunc;		// ��������Ҫһ���ص����������ڴ����ڲ�������Ϣ
		wc.lpszClassName = L"DX12 Game";	// �����������

		RegisterClass(&wc);					// ע�ᴰ���࣬��������¼�뵽����ϵͳ��

		// ʹ�����ĵĴ����ഴ������
		m_hwnd = CreateWindow(wc.lpszClassName, L"DX12����ʯԭ��", WS_SYSMENU | WS_OVERLAPPED,
			10, 10, WindowWidth, WindowHeight,
			NULL, NULL, hins, NULL);

		// ��Ϊָ���˴��ڴ�С���ɱ�� WS_SYSMENU �� WS_OVERLAPPED��Ӧ�ò����Զ���ʾ���ڣ���Ҫʹ�� ShowWindow ǿ����ʾ����
		ShowWindow(m_hwnd, SW_SHOW);
	}

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
			MessageBox(NULL, L"�Ҳ����κ���֧�� DX12 ���Կ��������������ϵ�Ӳ����", L"����", MB_OK | MB_ICONERROR);
			return false;
		}


	}

	void CreateCommandComponents()
	{
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // ֱ�������б�����ִ������ GPU ����
		m_D3D12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue));	
		m_D3D12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));	
		m_D3D12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList));
		m_CommandList->Close(); // ���������������б�Ĭ���Ǵ򿪵ģ�������Ҫ�ȹرգ��ȵ���Ҫ�õ�ʱ���ٴ�


	}
	void CreateRenderTarget()
	{
		D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc{};
		RTVHeapDesc.NumDescriptors = 3; // ������ȾĿ��
		RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // RTV ��������
		m_D3D12Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_RTVHeap));	

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = 3; // ����������
		swapChainDesc.Width = WindowWidth; // ���
		swapChainDesc.Height = WindowHeight; // �߶�
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32λ��ɫ��ʽ
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // �������Ļ�����������ȾĿ��
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ��ת����ģʽ
		swapChainDesc.SampleDesc.Count = 1; // ���ز�������Ϊ 1����ʾ��ʹ�ö��ز���
		ComPtr<IDXGISwapChain1> _temp_swapchain;

		m_DXGIFactory->CreateSwapChainForHwnd(
			m_CommandQueue.Get(),	// ��������Ҫ�󶨵�һ�����������
			m_hwnd,					// ��������Ҫ�󶨵�һ��������
			&swapChainDesc,			// ������������
			nullptr, nullptr, &_temp_swapchain
		);
		_temp_swapchain.As(&m_DXGISwapChain); // ��ѯ IDXGISwapChain4 �ӿ�
		RTVHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart(); // ��ȡ RTV �������ѵ���ʼ���
		RTVDescriptorSize = m_D3D12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // ��ȡ RTV �������Ĵ�С
		for (int i = 0;i < 3;i++)
		{
			m_DXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i]));
			m_D3D12Device->CreateRenderTargetView(m_RenderTarget[i].Get(), nullptr, RTVHandle);
			//RTVHandle��Ҫ����һ��ָ�������
			RTVHandle.ptr += RTVDescriptorSize; // ָ����һ��������

		}
	}

	void CreateFenceAndBarrier()
	{
		RenderEvent = CreateEvent(nullptr, FALSE, TRUE, nullptr);
		m_D3D12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
		//��Դ���Ϻ����õ��ٴ���
				// ������Դ����
		//// beg_barrier ��ʼ���ϣ�Present ����״̬ -> Render Target ��ȾĿ��״̬
		//beg_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;					// ָ������Ϊת������		
		//beg_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		//beg_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		//// end_barrier ��ֹ���ϣ�Render Target ��ȾĿ��״̬ -> Present ����״̬
		//end_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		//end_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		//end_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	}

	bool LoadTextureFromFile()
	{
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
		SRVHeapDesc.NumDescriptors = 1; // һ������
		SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // SRV ��������
		SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // �������������Ҫ����ɫ���пɼ�
		m_D3D12Device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(&m_SRVHeap));

	}

	inline UINT Ceil(UINT A, UINT B)
	{
		return (A + B - 1) / B;
	}

	void CreateUploadAndDefaultResource()
	{
		BytePerRowSize = TextureWidth * BitsPerPixel / 8; // �����ÿ�е��ֽ���
		TextureSize = BytePerRowSize * TextureHeight; // ������ܵ��������ݴ�С
		// ������ϴ�����Դÿ�еĴ�С�������� 256 �ֽڶ����
		UploadResourceRowSize = Ceil(BytePerRowSize, 256) * 256;

		UploadResourceSize = UploadResourceRowSize * (TextureHeight - 1) + BytePerRowSize;

		CD3DX12_RESOURCE_DESC UploadResourceDesc{};
		UploadResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // ��Դά���� Buffer
		UploadResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // ������
		UploadResourceDesc.Width = UploadResourceSize; // ��Դ��С
		UploadResourceDesc.Height = 1; // �߶�Ϊ 1
		UploadResourceDesc.DepthOrArraySize = 1; // ��Ȼ������СΪ 1
		UploadResourceDesc.MipLevels = 1; // Mip ����Ϊ 1
		UploadResourceDesc.SampleDesc.Count = 1; // ���ز���Ϊ 1

		CD3DX12_HEAP_PROPERTIES UploadHeapDesc{ D3D12_HEAP_TYPE_UPLOAD }; // �ϴ�������
		m_D3D12Device->CreateCommittedResource(&UploadHeapDesc, D3D12_HEAP_FLAG_NONE, &UploadResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_UploadTextureResource));

		CD3DX12_RESOURCE_DESC DefaultResourceDesc{};
		DefaultResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // ��Դά���� 2D ����
		DefaultResourceDesc.Alignment = 0; // Ĭ�϶���
		DefaultResourceDesc.Width = TextureWidth; // ������
		DefaultResourceDesc.Height = TextureHeight; // ����߶�
		DefaultResourceDesc.DepthOrArraySize = 1; // ��Ȼ������СΪ 1
		DefaultResourceDesc.MipLevels = 1; // Mip ����Ϊ 1
		DefaultResourceDesc.Format = TextureFormat; // �����ʽ
		DefaultResourceDesc.SampleDesc.Count = 1; // ���ز���Ϊ 1
		DefaultResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // ������δ֪

		D3D12_HEAP_PROPERTIES DefaultHeapDesc{ D3D12_HEAP_TYPE_DEFAULT };
		
		m_D3D12Device->CreateCommittedResource(&DefaultHeapDesc, D3D12_HEAP_FLAG_NONE, &DefaultResourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_DefaultTextureResource));

	}
	void CopyTextureDataToDefaultResource()
	{
		// ������ʱ�洢�������ݵ�ָ�룬����Ҫ�� malloc ����ռ�
		//TextureData = (BYTE*)malloc(TextureSize);
		//unique_ptr<BYTE> TextureData = nullptr; // ��������
		TextureData = std::make_unique<BYTE[]>(TextureSize);
		// �������������ݶ��� TextureData �У�������ĵ� memcpy ���Ʋ���
		m_WICBitmapSource->CopyPixels(nullptr, BytePerRowSize, TextureSize, TextureData.get());
		BYTE* TextureDataPtr = TextureData.get(); // �����ʼָ�룬�����ͷ��ڴ�ʱҪ��
		// ���ڴ�����Դ��ָ��
		BYTE* TransferPointer = nullptr;

		D3D12_SUBRESOURCE_DATA textureData{};
		textureData.pData = TextureData.get();					// ָ���������ݵ�ָ��
		textureData.RowPitch = BytePerRowSize;			//Ϊ�����ÿ�еĴ�С
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
		SRVDescriptorDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // SRV ��ͼά���� 2D ����
		SRVDescriptorDesc.Format = TextureFormat; // �����ʽ
		SRVDescriptorDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // Ĭ��ӳ��
		SRVDescriptorDesc.Texture2D.MipLevels = 1; // Mip ����Ϊ 1
		SRV_CPUHandle = m_SRVHeap->GetCPUDescriptorHandleForHeapStart(); // ��ȡ SRV �������ѵ���ʼ���
		m_D3D12Device->CreateShaderResourceView(m_DefaultTextureResource.Get(), &SRVDescriptorDesc, SRV_CPUHandle);
		//���ø�����ʱ��Ҫ�õ�SRV��GPU������󶨸�����
		SRV_GPUHandle = m_SRVHeap->GetGPUDescriptorHandleForHeapStart(); // ��ȡ SRV �������ѵ���ʼ GPU ���

	}

	// ���� Constant Buffer Resource ����������Դ������������һ��Ԥ�ȷ���ĸ����Դ棬���ڴ洢ÿһ֡��Ҫ�任����Դ����������Ҫ�洢 MVP ����
	void CreateCBVBufferResource()
	{

		// ������Դ��ȣ������������ṹ��Ĵ�С��ע�⣡Ӳ��Ҫ�󣬳���������Ҫ 256 �ֽڶ��룡��������Ҫ���� Ceil ����ȡ���������ڴ���룡
		UINT CBufferWidth = Ceil(sizeof(CBuffer), 256) * 256;//�˴��ʹ�MVP����
		//CBVͨ���Ƿ����ϴ����У���Ϊ����ҪƵ�����£����Բ����ϴ���Ĭ�϶���
		CD3DX12_RESOURCE_DESC CBVResourceDesc{};
		CBVResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // �ϴ�����Դ���ǻ���
		CBVResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; 	// �ϴ�����Դ���ǰ��д洢���ݵ� (һά���Դ洢)
		CBVResourceDesc.Width = CBufferWidth;				// ��Դ���
		CBVResourceDesc.Height = 1;							// ��Դ�߶�Ϊ 1
		CBVResourceDesc.Format = DXGI_FORMAT_UNKNOWN;		// �ϴ�����Դ�ĸ�ʽ����Ϊ DXGI_FORMAT_UNKNOWN
		CBVResourceDesc.DepthOrArraySize = 1;				// ��Դ��Ȼ������СΪ 1
		CBVResourceDesc.MipLevels = 1;						// Mip ����Ϊ 1
		CBVResourceDesc.SampleDesc.Count = 1;				// ���ز���Ϊ 1

		CD3DX12_HEAP_PROPERTIES CBVHeapDesc{ D3D12_HEAP_TYPE_UPLOAD }; // �ϴ�������
		m_D3D12Device->CreateCommittedResource(&CBVHeapDesc, D3D12_HEAP_FLAG_NONE, &CBVResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_CBVResource));	
		// ������������Դӳ�䵽 CPU �ɷ��ʵ��ڴ��ַ
		m_CBVResource->Map(0, nullptr, reinterpret_cast<void**>(&MVPBuffer));
	}


	// ������ǩ��
	void CreateRootSignature()
	{
		ComPtr<ID3DBlob> SignaatureBlob = nullptr; // ��ǩ���Ķ����ƴ���
		ComPtr<ID3DBlob> ErrorBlob = nullptr; // ���ڴ洢������Ϣ�� Blob
		CD3DX12_ROOT_PARAMETER RootParameters[2]{};

		CD3DX12_DESCRIPTOR_RANGE SRVDescriptorRangeDesc{}; // SRV ��������
		SRVDescriptorRangeDesc.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // ������������ SRV
		SRVDescriptorRangeDesc.NumDescriptors = 1; // һ������
		SRVDescriptorRangeDesc.BaseShaderRegister = 0; // �� t0 �Ĵ�����ʼ
		SRVDescriptorRangeDesc.RegisterSpace = 0; // �Ĵ����ռ� 0
		SRVDescriptorRangeDesc.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // �Զ�����ƫ����
		RootParameters[0].InitAsDescriptorTable(1, &SRVDescriptorRangeDesc, D3D12_SHADER_VISIBILITY_PIXEL); // ������ɫ���ɼ�
		//RootParameters[1].InitAsConstantBufferView(0); // b0 �Ĵ���
		//CD3DX12_ROOT_DESCRIPTOR CBVRootDescriptorDesc{};
		//CBVRootDescriptorDesc.RegisterSpace = 0; // �Ĵ����ռ� 0
		//CBVRootDescriptorDesc.ShaderRegister = 0; // b0 �Ĵ���
		//�˴�ֱ��ʹ�õ��Ǹ�������������Դ��Ϣֱ���������˸��������ˣ������Ͳ���Ҫ��������������&����������
		//��Ҫ˵�����ǣ��˴�ֻ��˵��Ҫ��������������������������������ǻ�û����ô�������Ժ�����ȻҪʹ��
		//ID3D12GraphicsCommandList::SetDescriptorHeaps��Ԫ���ݴ��뵽�������γɸ�������
		RootParameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // ������ɫ���ɼ�

		CD3DX12_STATIC_SAMPLER_DESC StaticSamplerDesc{}; // ��̬������
		StaticSamplerDesc.ShaderRegister = 0; // s0 �Ĵ���
		StaticSamplerDesc.RegisterSpace = 0; // �Ĵ����ռ� 
		StaticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // ������ɫ���ɼ�	
		StaticSamplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		StaticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		StaticSamplerDesc.MipLODBias = 0; // Mip ����ƫ��Ϊ 0
		StaticSamplerDesc.MaxAnisotropy = 0; // �������Թ��˵ȼ�Ϊ 0
		StaticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // �ȽϺ���
		StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK; // �߽���ɫΪ��͸����ɫ

		CD3DX12_ROOT_SIGNATURE_DESC rootsignatureDesc{};
		rootsignatureDesc.Init(_countof(RootParameters), RootParameters, 1, &StaticSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		D3D12SerializeRootSignature(&rootsignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &SignaatureBlob, &ErrorBlob);
		if (ErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
		}
		m_D3D12Device->CreateRootSignature(0, SignaatureBlob->GetBufferPointer(), SignaatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
	}

	// ������Ⱦ����״̬���� (Pipeline State Object, PSO)
	void CreatePSO()
	{
		//���벼����ר���������� GPU ���ǵĶ������ݳ�ʲô���ӵ�
		D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc{};
		D3D12_INPUT_LAYOUT_DESC InputLayoutDesc{}; // ���벼������
		D3D12_INPUT_ELEMENT_DESC InputElementDesc[2]{
			"POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0,
			"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		};

		InputLayoutDesc.NumElements = _countof(InputElementDesc); // ����Ԫ������
		InputLayoutDesc.pInputElementDescs = InputElementDesc; // ����Ԫ����������
		PSODesc.InputLayout = InputLayoutDesc; // ���벼��



		ComPtr<ID3DBlob> VertexShaderBlob = nullptr; // ������ɫ���Ķ����ƴ���
		ComPtr<ID3DBlob> PixelShaderBlob = nullptr; // ������ɫ���Ķ����ƴ���
		ComPtr<ID3DBlob> ErrorBlob = nullptr; // ���ڴ洢������Ϣ�� Blob
		D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\Block/shader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", NULL, NULL, &VertexShaderBlob, &ErrorBlob);
		if (ErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)ErrorBlob->GetBufferPointer());
			OutputDebugStringA("\n");
		}
		// ����������ɫ�� Pixel Shader
		D3DCompileFromFile(L"E:\\DirectX12Project\\DirectX12Pro\\Block/shader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", NULL, NULL, &PixelShaderBlob, &ErrorBlob);
		if (ErrorBlob)		// �����ɫ���������ErrorBlob �����ṩ������Ϣ
		{
			OutputDebugStringA((const char*)ErrorBlob->GetBufferPointer());
			OutputDebugStringA("\n");
		}

		PSODesc.VS.pShaderBytecode = VertexShaderBlob->GetBufferPointer(); // ������ɫ������
		PSODesc.VS.BytecodeLength = VertexShaderBlob->GetBufferSize(); // ������ɫ�������С
		PSODesc.PS.pShaderBytecode = PixelShaderBlob->GetBufferPointer(); // ������ɫ������
		PSODesc.PS.BytecodeLength = PixelShaderBlob->GetBufferSize(); // ������ɫ�������С

		// Rasterizer ��դ��
		PSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		PSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

		// ��һ�����ø�ǩ�������������ǽ���ǩ���� PSO �󶨣�������Ⱦ���ߵ��������״̬
		PSODesc.pRootSignature = m_RootSignature.Get();

		PSODesc.pRootSignature = m_RootSignature.Get(); // ��ǩ��
		PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // ͼԪ����������������
		PSODesc.NumRenderTargets = 1; // һ����ȾĿ��
		PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // ��ȾĿ���ʽ
		PSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		PSODesc.SampleDesc.Count = 1; // ���ز���Ϊ 1
		PSODesc.SampleMask = UINT_MAX; // ��������
		m_D3D12Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&m_PipelineStateObject));

	
	}


	void CreateVertexResource()
	{
#pragma region ���嶥��ṹ��
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

		D3D12_RESOURCE_DESC VertexDesc{};
		VertexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // ��Դά���� Buffer
		VertexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // ������
		VertexDesc.Width = sizeof(vertexs); // ��Դ��С
		VertexDesc.Height = 1; // �߶�Ϊ 1
		VertexDesc.Format = DXGI_FORMAT_UNKNOWN; // ��ʽδ֪
		VertexDesc.DepthOrArraySize = 1; // ��Ȼ������СΪ 1
		VertexDesc.MipLevels = 1; // Mip ����Ϊ 1
		VertexDesc.SampleDesc.Count = 1; // ���ز�������Ϊ 1����ʾ��ʹ�ö��ز���

		CD3DX12_HEAP_PROPERTIES UploadHeapDesc{ D3D12_HEAP_TYPE_UPLOAD }; // �ϴ�������

		// ������Դ��CreateCommittedResource ��Ϊ��Դ�Զ�����һ���ȴ�С����ʽ�ѣ������ʽ�ѵ�����Ȩ�ɲ���ϵͳ���������߲��ɿ���
		m_D3D12Device->CreateCommittedResource(&UploadHeapDesc, D3D12_HEAP_FLAG_NONE, &VertexDesc, D3D12_RESOURCE_STATE_GENERIC_READ
			, nullptr, IID_PPV_ARGS(&m_VertexResource));
		BYTE* TransferPointer = nullptr;
		m_VertexResource->Map(0, nullptr, reinterpret_cast<void**>(&TransferPointer));

		memcpy(TransferPointer, vertexs, sizeof(vertexs)); // ���ƶ������ݵ���Դ��
		m_VertexResource->Unmap(0, nullptr); // ���ӳ��
		// ��ʼ�����㻺����ͼ
		VertexBufferView.BufferLocation = m_VertexResource->GetGPUVirtualAddress(); // ���㻺��� GPU �����ַ
		VertexBufferView.StrideInBytes = sizeof(VERTEX); // ÿ������Ĵ�С
		VertexBufferView.SizeInBytes = sizeof(vertexs); // ���㻺���������С
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
		// �����ӿ� (��դ���׶�)�����ڹ�դ�������Ļӳ��
		m_CommandList->RSSetViewports(1, &viewPort);
		// ���òü����� (��դ���׶�)
		m_CommandList->RSSetScissorRects(1, &ScissorRect);

		m_CommandList->OMSetRenderTargets(1, &RTVHandle, false, nullptr);
		// ��յ�ǰ��ȾĿ��ı���Ϊ����ɫ
		m_CommandList->ClearRenderTargetView(RTVHandle, DirectX::Colors::SkyBlue, 0, nullptr);

		ID3D12DescriptorHeap* _temp_DescriptorHeaps[]{ m_SRVHeap.Get() };
		m_CommandList->SetDescriptorHeaps(1, _temp_DescriptorHeaps);
		m_CommandList->SetGraphicsRootDescriptorTable(0, SRV_GPUHandle);

		m_CommandList->SetGraphicsRootConstantBufferView(1, m_CBVResource->GetGPUVirtualAddress());

		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// ���� VBV ���㻺�������� (����װ��׶�) 
		m_CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);

		// ���� IBV �������������� (����װ��׶�) 
		m_CommandList->IASetIndexBuffer(&IndexBufferView);

		// Draw Call! ���Ʒ���
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
		m_DXGISwapChain->Present(1, 0);//�����ȴ�1�δ�ֱͬ��

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
	static LRESULT CALLBACK CallBackFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
	// ���д���
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


// ������
int WINAPI WinMain(HINSTANCE hins, HINSTANCE hPrev, LPSTR cmdLine, int cmdShow)
{
	DX12Engine::Run(hins);
}