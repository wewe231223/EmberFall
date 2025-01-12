#include "pch.h"
#include "Renderer.h"
#include "../Utility/Exceptions.h"
#include "../Utility/Crash.h"
#include "../Utility/Enumerate.h"
#include "../Config/Config.h"
#include "../EditorInterface/Console/Console.h"

struct Renderer::DirectXImpl {
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
	std::array<ComPtr<ID3D12Resource>, Config::BACKBUFFER_COUNT<UINT>> mRenderTargets{ nullptr };
	UINT mRTIndex{ 0 };

	ComPtr<ID3D12DescriptorHeap> mDSHeap{ nullptr };
	ComPtr<ID3D12Resource> mDepthStencilBuffer{ nullptr };
};

Renderer::Renderer(HWND rendererWindowHandle) 
	: mRendererWindow(rendererWindowHandle) {
	mDirectXImpl = std::make_unique<DirectXImpl>();
	
	Renderer::InitFactory();
	Renderer::InitDevice();
	Renderer::InitCommandQueue();
	Renderer::InitFence();
	Renderer::InitSwapChain();
	Renderer::InitCommandList();
	Renderer::InitRenderTargets();
	Renderer::InitDepthStencilBuffer();

	Renderer::FlushCommandQueue();

	Console.Log("Renderer 초기화가 완료되었습니다.", LogType::Info);
}

Renderer::~Renderer() {
}

void Renderer::Update() {
	// Update... 
}

void Renderer::Render() {
	Renderer::ResetCommandList();

	auto& currentBackBuffer = mDirectXImpl->mRenderTargets[mDirectXImpl->mRTIndex];

	CD3DX12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET) };

	mDirectXImpl->mCommandList->ResourceBarrier(1, &barrier);

	auto rtvDescriptorSize = mDirectXImpl->mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ mDirectXImpl->mRTVHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(mDirectXImpl->mRTIndex), rtvDescriptorSize };
	auto dsvHandle = mDirectXImpl->mDSHeap->GetCPUDescriptorHandleForHeapStart();
	
	mDirectXImpl->mCommandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::Black, 0, nullptr);
	mDirectXImpl->mCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	mDirectXImpl->mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	D3D12_VIEWPORT viewport{};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Config::WINDOW_WIDTH<float>;
	viewport.Height = Config::WINDOW_HEIGHT<float>;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = Config::WINDOW_WIDTH<LONG>;
	scissorRect.bottom = Config::WINDOW_HEIGHT<LONG>;

	mDirectXImpl->mCommandList->RSSetViewports(1, &viewport);
	mDirectXImpl->mCommandList->RSSetScissorRects(1, &scissorRect);

	// Rendering... 



	barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mDirectXImpl->mCommandList->ResourceBarrier(1, &barrier);

	CheckHR(mDirectXImpl->mCommandList->Close());
	ID3D12CommandList* commandLists[] = { mDirectXImpl->mCommandList.Get() };
	mDirectXImpl->mCommandQueue->ExecuteCommandLists(1, commandLists);

	CheckHR(mDirectXImpl->mSwapChain->Present(0, Config::ALLOW_TEARING ? DXGI_PRESENT_ALLOW_TEARING : NULL));

	mDirectXImpl->mRTIndex = (mDirectXImpl->mRTIndex + 1) % Config::BACKBUFFER_COUNT<UINT>;

	Renderer::FlushCommandQueue();
}

void Renderer::InitFactory() {
	CheckHR(CreateDXGIFactory1(IID_PPV_ARGS(&mDirectXImpl->mFactory)));
#ifdef _DEBUG
	CheckHR(D3D12GetDebugInterface(IID_PPV_ARGS(&mDirectXImpl->mDebugController)));
	CheckHR(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&mDirectXImpl->mDXGIDebug)));

	mDirectXImpl->mDebugController->EnableDebugLayer();
	mDirectXImpl->mDebugController->SetEnableGPUBasedValidation(true);

	mDirectXImpl->mDXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
	mDirectXImpl->mDXGIDebug->EnableLeakTrackingForThread();
#endif
}

