#include "DirectX12Aux.h"
#include <assert.h>
//std::assert;
//std::string HrToString(HRESULT hr)
//{
//	char s_str[64] = {};
//	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
//	return std::string(s_str);
//}
//
//void ThrowIfFailed(HRESULT hr)
//{
//	if (FAILED(hr))
//	{
//		throw HrException(hr);
//	}
//}

bool D3DApp::InitDirect3D()
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())));
		debugController->EnableDebugLayer();
	}
#endif
	// Create the DXGI factory.
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(mdxgiFactory.GetAddressOf())));
	//����Ӳ��������
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(md3dDevice.GetAddressOf()));
	if (FAILED(hardwareResult))
	{
		// �������Ӳ��������ʧ�ܣ����Դ���WARP������
		// WARP��Windows Advanced Rasterization Platform����д����һ����������DirectX������
		// ��������û������GPU��������ṩDirectX����
		// ��ͨ�����ڿ����Ͳ���Ŀ��
		ComPtr<IDXGIAdapter> pWarpAdapter;// ����һ��ָ��WARP��������ָ��,����������������directx
		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(pWarpAdapter.GetAddressOf())));// ö��WARP������
		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(), // ʹ��WARP������
			D3D_FEATURE_LEVEL_11_0, // �������Լ���Ϊ11.0
			IID_PPV_ARGS(md3dDevice.GetAddressOf())));// ����D3D12�豸
	}
	ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(mFence.GetAddressOf())));// ����D3D12դ��
	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);// ��ȡRTV��������������С
	mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);// ��ȡDSV��������������С
	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);// ��ȡCBV/SRV/UAV��������������С

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;// ���ز��������������ݽṹ
	msQualityLevels.Format = mBackBufferFormat; // ���ö��ز����ĸ�ʽΪ��������������ʽ
	msQualityLevels.SampleCount = 4; // ���ö��ز�����������Ϊ4
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE; // ���ö��ز������������־Ϊ��
	msQualityLevels.NumQualityLevels = 0; // ��ʼ��������������Ϊ0
	ThrowIfFailed(md3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
#ifdef _DEBUG
	LogAdapters();
#endif

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();
	return false;
}

void D3DApp::CreateSwapChain()
{
	mSwapChain.Reset();// ���ý�����ָ��
	DXGI_SWAP_CHAIN_DESC sd = {};//������������
	sd.BufferDesc.Width = mClientWidth; // ���ý������������Ŀ��
	sd.BufferDesc.Height = mClientHeight; // ���ý������������ĸ߶�
	sd.BufferDesc.RefreshRate.Numerator = 60;// ����ˢ���ʷ���
	sd.BufferDesc.RefreshRate.Denominator = 1;// ����ˢ���ʷ�ĸ
	sd.BufferDesc.Format = mBackBufferFormat; // ���ý������������ĸ�ʽ
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // ɨ����˳��δָ��
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // ���ŷ�ʽδָ��
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1; // ���ö��ز�������ݵ�������
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;// ���ö��ز�������ݵ���������
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ���û�������ʹ�÷�ʽ
	sd.BufferCount = SwapChainBufferCount;// ���ý�����������������
	sd.OutputWindow = mhMainWnd; // ����������ھ��
	sd.Windowed = TRUE; // ���ô���ģʽ��false��ʾȫ��ģʽ
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ���ý���Ч��Ϊ��ת����
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ���ñ�־λ
	// ����������
	ThrowIfFailed(mdxgiFactory->CreateSwapChain(
		mCommandQueue.Get(),// �������ָ��
		&sd,// ������������
		mSwapChain.GetAddressOf()// ������ָ��ĵ�ַ
		)
	);
}

void D3DApp::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};// �������������
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // ����������б�־Ϊ��
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // ���������б�����Ϊֱ�������б�
	// �����������
	ThrowIfFailed(md3dDevice->CreateCommandQueue(
		&queueDesc, // �������������
		IID_PPV_ARGS(mCommandQueue.GetAddressOf()) // �������ָ��
	));
	// �������������
	ThrowIfFailed(md3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, // �����б�����Ϊֱ�������б�
		IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf()) // ���������ָ��
	));
	// ���������б�
	ThrowIfFailed(md3dDevice->CreateCommandList(
		0, // �����б�����־Ϊ0
		D3D12_COMMAND_LIST_TYPE_DIRECT, // �����б�����Ϊֱ�������б�
		mDirectCmdListAlloc.Get(), // ���������ָ��
		nullptr, // ��ʼ��ǩ��Ϊnullptr
		IID_PPV_ARGS(mCommandList.GetAddressOf()) // �����б�ָ��
	));
	mCommandList->Close(); // �ر������б���׼���ύ,�մ����������б��ǿ���Reset�ģ�����ʹ�ù��ľ���ҪClose()֮��ſ���Reset()

}

void D3DApp::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};// RTV��������������
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount; // ����RTV����������Ϊ����������������
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // ����������������ΪRTV
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // ���ñ�־λΪ��
	rtvHeapDesc.NodeMask = 0;// �ڵ�����Ϊ0����ʾ���������ѿ���������GPU��ʹ��
	// ����RTV��������
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, // RTV��������������
		IID_PPV_ARGS(mRtvHeap.GetAddressOf()) // RTV��������ָ��
	));
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};// DSV��������������
	dsvHeapDesc.NumDescriptors = 1; // ����DSV����������Ϊ1
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; // ����������������ΪDSV
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // ���ñ�־λΪ��
	dsvHeapDesc.NodeMask = 0; // �ڵ�����Ϊ0����ʾ���������ѿ���������GPU��ʹ��
	// ����DSV��������
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, // DSV��������������
		IID_PPV_ARGS(mDsvHeap.GetAddressOf()) // DSV��������ָ��
	));
}

