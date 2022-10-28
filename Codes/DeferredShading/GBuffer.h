#pragma once

#include "../../Common/d3dUtil.h"

using Microsoft::WRL::ComPtr;

enum class BufferType
{
	pos = 0,
	normal,
	color
};

class GBuffer
{
public:
	GBuffer(ID3D12Device* device, UINT width, UINT height);

	GBuffer(const GBuffer& rhs) = delete;
	GBuffer& operator=(const GBuffer& rhs) = delete;
	~GBuffer() = default;

	UINT Width()const { return mWidth; }
	UINT Height()const { return mHeight; }
	ID3D12Resource* Resource(BufferType t)
	{
		if (t == BufferType::pos)
			return mPositionMap.Get();
		else if (t == BufferType::normal)
			return mNormalMap.Get();
		else
			return mDiffuseColorMap.Get();
	}
	CD3DX12_GPU_DESCRIPTOR_HANDLE Srv(BufferType t)const
	{
		auto gpuSrv = mGpuSrv;
		return gpuSrv.Offset((int)t, mSrvDescSize);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE Rtv(BufferType t)const
	{
		auto cpuRtv = mCpuRtv;
		return cpuRtv.Offset((int)t, mRtvDescSize);
	}

	D3D12_VIEWPORT Viewport()const { return mViewport; }
	D3D12_RECT ScissorRect()const { return mScissorRect; }

	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv,
		UINT srvDescSize,
		UINT rtvDescSize);

	void OnResize(UINT newWidth, UINT newHeight);

	FLOAT* clear(BufferType type)
	{
		if (type == BufferType::color)
		{
			optClear.Format = mColorFormat;
			return optClear.Color;
		}
		optClear.Format = mNormalPosFormat;
		return optClear.Color;
	}

private:
	void BuildDescriptors();
	void BuildResource();

private:

	ID3D12Device* md3dDevice = nullptr;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mColorFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mNormalPosFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;

	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	CD3DX12_CLEAR_VALUE optClear = { mNormalPosFormat, ClearColor };

	FLOAT* c = optClear.Color;
	
	UINT mSrvDescSize;
	UINT mRtvDescSize;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mCpuRtv;

	ComPtr<ID3D12Resource> mPositionMap = nullptr;
	ComPtr<ID3D12Resource> mNormalMap = nullptr;
	ComPtr<ID3D12Resource> mDiffuseColorMap = nullptr;
};


GBuffer::GBuffer(ID3D12Device* device, UINT width, UINT height)
{
	md3dDevice = device;

	mWidth = width;
	mHeight = height;

	mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };

	BuildResource();
}

void GBuffer::BuildResource()
{
	assert(md3dDevice);
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mNormalPosFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// 法线
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&mNormalMap)));

	// 位置
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&mPositionMap)));

	// 颜色
	texDesc.Format = mColorFormat;
	optClear.Format = mColorFormat;
	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&mDiffuseColorMap)));
}

void GBuffer::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv, CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuRtv, UINT srvDescSize, UINT rtvDescSize)
{
	mCpuSrv = hCpuSrv;
	mGpuSrv = hGpuSrv;
	mCpuRtv = hCpuRtv;

	mSrvDescSize = srvDescSize;
	mRtvDescSize = rtvDescSize;

	BuildDescriptors();
}

void GBuffer::BuildDescriptors()
{
	auto cpuSrv = mCpuSrv;
	auto cpuRtv = mCpuRtv;

	// 创建srv
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = mNormalPosFormat;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	// 位置
	md3dDevice->CreateShaderResourceView(mPositionMap.Get(), &srvDesc, cpuSrv);
	// 法线
	cpuSrv.Offset(1, mSrvDescSize);
	md3dDevice->CreateShaderResourceView(mNormalMap.Get(), &srvDesc, cpuSrv);
	// 颜色
	srvDesc.Format = mColorFormat;
	cpuSrv.Offset(1, mSrvDescSize);
	md3dDevice->CreateShaderResourceView(mDiffuseColorMap.Get(), &srvDesc, cpuSrv);


	// 接下来创建rtv
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = mNormalPosFormat;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;
	// 位置
	md3dDevice->CreateRenderTargetView(mPositionMap.Get(), &rtvDesc, cpuRtv);
	// 法线
	cpuRtv.Offset(1, mRtvDescSize);
	md3dDevice->CreateRenderTargetView(mNormalMap.Get(), &rtvDesc, cpuRtv);
	// 颜色
	rtvDesc.Format = mColorFormat;
	cpuRtv.Offset(1, mRtvDescSize);
	md3dDevice->CreateRenderTargetView(mDiffuseColorMap.Get(), &rtvDesc, cpuRtv);
}

void GBuffer::OnResize(UINT newWidth, UINT newHeight)
{
	mNormalMap.Reset();
	mPositionMap.Reset();
	mDiffuseColorMap.Reset();

	//BuildDescriptors();

	if (mWidth != newWidth || mHeight != newHeight)
	{
		mWidth = newWidth;
		mHeight = newHeight;

		// We render to ambient map at half the resolution.
		mViewport.TopLeftX = 0.0f;
		mViewport.TopLeftY = 0.0f;
		mViewport.Width = mWidth;
		mViewport.Height = mHeight;
		mViewport.MinDepth = 0.0f;
		mViewport.MaxDepth = 1.0f;

		mScissorRect = { 0, 0, (int)mWidth, (int)mHeight};

		BuildResource();
	}
}