#include "pch.h"
#include "Renderer.h"
#include "../EditorInterface/Console/Console.h"
#include "../Utility/Enumerate.h"
#include "../Utility/Serializer.h"
#include "../Utility/Exceptions.h"
#include "../Utility/Crash.h"


struct CameraConstants {
	SimpleMath::Matrix view;
	SimpleMath::Matrix proj;
	SimpleMath::Matrix viewProj;
	SimpleMath::Vector3 cameraPosition;
};

Renderer::Renderer(HWND rendererWindowHandle)
	: mRendererWindow(rendererWindowHandle) {

	// 셰이더 매니저 테스트용.. 
	gShaderManager.Test();

	Renderer::InitFactory();
	Renderer::InitDevice();
	Renderer::InitCommandQueue();
	Renderer::InitFence();
	Renderer::InitSwapChain();
	Renderer::InitCommandList();
	Renderer::InitRenderTargets();
	Renderer::InitDepthStencilBuffer();

	Renderer::ResetCommandList();

	mMeshRenderManager = std::make_shared<MeshRenderManager>(mDevice);
	mTextureManager = std::make_shared<TextureManager>(mDevice, mCommandList);
	mMaterialManager = std::make_shared<MaterialManager>();
}

Renderer::~Renderer() {

}


std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>> Renderer::GetManagers() {
	return std::make_tuple(mMeshRenderManager, mTextureManager, mMaterialManager);
}

ComPtr<ID3D12Device> Renderer::GetDevice() {
	return mDevice;
}

ComPtr<ID3D12GraphicsCommandList> Renderer::GetCommandList() {
	return mCommandList;
}

void Renderer::UploadResource(){
	mMaterialManager->UploadMaterial(mDevice, mCommandList);

	CheckHR(mCommandList->Close());

	ID3D12CommandList* commandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(1, commandLists);
	Renderer::FlushCommandQueue();

	Console.Log("Renderer 초기화가 완료되었습니다.", LogType::Info);
}

void Renderer::Update() {
	// Update... 
}

void Renderer::PrepareRender() {
	Renderer::ResetCommandList();

	auto& currentBackBuffer = mRenderTargets[mRTIndex];

	CD3DX12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer.GetResource().Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET) };

	mCommandList->ResourceBarrier(1, &barrier);

	auto rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ mRTVHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(mRTIndex), rtvDescriptorSize };
	auto dsvHandle = mDSHeap->GetCPUDescriptorHandleForHeapStart();

	mCommandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::Black, 0, nullptr);
	mCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

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

	mCommandList->RSSetViewports(1, &viewport);
	mCommandList->RSSetScissorRects(1, &scissorRect);

	mMeshRenderManager->PrepareRender(mCommandList);
}

void Renderer::Render() {
	auto& currentBackBuffer = mRenderTargets[mRTIndex];

	mTextureManager->Bind(mCommandList);
	mMaterialManager->Bind(mCommandList);
	

	mMeshRenderManager->Render(mCommandList);

	mMeshRenderManager->Reset();

	// Rendering End


	currentBackBuffer.Transition(mCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	CheckHR(mCommandList->Close());
	ID3D12CommandList* commandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(1, commandLists);

	CheckHR(mSwapChain->Present(0, Config::ALLOW_TEARING ? DXGI_PRESENT_ALLOW_TEARING : NULL));

	mRTIndex = (mRTIndex + 1) % Config::BACKBUFFER_COUNT<UINT>;

	Renderer::FlushCommandQueue();
}

void Renderer::InitFactory() {
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

void Renderer::InitDevice() {
	auto hr = ::D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));

	if (FAILED(hr)) {
		ComPtr<IDXGIAdapter> warpAdapter{ nullptr };
		CheckHR(mFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		CheckHR(::D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice)));
	}
}

void Renderer::InitCommandQueue() {
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CheckHR(mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mCommandQueue)));
}

