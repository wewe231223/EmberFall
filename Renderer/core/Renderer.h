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
#include "../Manager/MeshRenderManager.h"
#include "../Manager/TextureManager.h"
#include "../Resource/Mesh.h"
#include "../Renderer/Core/StringRenderer.h"
#include "../Renderer/Core/ShadowRenderer.h"
#include "../Renderer/Manager/ParticleManager.h"

class Renderer {
public:
	Renderer(HWND rendererWindowHandle);
	~Renderer();

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	Renderer(Renderer&& other) = delete;
	Renderer& operator=(Renderer&& other) = delete;
public:
	std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> GetManagers();
	DefaultBufferCPUIterator GetMainCameraBuffer();

	ComPtr<ID3D12Device> GetDevice();
	ComPtr<ID3D12GraphicsCommandList> GetCommandList();

	void UploadResource();

	void Update();

	void Render();
	void ExecuteRender();
private:
	void InitFactory();
	ComPtr<IDXGIAdapter1> GetBestAdapter();
	void InitDevice();
	void InitCommandQueue();
	void InitFence();
	void InitSwapChain();
	void InitCommandList();
	void InitRenderTargets();
	void InitDepthStencilBuffer();

	void InitStringRenderer(); 
	void InitFonts(); 

	void InitShadowRenderer();
	void InitParticleManager(); 

	void InitCoreResources(); 
	void InitDefferedRenderer();

	void TransitionGBuffers(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	void ResetCommandList();
	void FlushCommandQueue();

private:
	HWND mRendererWindow{ nullptr };

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
	std::array<Texture, Config::BACKBUFFER_COUNT<UINT>> mRenderTargets{};
	UINT mRTIndex{ 0 };

	StringRenderer mStringRenderer{}; 
	ShadowRenderer mShadowRenderer{};
	/*
	1. diffuse 
	2. normal
	3. position 
	*/
	std::array<Texture, 3> mGBuffers{};
	ComPtr<ID3D12DescriptorHeap> mGBufferHeap{ nullptr };

	DefferedRenderer mDefferedRenderer{};

	ComPtr<ID3D12DescriptorHeap> mDSHeap{ nullptr };
	Texture mDepthStencilBuffer{};

	std::shared_ptr<TextureManager> mTextureManager{};
	std::shared_ptr<MaterialManager> mMaterialManager{};
	std::shared_ptr<MeshRenderManager> mMeshRenderManager{};
	std::shared_ptr<ParticleManager> mParticleManager{};

	DefaultBuffer mMainCameraBuffer{};
};