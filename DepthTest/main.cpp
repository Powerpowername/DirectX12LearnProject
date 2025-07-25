#include <windows.h>
#include <d3d12.h>//directx12 API ��
#include <dxgi1_4.h> //DirectX ͼ�λ�����ʩ (DXGI) 
#include <wrl.h>
#include <iostream>
#include <d3dx12.h >
#include <DirectXMath.h>
#include <d3dcompiler.h>
//#include <xstring>
#pragma comment(lib,"d3dcompiler.lib") //������ɫ����Ҫ�Ŀ�
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
//#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
//#pragma comment(linker, "/ENTRY:WinMain")
using namespace Microsoft::WRL;
using namespace DirectX;
const UINT FrameCount = 2;
UINT width = 800;
UINT height = 600;
HWND hwnd;//���ھ��

//���߶���
CD3DX12_VIEWPORT viewport(0.0f, 0.0f, width, height);
CD3DX12_RECT scissorRect(0, 0, width, height);//�ü�����
ComPtr<IDXGISwapChain3> swapChain;
ComPtr<ID3D12Device> device;
ComPtr<ID3D12Resource> renderTargets[FrameCount];//��Ⱦ���󣬺�����Ҫ���������е��ڴ潻��������
ComPtr<ID3D12CommandAllocator> commandAllocator;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<ID3D12DescriptorHeap> rtvHeap;//rtv��������
ComPtr<ID3D12DescriptorHeap> dsvHeap;
ComPtr<ID3D12PipelineState> pipelineState;
ComPtr<ID3D12GraphicsCommandList> commandList;
UINT rtvDescriptorSize;//RTV�����ߴ�
ComPtr<ID3D12RootSignature> rootSignature;//��ǩ��
//shader
ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
ComPtr<ID3D12Resource> indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;
ComPtr<ID3D12Resource> depthStencilBuffer;
//ͬ������
UINT frameIndex;//��ǰ֡����
HANDLE fenceEvent;
ComPtr<ID3D12Fence> fence;
UINT64 fenceValue;

float color[3];
bool isRAdd = true;
bool isGAdd = true;
bool isBAdd = true;

struct Vertex
{
	XMFLOAT3 pos;   // λ��
	XMFLOAT4 color; // ��ɫ
};

std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

class HrException : public std::runtime_error
{
public:
	HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;
};

void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw HrException(hr);
	}
}

IDXGIAdapter1* GetSupportedAdapter(ComPtr<IDXGIFactory4>& dxgiFactory, const D3D_FEATURE_LEVEL featureLevel)
{
	IDXGIAdapter1* adapter = nullptr;
	for (unsigned int adapterIndex = 0;; ++adapterIndex)
	{
		IDXGIAdapter1* currentAdapter = nullptr;//��߿���ֱ��ʹ��ComPtr
		if (DXGI_ERROR_NOT_FOUND == dxgiFactory->EnumAdapters1(adapterIndex, &currentAdapter))//ֻ��ö����������Ҫ��û����������ֱ���˳�
		{
			break;
		}
		//IID_PPV_ARGS(ppType) __uuidof(**(ppType)), IID_PPV_ARGS_Helper(ppType) �˴� __uuidof(**(ppType))�е�(**(ppType))��Ϊ���ñ������Ƶ������ͣ��������Ƕ��󣬸�����������GUID
		const HRESULT hres = D3D12CreateDevice(currentAdapter, featureLevel, _uuidof(ID3D12Device), nullptr);//���һ����������nullptr�Ϳ���ֻ������������Զ���ȥʵ�ʴ����豸�������
		if (SUCCEEDED(hres))
		{
			adapter = currentAdapter;
			break;
		}
		currentAdapter->Release();//�ͷŲ�֧�ֵ�������
	}
	return adapter;
}

void LoadPipeline()
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

