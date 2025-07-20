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
	//创建硬件适配器
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(md3dDevice.GetAddressOf()));
	if (FAILED(hardwareResult))
	{
		// 如果创建硬件适配器失败，则尝试创建WARP适配器
		// WARP是Windows Advanced Rasterization Platform的缩写，是一个软件虚拟的DirectX适配器
		// 它可以在没有物理GPU的情况下提供DirectX功能
		// 这通常用于开发和测试目的
		ComPtr<IDXGIAdapter> pWarpAdapter;// 创建一个指向WARP适配器的指针,软件虚拟出来的适配directx
		ThrowIfFailed(mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(pWarpAdapter.GetAddressOf())));// 枚举WARP适配器
		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(), // 使用WARP适配器
			D3D_FEATURE_LEVEL_11_0, // 设置特性级别为11.0
			IID_PPV_ARGS(md3dDevice.GetAddressOf())));// 创建D3D12设备
	}
	ThrowIfFailed(md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(mFence.GetAddressOf())));// 创建D3D12栅栏
	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);// 获取RTV描述符的增量大小
	mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);// 获取DSV描述符的增量大小
	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);// 获取CBV/SRV/UAV描述符的增量大小

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;// 多重采样质量级别数据结构
	msQualityLevels.Format = mBackBufferFormat; // 设置多重采样的格式为交换链缓冲区格式
	msQualityLevels.SampleCount = 4; // 设置多重采样的样本数为4
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE; // 设置多重采样质量级别标志为无
	msQualityLevels.NumQualityLevels = 0; // 初始化质量级别数量为0
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
	mSwapChain.Reset();// 重置交换链指针
	DXGI_SWAP_CHAIN_DESC sd = {};//交换链描述符
	sd.BufferDesc.Width = mClientWidth; // 设置交换链缓冲区的宽度
	sd.BufferDesc.Height = mClientHeight; // 设置交换链缓冲区的高度
	sd.BufferDesc.RefreshRate.Numerator = 60;// 设置刷新率分子
	sd.BufferDesc.RefreshRate.Denominator = 1;// 设置刷新率分母
	sd.BufferDesc.Format = mBackBufferFormat; // 设置交换链缓冲区的格式
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // 扫描线顺序未指定
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // 缩放方式未指定
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1; // 设置多重采样抗锯齿的样本数
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;// 设置多重采样抗锯齿的质量级别
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 设置缓冲区的使用方式
	sd.BufferCount = SwapChainBufferCount;// 设置交换链缓冲区的数量
	sd.OutputWindow = mhMainWnd; // 设置输出窗口句柄
	sd.Windowed = TRUE; // 设置窗口模式，false表示全屏模式
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 设置交换效果为翻转丢弃
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // 设置标志位
	// 创建交换链
	ThrowIfFailed(mdxgiFactory->CreateSwapChain(
		mCommandQueue.Get(),// 命令队列指针
		&sd,// 交换链描述符
		mSwapChain.GetAddressOf()// 交换链指针的地址
		)
	);
}

void D3DApp::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};// 命令队列描述符
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // 设置命令队列标志为无
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 设置命令列表类型为直接命令列表
	// 创建命令队列
	ThrowIfFailed(md3dDevice->CreateCommandQueue(
		&queueDesc, // 命令队列描述符
		IID_PPV_ARGS(mCommandQueue.GetAddressOf()) // 命令队列指针
	));
	// 创建命令分配器
	ThrowIfFailed(md3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, // 命令列表类型为直接命令列表
		IID_PPV_ARGS(mDirectCmdListAlloc.GetAddressOf()) // 命令分配器指针
	));
	// 创建命令列表
	ThrowIfFailed(md3dDevice->CreateCommandList(
		0, // 命令列表创建标志为0
		D3D12_COMMAND_LIST_TYPE_DIRECT, // 命令列表类型为直接命令列表
		mDirectCmdListAlloc.Get(), // 命令分配器指针
		nullptr, // 初始根签名为nullptr
		IID_PPV_ARGS(mCommandList.GetAddressOf()) // 命令列表指针
	));
	mCommandList->Close(); // 关闭命令列表以准备提交,刚创建的命令列表是可以Reset的，但是使用过的就需要Close()之后才可以Reset()

}

void D3DApp::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};// RTV描述符堆描述符
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount; // 设置RTV描述符数量为交换链缓冲区数量
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // 设置描述符堆类型为RTV
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // 设置标志位为无
	rtvHeapDesc.NodeMask = 0;// 节点掩码为0，表示该描述符堆可以在所有GPU上使用
	// 创建RTV描述符堆
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, // RTV描述符堆描述符
		IID_PPV_ARGS(mRtvHeap.GetAddressOf()) // RTV描述符堆指针
	));
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};// DSV描述符堆描述符
	dsvHeapDesc.NumDescriptors = 1; // 设置DSV描述符数量为1
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; // 设置描述符堆类型为DSV
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // 设置标志位为无
	dsvHeapDesc.NodeMask = 0; // 节点掩码为0，表示该描述符堆可以在所有GPU上使用
	// 创建DSV描述符堆
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, // DSV描述符堆描述符
		IID_PPV_ARGS(mDsvHeap.GetAddressOf()) // DSV描述符堆指针
	));
}

