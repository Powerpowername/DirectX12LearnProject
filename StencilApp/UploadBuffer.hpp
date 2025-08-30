#pragma once
#include "d3dUtil.h"  // �������ID3D12Device�Ķ���
using Microsoft::WRL::ComPtr;

template<typename T>
class UploadBuffer
{
private:
	ComPtr<ID3D12Resource> mUploadBuffer; // �ϴ�����Դ
	BYTE* mMappedData = nullptr;           // ӳ���CPU�ڴ�ָ��
	UINT mElementByteSize = 0;             // ÿ��Ԫ�ص��ֽڴ�С
	bool isConstantBuffer = false;         // �Ƿ�Ϊ����������



private:
	// ���㳣����������С�ľ�̬����
    static UINT CalcConstantBufferByteSize(UINT byteSize);
public:
    UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer);

	//���ÿ���
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

// ���ⶨ��ģ����Ĺ��캯�����ؼ��޸������<T>��
template<typename T>
inline UploadBuffer<T>::UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) : isConstantBuffer(isConstantBuffer)
{
    mElementByteSize = sizeof(T);
	// �����������Ĵ�С������256�ֽڵı���
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
	Map���� GPU ��Դ���������ϴ���������ӳ�䵽 CPU �ɷ��ʵ��ڴ��ַ������һ��ָ�루mMappedData������ CPU ����ֱ�Ӷ�д����Դ�����ݡ�
	Unmap����� CPU ����Դ��ӳ�䣬���� GPU ���� ��CPU ��ʱ���ٷ��ʸ���Դ������������ֹ GPU ��ȡ����Դ��
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
