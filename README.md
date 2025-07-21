# DirectX12LearnProject
# DirectX12学习仓库

## 什么是顶点缓冲区，什么是顶点缓冲区视图？
顶点缓冲区是 GPU 内存中存储顶点数据的区域，包含了 3D 模型的所有几何信息（如位置、法线、纹理坐标等），由辅助描述结构体CD3DX12_RESOURCE_DESC所描述并由ID3D12Device::CreateCommittedResource函数所创建。
```cpp
流程如下
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