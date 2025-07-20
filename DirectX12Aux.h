#pragma once
#include <wrl.h>
#include <string>
#include <stdexcept> 
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include "d3dUtil.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;
class D3DApp
{
public:
	
	HWND      mhMainWnd = nullptr;// 主窗口句柄
	bool	m4xMsaaState = false;// 是否开启4倍多重采样抗锯齿
	UINT	m4xMsaaQuality = 0;// 4倍多重采样抗锯齿的质量级别

	UINT mRtvDescriptorSize = 0;// RTV描述符大小
	UINT mDsvDescriptorSize = 0;// DSV描述符大小
	UINT mCbvSrvUavDescriptorSize = 0;// CBV/SRV/UAV描述符大小
	
	static const int SwapChainBufferCount = 2;// 交换链缓冲区数量
	int mCurrBackBuffer = 0;// 当前后备缓冲区索引
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];// 后备缓冲区数组
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;// 深度模板缓冲区指针

	ComPtr<IDXGISwapChain> mSwapChain;//交换链
	ComPtr<IDXGIFactory4> mdxgiFactory;// DXGI工厂指针
	ComPtr<ID3D12Device> md3dDevice;// D3D12设备指针

	// D3D12栅栏
	ComPtr<ID3D12Fence> mFence;// D3D12栅栏指针
	UINT64 mCurrentFence = 0;// 栅栏初始值

	// Direct3D资源管理
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;// 命令队列指针
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;// 命令分配器指针
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;// 命令列表指针

	ComPtr<ID3D12DescriptorHeap> mRtvHeap;// RTV描述符堆
	ComPtr<ID3D12DescriptorHeap> mDsvHeap;// DSV描述符堆

	// 通用初始值
	int mClientWidth = 800;
	int mClientHeight = 600;
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//function
	bool InitDirect3D();// 初始化Direct3D
	void CreateSwapChain();// 创建交换链
	void CreateCommandObjects();// 创建命令对象
	void CreateRtvAndDsvDescriptorHeaps();// 创建RTV和DSV描述符堆
	void OnResize();// 窗口大小调整处理函数
	void FlushCommandQueue();// 刷新命令队列
	void LogAdapters();// 日志适配器信息
	void LogAdapterOutputs(IDXGIAdapter* adapter);// 日志适配器输出信息
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);// 日志输出显示模式信息

};