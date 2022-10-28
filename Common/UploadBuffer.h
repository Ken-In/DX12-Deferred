#pragma once

#include "d3dUtil.h"

template<typename T>
class UploadBuffer
{
public:
    UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) : mIsConstantBuffer(isConstantBuffer)
    {
        mElementByteSize = sizeof(T);

        // �����������Ĵ�С��ҪΪ256����. 
        // ������ΪӲ��ֻ�ܰ� n*256 ��ƫ�����Լ���������������
        // typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
        // UINT64 OffsetInBytes; // 256��������
        // UINT   SizeInBytes;   // 256��������
        // } D3D12_CONSTANT_BUFFER_VIEW_DESC;
        if(isConstantBuffer)
            mElementByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(T));//cb��Ҫ��256��������

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize*elementCount),
            // D3D12_RESOURCE_STATE_GENERIC_READ��������ȡ״̬λ���߼� OR ��ϡ�
            // �����ϴ����������ʼ״̬��
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mUploadBuffer)));

        //�õ�mUploadBuffer��cpuָ��
        ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));

        // ֻҪ�����޸ĸ���Դ,�Ͳ���Ҫȡ��ӳ��.
        // ��������Դ��GPUʹ���ڼ�, ���ɶ���Դ����д��! (������Ҫ������ͬ������)
    }

    //���ú���
    UploadBuffer(const UploadBuffer& rhs) = delete;
    UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

    // ������������ȡ��ӳ��
    ~UploadBuffer()
    {
        if(mUploadBuffer != nullptr)
            mUploadBuffer->Unmap(0, nullptr);

        mMappedData = nullptr;
    }

    ID3D12Resource* Resource()const
    {
        return mUploadBuffer.Get();
    }

    void CopyData(int elementIndex, const T& data)
    {
        memcpy(&mMappedData[elementIndex*mElementByteSize], &data, sizeof(T));
    }

private:
    ComPtr<ID3D12Resource> mUploadBuffer;
    BYTE* mMappedData = nullptr;

    UINT mElementByteSize = 0;
    bool mIsConstantBuffer = false;
};