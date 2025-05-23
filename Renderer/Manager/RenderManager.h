#pragma once 
#include "../Renderer/Manager/LightingManager.h"
#include "../Renderer/Manager/TextureManager.h"
#include "../Renderer/Manager/ParticleManager.h"
#include "../Renderer/Manager/MeshRenderManager.h"
#include "../Renderer/Core/ShadowRenderer.h"
#include "../Renderer/Render/Canvas.h"
#include "../Renderer/Manager/FeatureManager.h"


class RenderManager {
public:
	RenderManager() = default;
	RenderManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, HWND hWnd, DefaultBufferCPUIterator mainCameraBufferLocation);

	~RenderManager() = default;

	RenderManager(const RenderManager& other) = default;
	RenderManager& operator=(const RenderManager& other) = default;

	RenderManager(RenderManager&& other) = default;
	RenderManager& operator=(RenderManager&& other) = default;
public:
	LightingManager&		GetLightingManager();
	TextureManager&			GetTextureManager();
	MaterialManager&		GetMaterialManager();
	ParticleManager&		GetParticleManager();
	MeshRenderManager&		GetMeshRenderManager();
	ShadowRenderer&			GetShadowRenderer();
	Canvas&					GetCanvas();
	FeatureManager&			GetFeatureManager();
	HWND					GetWindowHandle(); 

private:
	HWND mHwnd{ nullptr };
	LightingManager mLightingManager{};
	TextureManager mTextureManager{};
	MaterialManager mMaterialManager{};
	MeshRenderManager mMeshRenderManager{};
	ParticleManager mParticleManager{};
	ShadowRenderer mShadowRenderer{};
	Canvas mCanvas{}; 
	FeatureManager mFeatureManager{};
};