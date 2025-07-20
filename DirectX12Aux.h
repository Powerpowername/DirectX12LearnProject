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
	
	HWND      mhMainWnd = nullptr;// �����ھ��
	bool	m4xMsaaState = false;// �Ƿ���4�����ز��������
	UINT	m4xMsaaQuality = 0;// 4�����ز�������ݵ���������

	UINT mRtvDescriptorSize = 0;// RTV��������С
	UINT mDsvDescriptorSize = 0;// DSV��������С
	UINT mCbvSrvUavDescriptorSize = 0;// CBV/SRV/UAV��������С
	
	static const int SwapChainBufferCount = 2;// ����������������
	int mCurrBackBuffer = 0;// ��ǰ�󱸻���������
	Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];// �󱸻���������
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;// ���ģ�建����ָ��

	ComPtr<IDXGISwapChain> mSwapChain;//������
	ComPtr<IDXGIFactory4> mdxgiFactory;// DXGI����ָ��
	ComPtr<ID3D12Device> md3dDevice;// D3D12�豸ָ��

	// D3D12դ��
	ComPtr<ID3D12Fence> mFence;// D3D12դ��ָ��
	UINT64 mCurrentFence = 0;// դ����ʼֵ

	// Direct3D��Դ����
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;// �������ָ��
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;// ���������ָ��
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;// �����б�ָ��

	ComPtr<ID3D12DescriptorHeap> mRtvHeap;// RTV��������
	ComPtr<ID3D12DescriptorHeap> mDsvHeap;// DSV��������

	// ͨ�ó�ʼֵ
	int mClientWidth = 800;
	int mClientHeight = 600;
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//function
	bool InitDirect3D();// ��ʼ��Direct3D
	void CreateSwapChain();// ����������
	void CreateCommandObjects();// �����������
	void CreateRtvAndDsvDescriptorHeaps();// ����RTV��DSV��������
	void OnResize();// ���ڴ�С����������
	void FlushCommandQueue();// ˢ���������
	void LogAdapters();// ��־��������Ϣ
	void LogAdapterOutputs(IDXGIAdapter* adapter);// ��־�����������Ϣ
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);// ��־�����ʾģʽ��Ϣ

};