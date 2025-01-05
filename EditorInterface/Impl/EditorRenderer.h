#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EditorRenderer.h
// 2025.01.04 김승범   - Editor 창을 렌더링 할 Renderer 클래스 파일을 생성하였음.   
//                      
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <array>

class EditorRenderer {
public:
	EditorRenderer();
	~EditorRenderer();

public:
	void Initialize(HWND hWnd);
	void Render();
private:
	void InitFactory();
	void InitDevice();
	void InitCommandQueue();
	void InitFence();
	void InitSwapChain();
	void InitRenderTargets();
	void InitCommandList();
	void InitIMGUI();

	void ResetCommandList();
	void FlushCommandQueue();

private:
	HWND mEditorWindow{ nullptr };
	ComPtr<IDXGIFactory6> mFactory{ nullptr };

#ifdef _DEBUG
	ComPtr<ID3D12Debug6> mDebugController{ nullptr };
	ComPtr<IDXGIDebug1> mDXGIDebug{ nullptr };
#endif 
	ComPtr<ID3D12Device> mDevice{ nullptr };

	ComPtr<ID3D12CommandQueue> mCommandQueue{ nullptr };

	UINT64 mFenceValue{ 0 };
	ComPtr<ID3D12Fence> mFence{ nullptr };

	ComPtr<IDXGISwapChain1> mSwapChain{ nullptr };

	ComPtr<ID3D12CommandAllocator> mAllocator{ nullptr };
	ComPtr<ID3D12GraphicsCommandList> mCommandList{ nullptr };

	ComPtr<ID3D12DescriptorHeap> mRTVHeap{ nullptr };
	std::array<ComPtr<ID3D12Resource>, 3> mRenderTargets{ nullptr };

	ComPtr<ID3D12DescriptorHeap> mIMGUIHeap{ nullptr };
};
