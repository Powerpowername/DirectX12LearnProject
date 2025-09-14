# DirectX12LearnProject
# DirectX12学习仓库
# 资源类型与访问方式
DirectX 12 将 GPU 资源分为两类：

### 1 通过描述符堆访问的资源（如纹理、常量缓冲区、采样器）
这些资源需要在描述符堆中创建对应的描述符（Descriptor）
着色器通过描述符堆中的索引间接访问这些资源
例如：纹理 SRV、常量缓冲区 CBV、采样器 Sampler
### 2 直接绑定的资源（如顶点缓冲区、索引缓冲区）
这些资源通过专用的命令直接绑定到渲染管线
不需要经过描述符堆，而是通过视图（View）结构指定资源位置和格式

## 什么是顶点缓冲区，什么是顶点缓冲区视图？
顶点缓冲区是 GPU 内存中存储顶点数据的区域，包含了 3D 模型的所有几何信息（如位置、法线、纹理坐标等），由辅助描述结构体CD3DX12_RESOURCE_DESC所描述并由ID3D12Device::CreateCommittedResource函数所创建。
```cpp
//流程如下
// 1. 创建默认缓冲区资源（GPU可访问）
ComPtr<ID3D12Resource> vertexBuffer;//顶点缓冲区
device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),//在GPU端创建默认堆，存储静态数据提高效率
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),//顶点尺寸以字节为基准
    D3D12_RESOURCE_STATE_COPY_DEST,
    nullptr,
    IID_PPV_ARGS(&vertexBuffer)
);

// 2. 创建上传缓冲区（CPU可写，用于初始化数据）
ComPtr<ID3D12Resource> vertexBufferUploadHeap;
    device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),//因为CPU无法直接操作GPU内存，所以将数据先存在CPU端的上传堆，让GPU自己去拿，符合硬件物理架构
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&vertexBufferUploadHeap)
);

// 3. 将顶点数据从CPU复制到上传缓冲区
Vertex vertices[] = { /* 顶点数据 */ };
D3D12_SUBRESOURCE_DATA vertexData = {};//用于描述需要复制到 GPU 资源（如缓冲区、纹理）中的CPU 端数据的布局和位置。
vertexData.pData = vertices;//数CPU 端数据起始地址的指针。
vertexData.RowPitch = vertexBufferSize;//数据行的字节大小，顶点数据一般是一维数据
vertexData.SlicePitch = vertexData.RowPitch;//SlicePitch 表示数据切片的字节大小，主要用于三维纹理或纹理数组。一个切片的字节数，类似于快递包裹的纵向高度。

// 4. 使用命令列表将数据从上传缓冲区复制到默认缓冲区
UpdateSubresources(commandList.Get(), vertexBuffer.Get(), 
    vertexBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
```
顶点缓冲是一个轻量级描述符，告诉 GPU 如何解析顶点缓冲区中的数据。它不存储实际数据，而是定义数据的布局和位置,由结构体D3D12_VERTEX_BUFFER_VIEW定义。
```cpp
typedef struct D3D12_VERTEX_BUFFER_VIEW {
    D3D12_GPU_VIRTUAL_ADDRESS BufferLocation;  // 缓冲区GPU地址
    UINT                       SizeInBytes;     // 总大小（字节）
    UINT                       StrideInBytes;   // 每个顶点的字节步长
} D3D12_VERTEX_BUFFER_VIEW;
```
BufferLocation：顶点缓冲区的 GPU 虚拟地址（通过vertexBuffer->GetGPUVirtualAddress()获取）。
SizeInBytes：整个顶点缓冲区的大小（例如sizeof(vertices)）。
StrideInBytes：相邻两个顶点之间的字节偏移量（例如sizeof(Vertex)）。
```cpp
// 创建顶点视图
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
vertexBufferView.StrideInBytes = sizeof(Vertex);
vertexBufferView.SizeInBytes = sizeof(vertices);

// 在命令列表中绑定顶点视图到输入装配器阶段,这样GPU才知道怎么用
commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
```
## 什么是常量缓冲区，什么是常量缓冲区描述符？什么是根签名？（CBV）
常量缓冲区是一块GPU 内存，用于存储 shader 程序运行时不会改变的变量（如世界矩阵、视图矩阵、光照参数）。它是数据的实际存储位置，可被多个 shader（顶点着色器、像素着色器等）共享访问。
通常位于 GPU 显存（如 DirectX 的 D3D12_HEAP_TYPE_DEFAULT）或 CPU-GPU 共享内存（如 D3D12_HEAP_TYPE_UPLOAD）。DirectX12暂时看到是使用D3D12_HEAP_TYPE_UPLOAD创建的
```cpp
// 创建常量缓冲区（位于上传堆，支持CPU写入）
ComPtr<ID3D12Resource> constantBuffer;
device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(1024),  // 1KB缓冲区
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&constantBuffer)
);

// CPU写入数据
void* dataBegin;
CD3DX12_RANGE readRange(0, 0);
constantBuffer->Map(0, &readRange, &dataBegin);//Map函数获取常量缓冲区的存储地址，好让CPU写入
memcpy(dataBegin, &myConstantData, sizeof(myConstantData));//将数据写入常量缓冲区
constantBuffer->Unmap(0, nullptr);//使用Unmap函数将数据提交，让GPU去读取
```
常量描述符是一个元数据对象，用于描述常量缓冲区的属性和位置，帮助 GPU 快速定位和访问常量缓冲区中的数据。它是常量缓冲区的 “指针”，不存储实际数据，而是指向存储数据的常量缓冲区。
```cpp
// 创建常量描述符堆
D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};//描述符堆描述结构体
heapDesc.NumDescriptors = 1;
heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;  // 常量缓冲区描述符类型
heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

// 创建常量缓冲区视图（CBV，即常量描述符）
D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
cbvDesc.SizeInBytes = (sizeof(myConstantData) + 255) & ~255;  // 按256字节对齐，可以看看是什么原理
device->CreateConstantBufferView(&cbvDesc, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
```
根签名到底是什么？其实就是将数据绑定到HLSL中的那些寄存器当中，以形成各式各样的函数着色器函数签名
![alt text](image.png)
```cpp
//创建流程如下

// 根参数数组
CD3DX12_ROOT_PARAMETER rootParameters[2];
    
// 参数0：常量缓冲区视图（CBV）
rootParameters[0].InitAsConstantBufferView(0);  // 寄存器b0
    
// 参数1：描述符表（包含多个SRV）
CD3DX12_DESCRIPTOR_RANGE descriptorRange;
descriptorRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);  // 1个SRV，寄存器t0
rootParameters[1].InitAsDescriptorTable(1, &descriptorRange);
    
// 静态采样器
CD3DX12_STATIC_SAMPLER_DESC staticSampler(
    0,  // 寄存器s0
    D3D12_FILTER_MIN_MAG_MIP_LINEAR
);
    
// 根签名描述
CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(
    _countof(rootParameters),
    rootParameters,
    1,
    &staticSampler,
    D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
);
    
// 编译根签名
ComPtr<ID3DBlob> signature;
ComPtr<ID3DBlob> error;
D3D12SerializeRootSignature(
    &rootSignatureDesc,
    D3D_ROOT_SIGNATURE_VERSION_1,
    &signature,
    &error
);
    
// 创建根签名对象
ComPtr<ID3D12RootSignature> rootSignature;
device->CreateRootSignature(
    0,
    signature->GetBufferPointer(),
    signature->GetBufferSize(),
    IID_PPV_ARGS(&rootSignature)
);
```

