#include "pch.h"
#include "RenderManager.h"

RenderManager::RenderManager(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, HWND hWnd, DefaultBufferCPUIterator mainCameraBufferLocation) {
	mLightingManager = LightingManager(device, commandList);
	mTextureManager = TextureManager(device, commandList);
	mMaterialManager = MaterialManager(device);
	mMeshRenderManager = MeshRenderManager(device);
	mParticleManager = ParticleManager(device, commandList);
	mShadowRenderer = ShadowRenderer(device, mainCameraBufferLocation);
	mCanvas = Canvas(device, commandList, hWnd);
	mHwnd = hWnd;
}

LightingManager& RenderManager::GetLightingManager() {
	return mLightingManager; 
}

TextureManager& RenderManager::GetTextureManager() {
	return mTextureManager; 
}

MaterialManager& RenderManager::GetMaterialManager() {
	return mMaterialManager; 
}

ParticleManager& RenderManager::GetParticleManager() {
	return mParticleManager; 
}

MeshRenderManager& RenderManager::GetMeshRenderManager() {
	return mMeshRenderManager; 
}

ShadowRenderer& RenderManager::GetShadowRenderer() {
	return mShadowRenderer; 
}

Canvas& RenderManager::GetCanvas() {
	return mCanvas; 
}

HWND RenderManager::GetWindowHandle() {
	return mHwnd; 
}
