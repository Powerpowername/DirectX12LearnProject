#pragma once
#include "d3dUtil.h"  // 假设包含ID3D12Device的定义
using Microsoft::WRL::ComPtr;

template<typename T>
class UploadBuffer
{
private:
	ComPtr<ID3D12Resource> mUploadBuffer; // 上传堆资源
	BYTE* mMappedData = nullptr;           // 映射的CPU内存指针
	UINT mElementByteSize = 0;             // 每个元素的字节大小
	bool isConstantBuffer = false;         // 是否为常量缓冲区



private:
	// 计算常量缓冲区大小的静态函数
    static UINT CalcConstantBufferByteSize(UINT byteSize);
public:
    UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer);

	//禁用拷贝
	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator= (const UploadBuffer& rhs) = delete;

	~UploadBuffer();

	ID3D12Resource* Resource() const;
	void CopyData(int elementIndex, const T& data);
};

template<typename T>
inline UINT UploadBuffer<T>::CalcConstantBufferByteSize(UINT byteSize)
{
    return (byteSize + 255) & ~255;
}

// 类外定义模板类的构造函数（关键修复：添加<T>）
template<typename T>
inline UploadBuffer<T>::UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) : isConstantBuffer(isConstantBuffer)
{
    mElementByteSize = sizeof(T);
	// 常量缓冲区的大小必须是256字节的倍数
    if (isConstantBuffer)
    {
		mElementByteSize = CalcConstantBufferByteSize(sizeof(T));
    }
    ThrowIfFailed(device->CreateCommittedResource(
		&CD3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3D12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(mUploadBuffer.GetAddressOf())));
	/*
	Map：将 GPU 资源（这里是上传缓冲区）映射到 CPU 可访问的内存地址，返回一个指针（mMappedData），让 CPU 可以直接读写该资源的内容。
	Unmap：解除 CPU 对资源的映射，告诉 GPU 驱动 “CPU 暂时不再访问该资源”，但不会阻止 GPU 读取该资源。
	*/
	ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));
}

template<typename T>
inline UploadBuffer<T>::~UploadBuffer()
{
	if (mUploadBuffer != nullptr)
	{
		mUploadBuffer->Unmap(0, nullptr);
	}
	mMappedData = nullptr;
}

template<typename T>
inline ID3D12Resource* UploadBuffer<T>::Resource() const
{
	return mUploadBuffer.Get();
}

template<typename T>
inline void UploadBuffer<T>::CopyData(int elementIndex, const T& data)
{
	memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
}
