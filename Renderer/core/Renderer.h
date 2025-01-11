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
#include <memory>

class Renderer {
public:
	Renderer(HWND rendererWindowHandle);
	~Renderer();

	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	Renderer(Renderer&& other) = delete;
	Renderer& operator=(Renderer&& other) = delete;
public:
	void Update();
	void Render();
private:
	void InitFactory();
	void InitDevice();
	void InitCommandQueue();
	void InitFence();
	void InitSwapChain();
	void InitCommandList();
	void InitRenderTargets();
	void InitDepthStencilBuffer();

	void ResetCommandList();
	void FlushCommandQueue();
private:
	HWND mRendererWindow{ nullptr };

	struct DirectXImpl;
	std::unique_ptr<DirectXImpl> mDirectXImpl{ nullptr };
};