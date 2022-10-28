//***************************************************************************************
// d3dApp.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "d3dUtil.h"
#include "GameTimer.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class D3DApp
{
protected:

    D3DApp(HINSTANCE hInstance);
    //��ֵ
    D3DApp(const D3DApp& rhs) = delete;
    D3DApp& operator=(const D3DApp& rhs) = delete;
    virtual ~D3DApp();

public:

    static D3DApp* GetApp();
    
	HINSTANCE AppInst()const;
	HWND      MainWnd()const;
	float     AspectRatio()const;

    bool Get4xMsaaState()const;
    void Set4xMsaaState(bool value);

	int Run();
 
    virtual bool Initialize();  //1.����д�ĺ���1, �����ֳ�ʼ��,Ӧ�ȵ��û���ĳ�ʼ������
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);  // 2. ����д�ĺ���2, ���������Ϣ�ĺ���, �簴����, ��д�ĺ�����δ�������Ϣת���������MsgProc

protected:
    //�����Լ�draw call���Ǵ��麯��,������д
    virtual void CreateRtvAndDsvDescriptorHeaps();  //3. ����д�ĺ���3, ����RTV��DSV, Ĭ�Ϲ���. ����Ⱦ��Ŀ��ʱ����д
	virtual void OnResize();    //4. ����д����4, �����ڴ�С�ı�ĺ���, ��Ҫ������̨�����Լ���Ȼ���Ĵ�С, ͶӰ�����ҲҪ��Ӧ����
	virtual void Update(const GameTimer& gt)=0;  //5. ����д����5, ����ÿһ֡ʱ����, �����������ָ���(������ƶ�, ��ײ���, �û������)
    virtual void Draw(const GameTimer& gt)=0;  //6. ����д����6, ����ÿһ֡ʱ����, ������Ⱦ����

	// �������������麯��,��������д
    //�������Ӧ��ʱ��, ��������дMsgProc, ����1�����״̬(�ĸ���������), ����2,3������ڹ�����������
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:

	bool InitMainWindow();  //���ڳ�ʼ��
	bool InitDirect3D();   //d3d��ʼ��
	void CreateCommandObjects();  //�����������, �����б�, ���������
    void CreateSwapChain();  //����������

	void FlushCommandQueue();  //ǿ��cpu�ȴ�gpu�������������

	ID3D12Resource* CurrentBackBuffer()const;  //��̨�����Resource
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;  //���غ�̨�������ȾĿ��(RTV)
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;  //�������ģ����ͼ(DSV)

	void CalculateFrameStats();  //����ÿ��ƽ��֡��,�Լ�ÿ֡����ʱ��

    void LogAdapters();   //ö�������Կ�
    void LogAdapterOutputs(IDXGIAdapter* adapter);  //ö����ʾ��
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);  //ö����ʾ��֧�ֵ���ʾ��ʽ

protected:

    static D3DApp* mApp;

    HINSTANCE mhAppInst = nullptr; // Ӧ�ó���ʵ�����
    HWND      mhMainWnd = nullptr; // ���ھ��
	bool      mAppPaused = false;  // �Ƿ�����ͣ
	bool      mMinimized = false;  // �Ƿ���С��
	bool      mMaximized = false;  // �Ƿ����
	bool      mResizing = false;   // ��С�������Ƿ�����ק
    bool      mFullscreenState = false;// �Ƿ���ȫ��

	// ����Ϊtrue����MSAA,Ĭ��Ϊfalse
    bool      m4xMsaaState = false;    // 4X MSAA enabled
    UINT      m4xMsaaQuality = 0;      // �������� of 4X MSAA

	// ��¼ʱ�����,���ڼ���ÿ֡ʱ���Լ���ʱ��
	GameTimer mTimer;
	
    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;
	
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList;

	static const int SwapChainBufferCount = 2;
	int mCurrBackBuffer = 0;
    Microsoft::WRL::ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
    Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mRtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDsvHeap;

    D3D12_VIEWPORT mScreenViewport; 
    D3D12_RECT mScissorRect;

	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	// �û�Ӧ�����������Զ�����Щֵ
	std::wstring mMainWndCaption = L"d3d App";
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int mClientWidth = 1600;
	int mClientHeight = 1200;
};

