#include "pch.h"
#include "EditorRenderer.h"

#include "../Utility/Exceptions.h"
#include "../Utility/Enumerate.h"
#include "../Config/Config.h"

#include <ranges>

EditorRenderer::EditorRenderer()
{
}

EditorRenderer::~EditorRenderer()
{
}

void EditorRenderer::Initialize(HWND hWnd)
{
	mEditorWindow = hWnd;

	EditorRenderer::InitFactory();
	EditorRenderer::InitDevice();
	EditorRenderer::InitCommandQueue();
	EditorRenderer::InitFence();
	EditorRenderer::InitSwapChain();
}

void EditorRenderer::Render()
{
}

void EditorRenderer::InitFactory()
{
	CheckHR(CreateDXGIFactory1(IID_PPV_ARGS(&mFactory)));
#ifdef _DEBUG
	CheckHR(D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugController)));
	CheckHR(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&mDXGIDebug)));

	mDebugController->EnableDebugLayer();
	mDebugController->SetEnableGPUBasedValidation(true);
	
	mDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	mDXGIDebug->EnableLeakTrackingForThread();
#endif 
}

void EditorRenderer::InitDevice()
{
	auto hr = ::D3D12CreateDevice(nullptr, Config::DIRECTX_FEATURE_LEVEL, IID_PPV_ARGS(&mDevice));

	if (FAILED(hr)) {
		ComPtr<IDXGIAdapter> warpAdapter{ nullptr };
		CheckHR(mFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		CheckHR(::D3D12CreateDevice(warpAdapter.Get(), Config::DIRECTX_FEATURE_LEVEL, IID_PPV_ARGS(&mDevice)));
	}

}

void EditorRenderer::InitCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CheckHR(mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mCommandQueue)));
}

void EditorRenderer::InitFence()
{
	CheckHR(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
}

void EditorRenderer::InitSwapChain()
{
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC1 sd{};
	sd.BufferCount = Config::BACKBUFFER_COUNT<UINT>;
	sd.Width = Config::EDITOR_WINDOW_WIDTH<UINT>;
	sd.Height = Config::EDITOR_WINDOW_HEIGHT<UINT>;
	sd.Format = Config::RENDER_TARGET_FORMAT; // 버퍼 형식
	sd.Stereo = FALSE; // 스테레오 모드 비활성화
	sd.SampleDesc.Count = 1; // 멀티샘플링 비활성화
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 버퍼 사용 방법
	sd.Scaling = DXGI_SCALING_STRETCH; // 스케일링 모드
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 효과
	sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // 알파 모드
	sd.Flags = Config::ALLOW_TEARING ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : NULL;

	CheckHR(
		mFactory->CreateSwapChainForHwnd(
			mCommandQueue.Get(),
			mEditorWindow,
			&sd,
			nullptr,
			nullptr,
			mSwapChain.GetAddressOf()
		)
	);
}

void EditorRenderer::InitRenderTargets()
{
	
}
