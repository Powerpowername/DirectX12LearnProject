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
//	//交换链
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
//	swapChainDesc.BufferDesc.Width = 800; // 设置交换链缓冲区的宽度
//	swapChainDesc.BufferDesc.Height = 600; // 设置交换链缓冲区的高度
//	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 设置交换链缓冲区的格式
//	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60; // 设置刷新率
//	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1; // 设置刷新率分母
//	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 设置缓冲区的使用方式
//	swapChainDesc.BufferCount = 2; // 设置交换链缓冲区的数量
//	swapChainDesc.OutputWindow = nullptr; // 设置输出窗口句柄
//	swapChainDesc.Windowed = TRUE; // 设置窗口模式
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 设置交换效果
//	swapChainDesc.Flags = 0; // 设置标志位
//	// 创建交换链
//	ComPtr<IDXGIFactory4> factory;
//	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
//}
//
//class D3DApp
//{
//public:
//	void CreateSwapChain();
//};