void D3DApp::OnResize()
{
	assert(md3dDevice);// 确保D3D12设备已创建
	assert(mSwapChain);// 确保交换链已创建
	assert(mDirectCmdListAlloc);// 确保命令分配器已创建
	// Flush before changing any resources.
	FlushCommandQueue();// 刷新命令队列，确保所有命令都已执行完毕
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));// 重置命令列表以准备新的命令
	for (int i = 0;i < SwapChainBufferCount;i++)
	{
		mSwapChainBuffer[i].Reset(); // 重置交换链缓冲区指针,就是将交换链缓冲区指针置为nullptr释放了交换链
	}
	mDepthStencilBuffer.Reset(); // 重置深度模板缓冲区指针
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount, // 交换链缓冲区数量
		mClientWidth, // 窗口宽度
		mClientHeight, // 窗口高度
		mBackBufferFormat, // 交换链缓冲区格式
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH // 允许模式切换
	));// 调整交换链缓冲区大小
	mCurrBackBuffer = 0; // 重置当前后备缓冲区索引为0
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());// 获取RTV描述符堆的CPU描述符句柄
	for (int i = 0;i < SwapChainBufferCount;i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(mSwapChainBuffer[i].GetAddressOf())));// 获取交换链缓冲区
		md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);// 创建渲染目标视图
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);// 移动到下一个RTV描述符位置
	}
	// 创建深度模板缓冲区描述符
	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 设置资源维度为2D纹理
	depthStencilDesc.Alignment = 0; // 对齐方式为0
	depthStencilDesc.Width = mClientWidth; // 设置深度模板缓冲区宽度
	depthStencilDesc.Height = mClientHeight; // 设置深度模板缓冲区高度'
	depthStencilDesc.DepthOrArraySize = 1; // 设置深度或数组大小为1
	depthStencilDesc.MipLevels = 1; // 设置mip级别为1
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // 设置深度模板缓冲区格式为R24G8_TYPELESS
	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1; // 设置多重采样抗锯齿的样本数
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0; // 设置多重采样抗锯齿的质量级别
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN; // 设置纹理布局为未知
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // 设置资源标志为允许深度模板

	D3D12_CLEAR_VALUE optClear = {};// 清除值
	optClear.Format = mDepthStencilFormat; // 设置深度模板格式
	optClear.DepthStencil.Depth = 1.0f; // 设置深度清除值为1.0
	optClear.DepthStencil.Stencil = 0; // 设置模板清除值为0

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT); // 局部变量解决C2102
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&heapProps, // 设置堆属性为默认堆
		D3D12_HEAP_FLAG_NONE, // 设置堆标志为无
		&depthStencilDesc, // 深度模板缓冲区描述符
		D3D12_RESOURCE_STATE_COMMON, // 初始资源状态为通用状态
		&optClear, // 清除值
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf()) // 深度模板缓冲区指针
	));// 创建深度模板缓冲区资源

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;// 深度模板视图描述符
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE; // 设置标志位为无
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 设置视图维度为2D纹理
	dsvDesc.Format = mDepthStencilFormat; // 设置深度模板格式
	dsvDesc.Texture2D.MipSlice = 0; // 设置mip级别切片为0
	md3dDevice->CreateDepthStencilView(
		mDepthStencilBuffer.Get(), // 深度模板缓冲区指针
		&dsvDesc, // 深度模板视图描述符
		mDsvHeap->GetCPUDescriptorHandleForHeapStart() // 获取DSV描述符堆的CPU描述符句柄
	);// 创建深度模板视图
	
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mDepthStencilBuffer.Get(), // 深度模板缓冲区指针
		D3D12_RESOURCE_STATE_COMMON, // 初始资源状态为通用状态
		D3D12_RESOURCE_STATE_DEPTH_WRITE // 目标资源状态为深度写入状态
	);// 添加资源屏障，确保深度模板缓冲区在渲染时处于正确的状态
	mCommandList->ResourceBarrier(1, &barrier);// 在命令列表中添加资源屏障
}

void D3DApp::FlushCommandQueue()
{
	mCurrentFence++;// 增加当前栅栏值
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));// 信号命令队列，提交当前栅栏值，只有当命令队列中的所有命令都执行完毕后，栅栏值才会被更新
	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);// 创建一个事件句柄，用于等待栅栏值的更新
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));// 设置栅栏值完成时的事件
		WaitForSingleObject(eventHandle, INFINITE);// 等待事件被触发，直到栅栏值更新
		CloseHandle(eventHandle);// 关闭事件句柄
	}
}

void D3DApp::LogAdapters()
{
	UINT i = 0;// 适配器索引
	IDXGIAdapter* adapter = nullptr;// 适配器指针
	std::vector<IDXGIAdapter*> adapterList;// 适配器列表
	while (mdxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc = {};// 适配器描述符
		adapter->GetDesc(&desc);// 获取适配器描述符
		std::wstring text = L"***Adapter: ";
		text += desc.Description;// 添加适配器描述
		text += L"\n";
		OutputDebugString(text.c_str());// 输出适配器信息到调试输出窗口
		adapterList.push_back(adapter);// 将适配器添加到适配器列表
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

		LogOutputDisplayModes(output, mBackBufferFormat);// 日志输出显示模式信息

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

