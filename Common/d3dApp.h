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
    //赋值
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
 
    virtual bool Initialize();  //1.需重写的函数1, 做各种初始化,应先调用基类的初始化函数
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);  // 2. 需重写的函数2, 处理各种消息的函数, 如按键等, 重写的函数中未处理的消息转发给基类的MsgProc

protected:
    //更新以及draw call都是纯虚函数,必须重写
    virtual void CreateRtvAndDsvDescriptorHeaps();  //3. 需重写的函数3, 创建RTV和DSV, 默认够用. 在渲染多目标时会重写
	virtual void OnResize();    //4. 需重写函数4, 处理窗口大小改变的函数, 需要调整后台缓冲以及深度缓冲的大小, 投影矩阵等也要对应调整
	virtual void Update(const GameTimer& gt)=0;  //5. 需重写函数5, 绘制每一帧时调用, 用它来做各种更新(摄像机移动, 碰撞检测, 用户输入等)
    virtual void Draw(const GameTimer& gt)=0;  //6. 需重写函数6, 绘制每一帧时调用, 发出渲染命令

	// 处理鼠标输入的虚函数,可派生重写
    //鼠标所对应的时间, 而不必重写MsgProc, 参数1是鼠标状态(哪个键被按下), 参数2,3是鼠标在工作区的坐标
	virtual void OnMouseDown(WPARAM btnState, int x, int y){ }
	virtual void OnMouseUp(WPARAM btnState, int x, int y)  { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y){ }

protected:

	bool InitMainWindow();  //窗口初始化
	bool InitDirect3D();   //d3d初始化
	void CreateCommandObjects();  //创建命令队列, 命令列表, 命令分配器
    void CreateSwapChain();  //创建交换链

	void FlushCommandQueue();  //强制cpu等待gpu处理完命令队列

	ID3D12Resource* CurrentBackBuffer()const;  //后台缓存的Resource
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;  //返回后台缓冲的渲染目标(RTV)
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;  //返回深度模板视图(DSV)

	void CalculateFrameStats();  //计算每秒平均帧数,以及每帧消耗时间

    void LogAdapters();   //枚举所有显卡
    void LogAdapterOutputs(IDXGIAdapter* adapter);  //枚举显示器
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);  //枚举显示器支持的显示格式

protected:

    static D3DApp* mApp;

    HINSTANCE mhAppInst = nullptr; // 应用程序实例句柄
    HWND      mhMainWnd = nullptr; // 窗口句柄
	bool      mAppPaused = false;  // 是否在暂停
	bool      mMinimized = false;  // 是否最小化
	bool      mMaximized = false;  // 是否最大化
	bool      mResizing = false;   // 大小调整栏是否有拖拽
    bool      mFullscreenState = false;// 是否开启全屏

	// 若设为true则开启MSAA,默认为false
    bool      m4xMsaaState = false;    // 4X MSAA enabled
    UINT      m4xMsaaQuality = 0;      // 质量级别 of 4X MSAA

	// 记录时间的类,用于计算每帧时间以及总时间
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

	// 用户应在派生类中自定义这些值
	std::wstring mMainWndCaption = L"d3d App";
	D3D_DRIVER_TYPE md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int mClientWidth = 1600;
	int mClientHeight = 1200;
};