ComPtr<IDXGIFactory4> mDxgiFactory;//���Դ�����������������
ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(mDxgiFactory.GetAddressOf())));
D3D_FEATURE_LEVEL featureLevels[] =
{
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0
};
D3D_FEATURE_LEVEL select_feature;
IDXGIAdapter1* adapter = nullptr;
for (unsigned int i = 0; i < _countof(featureLevels); i++)
{
	adapter = GetSupportedAdapter(mDxgiFactory, featureLevels[i]);
	if (adapter != nullptr)
	{
		select_feature = featureLevels[i];
		break;
	}
}
if (adapter != nullptr)
{
	//�ҵ�����Ҫ�����ʾ������������device
	D3D12CreateDevice(adapter, select_feature, IID_PPV_ARGS(device.GetAddressOf()));
}
D3D12_COMMAND_QUEUE_DESC queueDesc{};
queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(commandQueue.GetAddressOf())));

DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
swapChainDesc.BufferCount = FrameCount;
swapChainDesc.Width = width;
swapChainDesc.Height = height;
swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
swapChainDesc.SampleDesc.Count = 1;
ComPtr<IDXGISwapChain1> swapChain1;
ThrowIfFailed(mDxgiFactory->CreateSwapChainForHwnd(
	commandQueue.Get(),
	hwnd,
	&swapChainDesc,
	nullptr,
	nullptr,
	&swapChain1
));//API��������ֻ׼ʹ��IDXGISwapChain1��Ͱ汾
ThrowIfFailed(swapChain1.As(&swapChain));
frameIndex = swapChain->GetCurrentBackBufferIndex();//��ȡ����ǰ֡������
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;//������������
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf())));
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);//��������С����

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)));

}
CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
for (unsigned int i = 0; i < FrameCount; i++)
{
	ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargets[i].GetAddressOf())));//�����������еĺ�̨����������renderTargets
	device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);//���������ѵĵ�һ�����������ڴ���RTV
	rtvHandle.Offset(1, rtvDescriptorSize);//ת���������ѵ���һ��������
}
ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator.GetAddressOf())));

}

void LoadAsset()
{
	//������ǩ��
	{
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		if (error != nullptr)
		{
			std::cerr << "Error creating root signature: " << static_cast<const char*>(error->GetBufferPointer()) << std::endl;
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
		ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature.GetAddressOf())));
	}
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ThrowIfFailed(D3DCompileFromFile(std::wstring(L"../Assets/shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(std::wstring(L"../Assets/shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
		};//���벼��������
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};//��Ⱦ����״̬������
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };//���벼��
		psoDesc.pRootSignature = rootSignature.Get();//��ǩ��
		psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };//������ɫ��
		psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };//������ɫ��
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);//��դ��״̬
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);//���״̬
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);//���ģ��״̬
		psoDesc.SampleMask = UINT_MAX;//��������
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//ͼԪ��������
		psoDesc.NumRenderTargets = 1;//��ȾĿ������
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//��ȾĿ���ʽ
		psoDesc.SampleDesc.Count = 1;//����������
		//psoDesc.SampleDesc.Quality = 0;//��������
		ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.GetAddressOf())));//������Ⱦ����״̬����
	}


	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));
	ThrowIfFailed(commandList->Close());
	{
		Vertex triangleVertices[] =
		{
                        //��һ����ɫ�ı���
			{ { -0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { 0.5f, -0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.5f, -0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { 0.5f, 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
                        //�ڶ�����ɫ�ı���
			{ { -0.75f, 0.75f, 0.7f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
			{ { 0.0f, 0.0f, 0.7f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
			{ { -0.75f, 0.0f, 0.7f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
			{ { 0.0f, 0.75f, 0.7f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
		};
		DWORD triangleIndexs[]
		{
			0,1,2,
			0,3,1
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);
		const UINT indexBufferSize = sizeof(triangleIndexs);

		CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);//�ϴ�������)
		CD3DX12_RESOURCE_DESC vertexBufferResourceDes = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		CD3DX12_RESOURCE_DESC indexBufferResourceDes = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
		//��������Դ������
		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferResourceDes,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(vertexBuffer.GetAddressOf())));//������������Դ,vertexBuffer�����ڶѹ�����ڴ�

		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferResourceDes,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&indexBuffer)));

		UINT8* pDataBegin;
		CD3DX12_RANGE readRange(0, 0);//��ȡ��Χ,��Ϊ�˸��������������ǲ����ȡ����ڴ�
		ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));//ӳ�仺������CPU�ɶ��ڴ�
		memcpy(pDataBegin, triangleVertices, sizeof(triangleVertices));//���������ݸ��Ƶ�������
		vertexBuffer->Unmap(0, nullptr);//ȡ��ӳ�仺�������ڶ���������CD3DX12_RANGE

		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();//��ȡGPU�����ַ����Ϊ������㻺����ͼ����Ҫ��GPU�õ�
		vertexBufferView.StrideInBytes = sizeof(Vertex);//���㻺��������
		vertexBufferView.SizeInBytes = vertexBufferSize;

		ThrowIfFailed(indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
		memcpy(pDataBegin, triangleIndexs, sizeof(triangleIndexs));
		indexBuffer->Unmap(0, nullptr);

		indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		indexBufferView.SizeInBytes = indexBufferSize;

		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		CD3DX12_HEAP_PROPERTIES heapProperties2 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC tex2D = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		ThrowIfFailed(device->CreateCommittedResource(
			&heapProperties2,
			D3D12_HEAP_FLAG_NONE,
			&tex2D,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&depthStencilBuffer)));

		device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	}
	{
		ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
		fenceValue = 0;

		fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}
	}
}


void PopulateCommendList()//��������б�
{
	ThrowIfFailed(commandAllocator->Reset());//��Ҫ������������������������������б�
	ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState.Get()));//���������Ҫʹ��������������ڴ�����ϴ����

	commandList->SetGraphicsRootSignature(rootSignature.Get());//�������б������ø�ǩ��
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);//�����ӿںͲü�����


	D3D12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &resBarrier);//1Ϊ�����ύ����Դ������

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());

	const float clearColor[] = { color[0], color[1], color[2], 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);//��һ����ɫ�ı���
	commandList->DrawIndexedInstanced(6, 1, 0, 4, 0);//�ڶ�����ɫ�ı���,ʹ����ʼ����ƫ������������

	resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &resBarrier);
	ThrowIfFailed(commandList->Close());
}

