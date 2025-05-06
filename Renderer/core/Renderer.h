#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Renderer.h
// 2025.01.10 김승범   - 메인 창에 그림을 그리는 역할을 하는 Renderer 클래스를 생성하고 DirectX12 초기화 과정을 구현함. 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef win32_lean_and_mean
#define win32_lean_and_mean
#endif
#include <Windows.h>
#include <array>
#include "../Config/Config.h"
#include "../Resource/Texture.h"
#include "../Renderer/core/Shader.h"
#include "../Renderer/Core/DefferedRenderer.h"
#include "../Renderer/Manager/RenderManager.h"
#include "../Resource/Mesh.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Renderer/Render/GrassRenderer.h"
#include "../Renderer/Core/BlurComputeProcessor.h"

enum class RenderFeature : BYTE {
	PARTICLE, 
	GRASS,
	BLOOM,
	END,
};

class Renderer {
public:
	Renderer(HWND rendererWindowHandle);
	~Renderer();

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	Renderer(Renderer&& other) = delete;
	Renderer& operator=(Renderer&& other) = delete;
public:
	std::shared_ptr<RenderManager> GetRenderManager(); 
	DefaultBufferCPUIterator GetMainCameraBuffer();

	ComPtr<ID3D12Device10> GetDevice();
	ComPtr<ID3D12GraphicsCommandList> GetCommandList();
	ComPtr<ID3D12GraphicsCommandList> GetLoadCommandList(); 

	void LoadTextures(); 

	void UploadResource();
	void ResetLoadCommandList(); 
	void ExecuteLoadCommandList();

	void SetFeatureEnabled(SceneFeatureType type);

	void Update();

	void Render();
	void ExecuteRender();

	void ToggleFullScreen();
	void SetFullScreenState(bool state); 
private:
	void InitFactory();
	
	ComPtr<IDXGIAdapter1> GetBestAdapter();
	bool CheckMeshShaderSupport();

	void InitDevice();
	void InitCommandQueue();
	void InitFence();
	void InitSwapChain();
	void InitCommandList();
	void InitRenderTargets();
	void InitDepthStencilBuffer();

	void InitStringRenderer(); 
	void InitFonts(); 

	void InitCameraBuffer(); 
	void InitTerrainBuffer();
	void InitParticleManager();
	void InitGrassRenderer(); 

	void InitCoreResources(); 
	void InitDefferedRenderer();
	void InitBlurComputeProcesser();

	void TransitionGBuffers(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	void ResetCommandList();
	void FlushCommandQueue();

	void SetWindowFullScreen(); 
	void SetWindowedMode();

private:
	HWND mRendererWindow{ nullptr };

	ComPtr<IDXGIFactory6> mFactory{ nullptr };

#ifdef _DEBUG
	ComPtr<ID3D12Debug6> mDebugController{ nullptr };
	ComPtr<IDXGIDebug1> mDXGIDebug{ nullptr };
#endif 
	ComPtr<ID3D12Device10> mDevice{ nullptr };

	ComPtr<ID3D12CommandQueue> mCommandQueue{ nullptr };

	UINT64 mFenceValue{ 0 };
	ComPtr<ID3D12Fence> mFence{ nullptr };

	ComPtr<IDXGISwapChain1> mSwapChain{ nullptr };

	bool mExecute{ false };
	ComPtr<ID3D12CommandAllocator> mLoadAllocator{ nullptr };
	ComPtr<ID3D12GraphicsCommandList> mLoadCommandList{ nullptr };

	ComPtr<ID3D12CommandAllocator> mAllocator{ nullptr };
	ComPtr<ID3D12GraphicsCommandList6> mCommandList{ nullptr };

	ComPtr<ID3D12DescriptorHeap> mRTVHeap{ nullptr };
	std::array<Texture, Config::BACKBUFFER_COUNT<UINT>> mRenderTargets{};
	UINT mRTIndex{ 0 };

	StringRenderer mStringRenderer{}; 
	/*
	1. diffuse 
	2. normal
	3. position 
	*/
	std::array<Texture, Config::GBUFFER_COUNT<UINT>> mGBuffers{};
	ComPtr<ID3D12DescriptorHeap> mGBufferHeap{ nullptr };

	DefferedRenderer mDefferedRenderer{};
	BlurComputeProcessor mBlurComputeProcessor{};

	ComPtr<ID3D12DescriptorHeap> mDSHeap{ nullptr };
	Texture mDepthStencilBuffer{};

	std::shared_ptr<RenderManager> mRenderManager{ nullptr };

	SceneFeatureType mFeatureEnabled{ false, false, false };
	GrassRenderer mGrassRenderer{};

	DefaultBuffer mTerrainHeaderBuffer{}; 
	DefaultBuffer mTerrainDataBuffer{}; 

	DefaultBuffer mMainCameraBuffer{};

	bool mIsFullScreen{ false };
};