void Renderer::InitFence() {
	CheckHR(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
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
		mFactory->CreateSwapChainForHwnd(
			mCommandQueue.Get(), 
			mRendererWindow, 
			&desc, 
			nullptr, 
			nullptr, 
			mSwapChain.GetAddressOf()
		)
	);

}

void Renderer::InitCommandList() {
	CheckHR(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mAllocator)));
	CheckHR(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mAllocator.Get(), nullptr, IID_PPV_ARGS(&mCommandList)));

	mCommandList->SetName(L"Main Command List");
	mAllocator->SetName(L"Main Command Allocator");

	CheckHR(mCommandList->Close());
}

void Renderer::InitRenderTargets() {
	for (const auto [index, renderTarget] : Enumerate(mRenderTargets)) {
		CheckHR(mSwapChain->GetBuffer(static_cast<UINT>(index), IID_PPV_ARGS(renderTarget.GetAddressOf())));
	}

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = Config::BACKBUFFER_COUNT<UINT>;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CheckHR(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mRTVHeap.GetAddressOf())));

	auto rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE handle{ mRTVHeap->GetCPUDescriptorHandleForHeapStart() };

	for (auto& renderTarget : mRenderTargets) {
		mDevice->CreateRenderTargetView(renderTarget.GetResource().Get(), nullptr, handle);
		handle.ptr += rtvDescriptorSize;
	}

	//D3D12_DESCRIPTOR_HEAP_DESC gBufferDesc{};
	//gBufferDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	//gBufferDesc.NumDescriptors = 3;
	//gBufferDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	//CheckHR(mDevice->CreateDescriptorHeap(&gBufferDesc, IID_PPV_ARGS(mGBufferHeap.GetAddressOf())));

	//mGBuffers[0] = Texture(mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	//mGBuffers[1] = Texture(mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
	//mGBuffers[2] = Texture(mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	//D3D12_CPU_DESCRIPTOR_HANDLE gBufferHandle{ mGBufferHeap->GetCPUDescriptorHandleForHeapStart() };

	//mDevice->CreateRenderTargetView(mGBuffers[0].GetResource().Get(), nullptr, gBufferHandle);
	//gBufferHandle.ptr += rtvDescriptorSize;
	//mDevice->CreateRenderTargetView(mGBuffers[1].GetResource().Get(), nullptr, gBufferHandle);
	//gBufferHandle.ptr += rtvDescriptorSize;
	//mDevice->CreateRenderTargetView(mGBuffers[2].GetResource().Get(), nullptr, gBufferHandle);
}

void Renderer::InitDepthStencilBuffer() {
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CheckHR(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mDSHeap.GetAddressOf())));

	CD3DX12_CLEAR_VALUE clearValue{ DXGI_FORMAT_D24_UNORM_S8_UINT, 1.0f, 0 };
	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		Config::WINDOW_WIDTH<UINT64>, 
		Config::WINDOW_HEIGHT<UINT>, 
		1, 0, 1, 0, 
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	CheckHR(mDevice->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())
		)
	);

	mDevice->CreateDepthStencilView(mDepthStencilBuffer.GetResource().Get(), nullptr, mDSHeap->GetCPUDescriptorHandleForHeapStart());
}

void Renderer::ResetCommandList() {
	CheckHR(mAllocator->Reset());
	CheckHR(mCommandList->Reset(mAllocator.Get(), nullptr));
}

void Renderer::FlushCommandQueue() {
	mFenceValue++;
	CheckHR(mCommandQueue->Signal(mFence.Get(), mFenceValue));

	if (mFence->GetCompletedValue() < mFenceValue) {
		HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		CrashExp((eventHandle != nullptr), "Event can not be nullptr");

		mFence->SetEventOnCompletion(mFenceValue, eventHandle);
		::WaitForSingleObject(eventHandle, INFINITE);
		::CloseHandle(eventHandle);
	}
}