void WaitForPreviousFrame()
{
	const UINT64 tempFenceValue = fenceValue;
	ThrowIfFailed(commandQueue->Signal(fence.Get(), tempFenceValue));
	fenceValue++;
	if (fence->GetCompletedValue() < tempFenceValue)
	{
		ThrowIfFailed(fence->SetEventOnCompletion(tempFenceValue, fenceEvent));
		WaitForSingleObject(fenceEvent, INFINITE);//�ȴ��¼����,���߳�����
	}
	frameIndex = swapChain->GetCurrentBackBufferIndex();//��ȡ��ǰ֡����
}


void OnUpdate()
{

	if (color[0] <= 1.0f && isRAdd)
	{
		color[0] += 0.001f;
		isRAdd = true;
	}
	else
	{
		color[0] -= 0.002f;
		color[0] <= 0 ? isRAdd = true : isRAdd = false;

	}

	if (color[1] <= 1.0f && isGAdd)
	{
		color[1] += 0.002f;
		isGAdd = true;
	}
	else
	{
		color[1] -= 0.001f;
		color[1] <= 0 ? isGAdd = true : isGAdd = false;

	}

	if (color[2] <= 1.0f && isBAdd)
	{
		color[2] += 0.001f;
		isBAdd = true;
	}
	else
	{
		color[2] -= 0.001f;
		color[2] <= 0 ? isBAdd = true : isBAdd = false;

	}

	color[3] = 1.0f;

}

void OnRender()
{
	PopulateCommendList();
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	ThrowIfFailed(swapChain->Present(1, 0)); //��������Present��������һ������1��ʾ��ֱͬ����0��ʾû������ѡ��ڶ�������ͨ��ֱ������0
	WaitForPreviousFrame();
}

void OnDestroy()
{
	WaitForPreviousFrame();

	CloseHandle(fenceEvent);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		OnUpdate();
		OnRender();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = L"YSHRenderClass";

	RegisterClassEx(&windowClass);

	hwnd = CreateWindow(
		windowClass.lpszClassName,
		L"YSHRender",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	LoadPipeline();
	LoadAsset();

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

	OnDestroy();

	return 0;
}