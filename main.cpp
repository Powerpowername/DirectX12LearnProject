#include<Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
using namespace Microsoft::WRL;
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}




int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif 
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"YSHRenderClass";

	RegisterClassEx(&windowClass);

	HWND hwnd = CreateWindow(
		windowClass.lpszClassName,
		L"YSHRender",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		800,
		600,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	ShowWindow(hwnd, SW_SHOW);


	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}


	return 0;
}


//void text()
//{
//	//������
//	/*
//	typedef struct DXGI_SWAP_CHAIN_DESC
//    {
//    DXGI_MODE_DESC BufferDesc;
//    DXGI_SAMPLE_DESC SampleDesc;
//    DXGI_USAGE BufferUsage;
//    UINT BufferCount;
//    HWND OutputWindow;
//    BOOL Windowed;
//    DXGI_SWAP_EFFECT SwapEffect;
//    UINT Flags;
//    } 	DXGI_SWAP_CHAIN_DESC;
//	*/
//	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
//	swapChainDesc.BufferDesc.Width = 800; // ���ý������������Ŀ��
//	swapChainDesc.BufferDesc.Height = 600; // ���ý������������ĸ߶�
//	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ���ý������������ĸ�ʽ
//	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60; // ����ˢ����
//	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1; // ����ˢ���ʷ�ĸ
//	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ���û�������ʹ�÷�ʽ
//	swapChainDesc.BufferCount = 2; // ���ý�����������������
//	swapChainDesc.OutputWindow = nullptr; // ����������ھ��
//	swapChainDesc.Windowed = TRUE; // ���ô���ģʽ
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ���ý���Ч��
//	swapChainDesc.Flags = 0; // ���ñ�־λ
//	// ����������
//	ComPtr<IDXGIFactory4> factory;
//	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
//}
//
//class D3DApp
//{
//public:
//	void CreateSwapChain();
//};