# 几种堆位于什么位置？
## D3D12_HEAP_TYPE_DEFAULT（默认堆）
内存位置：GPU 专用内存（通常是显卡的 VRAM）。
特点：
只能被 GPU 直接访问，CPU 无法直接读写（必须通过 “上传堆” 间接复制数据到这里）。
带宽最高，延迟最低，是性能敏感资源（如顶点缓冲区、纹理）的首选。
示例：如果你的顶点缓冲区是用 D3D12_HEAP_TYPE_DEFAULT 创建的，它就位于 GPU 内存中。
## D3D12_HEAP_TYPE_UPLOAD（上传堆）
内存位置：CPU 可访问的内存（通常是系统内存中被 GPU 映射的区域）。
特点：
CPU 可以直接读写（通过 Map() 映射后操作），GPU 也可以读取。
常用于临时数据传输（如动态更新的顶点数据），性能略低于默认堆。
示例：如果用上传堆创建缓冲区，它位于 CPU 可访问内存，但 GPU 可以通过总线读取。
## D3D12_HEAP_TYPE_READBACK（回读堆）
内存位置：CPU 可访问的内存（系统内存中 GPU 可写入的区域）。
特点：
主要用于 GPU 向 CPU 传输数据（如读取渲染结果），CPU 可读，GPU 可写。
性能较低，通常用于调试或离线数据处理。
## 描述符堆
描述符堆时专门用来存储描述符的堆，通常情况下GUP在使用数据时需要借助于描述符/视图来了解如何解析数据，GPU需要从CPU端内存或获取到描述符堆中的描述符才能知道如何使用数据，常用描述符堆：RTV描述符堆，DSV描述符堆等
### 创建描述符通用格式解释
![alt text](READMResouce\ViewExplain.png)
# 堆内存演示
## 描述符堆
![alt text](READMResouce\heap.png)
## 默认堆，上传堆，回读堆
![alt text](READMResouce\heap-1.png)