void D3DApp::OnResize()
{
	assert(md3dDevice);// ȷ��D3D12�豸�Ѵ���
	assert(mSwapChain);// ȷ���������Ѵ���
	assert(mDirectCmdListAlloc);// ȷ������������Ѵ���
	// Flush before changing any resources.
	FlushCommandQueue();// ˢ��������У�ȷ�����������ִ�����
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));// ���������б���׼���µ�����
	for (int i = 0;i < SwapChainBufferCount;i++)
	{
		mSwapChainBuffer[i].Reset(); // ���ý�����������ָ��,���ǽ�������������ָ����Ϊnullptr�ͷ��˽�����
	}
	mDepthStencilBuffer.Reset(); // �������ģ�建����ָ��
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount, // ����������������
		mClientWidth, // ���ڿ��
		mClientHeight, // ���ڸ߶�
		mBackBufferFormat, // ��������������ʽ
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH // ����ģʽ�л�
	));// ������������������С
	mCurrBackBuffer = 0; // ���õ�ǰ�󱸻���������Ϊ0
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());// ��ȡRTV�������ѵ�CPU���������
	for (int i = 0;i < SwapChainBufferCount;i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(mSwapChainBuffer[i].GetAddressOf())));// ��ȡ������������
		md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);// ������ȾĿ����ͼ
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);// �ƶ�����һ��RTV������λ��
	}
	// �������ģ�建����������
	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // ������Դά��Ϊ2D����
	depthStencilDesc.Alignment = 0; // ���뷽ʽΪ0
	depthStencilDesc.Width = mClientWidth; // �������ģ�建�������
	depthStencilDesc.Height = mClientHeight; // �������ģ�建�����߶�'
	depthStencilDesc.DepthOrArraySize = 1; // ������Ȼ������СΪ1
	depthStencilDesc.MipLevels = 1; // ����mip����Ϊ1
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // �������ģ�建������ʽΪR24G8_TYPELESS
	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1; // ���ö��ز�������ݵ�������
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0; // ���ö��ز�������ݵ���������
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // ����������Ϊδ֪
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // ������Դ��־Ϊ�������ģ��

	D3D12_CLEAR_VALUE optClear = {};// ���ֵ
	optClear.Format = mDepthStencilFormat; // �������ģ���ʽ
	optClear.DepthStencil.Depth = 1.0f; // ����������ֵΪ1.0
	optClear.DepthStencil.Stencil = 0; // ����ģ�����ֵΪ0

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT); // �ֲ��������C2102
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&heapProps, // ���ö�����ΪĬ�϶�
		D3D12_HEAP_FLAG_NONE, // ���öѱ�־Ϊ��
		&depthStencilDesc, // ���ģ�建����������
		D3D12_RESOURCE_STATE_COMMON, // ��ʼ��Դ״̬Ϊͨ��״̬
		&optClear, // ���ֵ
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf()) // ���ģ�建����ָ��
	));// �������ģ�建������Դ

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;// ���ģ����ͼ������
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE; // ���ñ�־λΪ��
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // ������ͼά��Ϊ2D����
	dsvDesc.Format = mDepthStencilFormat; // �������ģ���ʽ
	dsvDesc.Texture2D.MipSlice = 0; // ����mip������ƬΪ0
	md3dDevice->CreateDepthStencilView(
		mDepthStencilBuffer.Get(), // ���ģ�建����ָ��
		&dsvDesc, // ���ģ����ͼ������
		mDsvHeap->GetCPUDescriptorHandleForHeapStart() // ��ȡDSV�������ѵ�CPU���������
	);// �������ģ����ͼ
	
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mDepthStencilBuffer.Get(), // ���ģ�建����ָ��
		D3D12_RESOURCE_STATE_COMMON, // ��ʼ��Դ״̬Ϊͨ��״̬
		D3D12_RESOURCE_STATE_DEPTH_WRITE // Ŀ����Դ״̬Ϊ���д��״̬
	);// �����Դ���ϣ�ȷ�����ģ�建��������Ⱦʱ������ȷ��״̬
	mCommandList->ResourceBarrier(1, &barrier);// �������б��������Դ����
}

void D3DApp::FlushCommandQueue()
{
	mCurrentFence++;// ���ӵ�ǰդ��ֵ
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));// �ź�������У��ύ��ǰդ��ֵ��ֻ�е���������е��������ִ����Ϻ�դ��ֵ�Żᱻ����
	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);// ����һ���¼���������ڵȴ�դ��ֵ�ĸ���
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));// ����դ��ֵ���ʱ���¼�
		WaitForSingleObject(eventHandle, INFINITE);// �ȴ��¼���������ֱ��դ��ֵ����
		CloseHandle(eventHandle);// �ر��¼����
	}
}

void D3DApp::LogAdapters()
{
	UINT i = 0;// ����������
	IDXGIAdapter* adapter = nullptr;// ������ָ��
	std::vector<IDXGIAdapter*> adapterList;// �������б�
	while (mdxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc = {};// ������������
		adapter->GetDesc(&desc);// ��ȡ������������
		std::wstring text = L"***Adapter: ";
		text += desc.Description;// �������������
		text += L"\n";
		OutputDebugString(text.c_str());// �����������Ϣ�������������
		adapterList.push_back(adapter);// ����������ӵ��������б�
		i++;
	}
	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
		ReleaseCom(adapterList[i]);
		//{ if (adapterList[i]) { adapterList[i]->Release(); adapterList[i] = 0; } }
	}

}

void D3DApp::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, mBackBufferFormat);// ��־�����ʾģʽ��Ϣ

		ReleaseCom(output);

		++i;
	}
}

void D3DApp::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}

