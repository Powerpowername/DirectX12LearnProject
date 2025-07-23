//#include "d3dUtil.h"
//#include <DirectXMath.h>
//#include <DirectXPackedVector.h>
//
////using namespace DirectX;
//using namespace DirectX::PackedVector;
//using Microsoft::WRL::ComPtr;
//struct Vertex
//{
//	// 顶点结构体，包含位置和颜色信息
//	DirectX::XMFLOAT3 Pos;   // 位置
//	DirectX::XMFLOAT4 Color; // 颜色
//};
//
//
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
//
//struct Vertex {
//	DirectX::XMFLOAT3 pos;   // 位置
//	DirectX::XMFLOAT4 color; // 颜色
//};
//
//Microsoft::WRL::ComPtr<ID3D12Resource> d3dUtil::CreateDefaultBuffer(
//    ID3D12Device* device,
//    ID3D12GraphicsCommandList* cmdList,
//    const void* initData,
//    UINT64 byteSize,
//    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
//{
//    ComPtr<ID3D12Resource> defaultBuffer;
//
//    // Create the actual default buffer resource.
//    ThrowIfFailed(device->CreateCommittedResource(
//        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
//        D3D12_HEAP_FLAG_NONE,
//        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
//        D3D12_RESOURCE_STATE_COMMON,
//        nullptr,
//        IID_PPV_ARGS(defaultBuffer.GetAddressOf())));
//
//    // In order to copy CPU memory data into our default buffer, we need to create
//    // an intermediate upload heap. 
//    ThrowIfFailed(device->CreateCommittedResource(
//        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//        D3D12_HEAP_FLAG_NONE,
//        &CD3DX12_RESOURCE_DESC::Buffer(byteSize),
//        D3D12_RESOURCE_STATE_GENERIC_READ,
//        nullptr,
//        IID_PPV_ARGS(uploadBuffer.GetAddressOf())));
//
//
//    // Describe the data we want to copy into the default buffer.
//    D3D12_SUBRESOURCE_DATA subResourceData = {};
//    subResourceData.pData = initData;
//    subResourceData.RowPitch = byteSize;
//    subResourceData.SlicePitch = subResourceData.RowPitch;
//
//    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
//    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
//    // the intermediate upload heap data will be copied to mBuffer.
//    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
//        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
//    UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
//    cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
//        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
//
//    // Note: uploadBuffer has to be kept alive after the above function calls because
//    // the command list has not been executed yet that performs the actual copy.
//    // The caller can Release the uploadBuffer after it knows the copy has been executed.
//
//
//    return defaultBuffer;
//}