注：着色器可见资源由根签名和描述符表管理。非着色器可见资源由命令列表直接管理。
# 几种堆资源的创建与使用
## 描述符堆（RTV,DSV,CSV等）
```cpp
//渲染缓冲区，渲染对象，后面需要将交换链中的内存交给他管理
ComPtr<ID3D12Resource> renderTargets[FrameCount];

ComPtr<ID3D12DescriptorHeap> rtvHeap;//rtv描述符堆
//创建描述符堆描述结构
D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
rtvHeapDesc.NumDescriptors = FrameCount;//描述符的数量
rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//指定堆类型
rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//创建描述符堆
ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf())));
//描述符大小增量，供后续寻找描述符使用
rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//找到描述符堆的起始位置
CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
//使用CreateRenderTargetView根据资源在描述符堆中创建描述符
for (unsigned int i = 0; i < FrameCount; i++)
{
    //因为后台缓冲区是已经存在的资源，所以并不需要CreateCommittedResource显式创建资源，但是深度缓冲区等需要
    /*
    *	ThrowIfFailed(device->CreateCommittedResource(
	*	&heapProperties2,//一般在默认堆上
	*	D3D12_HEAP_FLAG_NONE,
	*	&tex2D,
	*	D3D12_RESOURCE_STATE_DEPTH_WRITE,
	*	&depthOptimizedClearValue,
	*	IID_PPV_ARGS(&depthStencilBuffer)));//在默认堆上创建深度模板缓冲区资源
    *   device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
    */
	ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargets[i].GetAddressOf())));//将交换链当中的后台缓冲区交给renderTargets
	device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);//将描述符堆的第一个描述符用于创建RTV
	rtvHandle.Offset(1, rtvDescriptorSize);//转到描述符堆的下一个描述符
}
//注：需要主动处理屏障，变换帧状态
D3D12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
commandList->ResourceBarrier(1, &resBarrier);//1为本次提交的资源屏障数
/*
*   //着色器可见资源由根签名中的根参数来访问，着色器不可见资源由命令列表直接帮定
*   //此处需要显示传递常量缓冲区堆顶指针
*   ID3D12DescriptorHeap* ppHeaps[] = { cbvHeap.Get() };
*   commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);//设置描述符堆,这个堆是可见的，GPU可以直接访问
*   commandList->SetGraphicsRootDescriptorTable(0, cbvHeap->GetGPUDescriptorHandleForHeapStart());
*/
//找到后台第一个渲染帧
rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
commandList->ResourceBarrier(1, &resBarrier);
ThrowIfFailed(commandList->Close());
```
# 上传堆
```cpp
//顶点缓冲区与定点视图
ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
//数据尺寸
const UINT vertexBufferSize = sizeof(triangleVertices);
//创建资源
CD3DX12_RESOURCE_DESC vertexBufferResourceDes = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
//创建顶点缓冲区资源
CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);//上传堆类型
ThrowIfFailed(device->CreateCommittedResource(
	&heapProperties, 
	D3D12_HEAP_FLAG_NONE,
	&vertexBufferResourceDes,
	D3D12_RESOURCE_STATE_GENERIC_READ,
	nullptr,
	IID_PPV_ARGS(vertexBuffer.GetAddressOf())));//创建缓冲区资源,vertexBuffer是属于堆管理的内存
//上传堆的通用操做
UINT8* pDataBegin;
CD3DX12_RANGE readRange(0, 0);//读取范围,是为了告诉驱动程序我们不会读取这个内存
ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));//映射缓冲区到CPU可读内存
memcpy(pDataBegin, triangleVertices, sizeof(triangleVertices));//将顶点数据复制到缓冲区
vertexBuffer->Unmap(0, nullptr);//取消映射缓冲区，第二个参数是CD3DX12_RANGE

vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();//获取GPU虚拟地址，因为这个顶点缓冲视图是需要给GPU用的
vertexBufferView.StrideInBytes = sizeof(Vertex);//顶点缓冲区步长
vertexBufferView.SizeInBytes = vertexBufferSize;
//设置顶点缓冲区视图，
//注：需要主动处理屏障，变换帧状态
D3D12_RESOURCE_BARRIER resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
commandList->ResourceBarrier(1, &resBarrier);//1为本次提交的资源屏障数
commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
resBarrier = CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
commandList->ResourceBarrier(1, &resBarrier);
ThrowIfFailed(commandList->Close());
```



# 堆资源分为资源堆与描述符堆
```cpp
\\资源堆创建方式
ID3D12Device::CreateCommittedResource(
  const D3D12_HEAP_PROPERTIES *pHeapProperties, //堆属性
  D3D12_HEAP_FLAGS HeapFlags,                   //堆标志
  const D3D12_RESOURCE_DESC *pDesc,             //资源描述
  D3D12_RESOURCE_STATES InitialResourceState,    //初始资源状态
  const D3D12_CLEAR_VALUE *pOptimizedClearValue,//优化清除值
  REFIID riid,                                  //接口ID
  void **ppvResource                            //输出资源指针
);

\\描述符堆创建方式
ID3D12Device::CreateDescriptorHeap(
  const D3D12_DESCRIPTOR_HEAP_DESC *pDescriptorHeapDesc, //描述符堆描述
  REFIID riid,                                           //接口ID
  void **ppvHeap                                        //输出描述符堆指针
);


\\资源操作资源
ID3D12Resource::Map(
  UINT Subresource,            //子资源索引
  const D3D12_RANGE *pReadRange,//读取范围
  void **ppData                //输出数据指针
);

\\描述符堆操作资源，借助描述符句柄
D3D12_CPU_DESCRIPTOR_HANDLE D3D12_DESCRIPTOR_HEAP::GetCPUDescriptorHandleForHeapStart();

```