void Renderer::InitDevice() {
	auto hr = ::D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDirectXImpl->mDevice));

	if (FAILED(hr)) {
		ComPtr<IDXGIAdapter> warpAdapter{ nullptr };
		CheckHR(mDirectXImpl->mFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		CheckHR(::D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDirectXImpl->mDevice)));
	}
}

void Renderer::InitCommandQueue() {
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CheckHR(mDirectXImpl->mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mDirectXImpl->mCommandQueue)));
}

void Renderer::InitFence() {
	CheckHR(mDirectXImpl->mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mDirectXImpl->mFence)));
}

void Renderer::InitSwapChain() {
	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Width = 0;
	desc.Height = 0;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = FALSE;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 3;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = Config::ALLOW_TEARING ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;


	CheckHR(
		mDirectXImpl->mFactory->CreateSwapChainForHwnd(
			mDirectXImpl->mCommandQueue.Get(), 
			mRendererWindow, 
			&desc, 
			nullptr, 
			nullptr, 
			mDirectXImpl->mSwapChain.GetAddressOf()
		)
	);

}

void Renderer::InitCommandList() {
	CheckHR(mDirectXImpl->mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mDirectXImpl->mAllocator)));
	CheckHR(mDirectXImpl->mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mDirectXImpl->mAllocator.Get(), nullptr, IID_PPV_ARGS(&mDirectXImpl->mCommandList)));

	mDirectXImpl->mCommandList->Close();
}

void Renderer::InitRenderTargets() {
	for (const auto [index, renderTarget] : Enumerate(mDirectXImpl->mRenderTargets)) {
		CheckHR(mDirectXImpl->mSwapChain->GetBuffer(static_cast<UINT>(index), IID_PPV_ARGS(renderTarget.GetAddressOf())));
	}

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = Config::BACKBUFFER_COUNT<UINT>;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CheckHR(mDirectXImpl->mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mDirectXImpl->mRTVHeap.GetAddressOf())));

	auto rtvDescriptorSize = mDirectXImpl->mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE handle{ mDirectXImpl->mRTVHeap->GetCPUDescriptorHandleForHeapStart() };

	for (auto& renderTarget : mDirectXImpl->mRenderTargets) {
		mDirectXImpl->mDevice->CreateRenderTargetView(renderTarget.Get(), nullptr, handle);
		handle.ptr += rtvDescriptorSize;
	}
}

void Renderer::InitDepthStencilBuffer() {
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CheckHR(mDirectXImpl->mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mDirectXImpl->mDSHeap.GetAddressOf())));

	CD3DX12_CLEAR_VALUE clearValue{ DXGI_FORMAT_D32_FLOAT, 1.0f, 0 };
	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		Config::WINDOW_WIDTH<UINT64>, 
		Config::WINDOW_HEIGHT<UINT>, 
		1, 0, 1, 0, 
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	CheckHR(mDirectXImpl->mDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(mDirectXImpl->mDepthStencilBuffer.GetAddressOf())
		)
	);

	mDirectXImpl->mDevice->CreateDepthStencilView(mDirectXImpl->mDepthStencilBuffer.Get(), nullptr, mDirectXImpl->mDSHeap->GetCPUDescriptorHandleForHeapStart());
}

void Renderer::ResetCommandList() {
	CheckHR(mDirectXImpl->mAllocator->Reset());
	CheckHR(mDirectXImpl->mCommandList->Reset(mDirectXImpl->mAllocator.Get(), nullptr));
}

void Renderer::FlushCommandQueue() {
	mDirectXImpl->mFenceValue++;
	CheckHR(mDirectXImpl->mCommandQueue->Signal(mDirectXImpl->mFence.Get(), mDirectXImpl->mFenceValue));

	if (mDirectXImpl->mFence->GetCompletedValue() < mDirectXImpl->mFenceValue) {
		HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		CrashExp(eventHandle != nullptr, "Event can not be nullptr");

		mDirectXImpl->mFence->SetEventOnCompletion(mDirectXImpl->mFenceValue, eventHandle);
		::WaitForSingleObject(eventHandle, INFINITE);
		::CloseHandle(eventHandle);
	}
}
