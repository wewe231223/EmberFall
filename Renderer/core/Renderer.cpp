#include "pch.h"
#include "Renderer.h"
#include "../EditorInterface/Console/Console.h"
#include "../Utility/Enumerate.h"
#include "../Utility/Serializer.h"
#include "../Utility/Exceptions.h"
#include "../Utility/Crash.h"
#include "../Utility/IntervalTimer.h"

#define DIRECTWRITE

Renderer::Renderer(HWND rendererWindowHandle)
	: mRendererWindow(rendererWindowHandle) {

	gShaderManager.Test();

	// 기존 렌더러에 있는 초기화 작업은 이미지 로딩 이외에 모두 메인 쓰레드에서 해도 무방함. ( 충분히 빠른 시간 안에 완료됨 ) 
	Renderer::InitFactory();
	Renderer::InitDevice();
	Renderer::InitCommandQueue();
	Renderer::InitFence();
	Renderer::InitSwapChain();
	Renderer::InitCommandList();
	Renderer::InitRenderTargets();
	Renderer::InitDepthStencilBuffer();
	Renderer::InitStringRenderer();
	Renderer::InitBlurComputeProcesser();

	Renderer::ResetCommandList();

	
	Renderer::InitCoreResources(); 
	Renderer::InitCameraBuffer(); 
	Renderer::InitShadowRenderer();
	Renderer::InitDefferedRenderer();
	Renderer::InitTerrainBuffer();
	Renderer::InitParticleManager();
	Renderer::InitGrassRender();
	Renderer::InitCanvas(); 
}

Renderer::~Renderer() {
	Renderer::FlushCommandQueue(); 
}


std::tuple<std::shared_ptr<MeshRenderManager>, std::shared_ptr<TextureManager>, std::shared_ptr<MaterialManager>, std::shared_ptr<ParticleManager>, std::shared_ptr<Canvas>> Renderer::GetManagers() {
	return std::make_tuple(mMeshRenderManager, mTextureManager, mMaterialManager, mParticleManager, mCanvas);
}

DefaultBufferCPUIterator Renderer::GetMainCameraBuffer() {
	return mMainCameraBuffer.CPUBegin();
}

ComPtr<ID3D12Device10> Renderer::GetDevice() {
	return mDevice;
}

ComPtr<ID3D12GraphicsCommandList> Renderer::GetCommandList() {
	return mCommandList;
}

ComPtr<ID3D12GraphicsCommandList> Renderer::GetLoadCommandList() {
	return mLoadCommandList;
}

std::shared_ptr<ShadowRenderer> Renderer::GetShadowRenderer() {
	return mShadowRenderer;
}

void Renderer::LoadTextures() {
	mTextureManager->LoadAllImages(mDevice, mLoadCommandList); 

	MaterialConstants material{};

	material.mDiffuseTexture[0] = mTextureManager->GetTexture("Grass0");
	material.mDiffuseTexture[1] = mTextureManager->GetTexture("Grass1");
	material.mDiffuseTexture[2] = mTextureManager->GetTexture("Grass2");
	material.mDiffuseTexture[3] = mTextureManager->GetTexture("Grass3");

	mMaterialManager->CreateMaterial("GrassMaterial", material);

	mGrassRenderer.SetMaterial(mMaterialManager->GetMaterial("GrassMaterial"));

}

void Renderer::UploadResource(){ 
	mMaterialManager->UploadMaterial(mDevice, mCommandList);
	CheckHR(mCommandList->Close());

	ID3D12CommandList* commandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(1, commandLists);
	Renderer::FlushCommandQueue();

	Console.Log("Renderer 초기화가 완료되었습니다.", LogType::Info);
}

void Renderer::ResetLoadCommandList() {
	CheckHR(mLoadAllocator->Reset());
	CheckHR(mLoadCommandList->Reset(mLoadAllocator.Get(), nullptr));
}

void Renderer::ExecuteLoadCommandList() {
	mMaterialManager->UploadMaterial(mDevice, mLoadCommandList);
	mExecute = true; 
}

void Renderer::SetFeatureEnabled(std::tuple<bool, bool> type) {
	mFeatureEnabled = type;
}

void Renderer::Update() {
	// Update... 
}


void Renderer::Render() {
	Renderer::ResetCommandList();
	D3D12_VIEWPORT viewport{};



	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = ShadowRenderer::GetShadowMapSize<float>();
	viewport.Height = ShadowRenderer::GetShadowMapSize<float>();
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = ShadowRenderer::GetShadowMapSize<LONG>();
	scissorRect.bottom = ShadowRenderer::GetShadowMapSize<LONG>();

	mCommandList->RSSetViewports(1, &viewport);
	mCommandList->RSSetScissorRects(1, &scissorRect);

	mMainCameraBuffer.Upload(mCommandList);
	mShadowRenderer->Upload(mCommandList);
	mMeshRenderManager->PrepareRender(mCommandList);
	mTextureManager->Bind(mCommandList);

	mShadowRenderer->TransitionShadowMap(mCommandList, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mShadowRenderer->SetShadowDSVRTV(mDevice, mCommandList, 0);
	mMeshRenderManager->RenderShadowPass(0, mCommandList, mTextureManager->GetTextureHeapAddress(), mMaterialManager->GetMaterialBufferAddress(), *mShadowRenderer->GetShadowCameraBuffer(0));

	// mMeshRenderManager->RenderShadowPass(mCommandList, mMaterialManager->GetMaterialBufferAddress(), *mMainCameraBuffer.GPUBegin());

	mShadowRenderer->SetShadowDSVRTV(mDevice, mCommandList, 1);
	mMeshRenderManager->RenderShadowPass(1, mCommandList, mTextureManager->GetTextureHeapAddress(), mMaterialManager->GetMaterialBufferAddress(), *mShadowRenderer->GetShadowCameraBuffer(1));

	//mShadowRenderer->SetShadowDSVRTV(mDevice, mCommandList, 2);
	//mMeshRenderManager->RenderShadowPass(2, mCommandList, mTextureManager->GetTextureHeapAddress(), mMaterialManager->GetMaterialBufferAddress(), *mShadowRenderer->GetShadowCameraBuffer(2));


	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Config::WINDOW_WIDTH<float>;
	viewport.Height = Config::WINDOW_HEIGHT<float>;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = Config::WINDOW_WIDTH<LONG>;
	scissorRect.bottom = Config::WINDOW_HEIGHT<LONG>;

	mCommandList->RSSetViewports(1, &viewport);
	mCommandList->RSSetScissorRects(1, &scissorRect);

	// G-Buffer Pass 
	auto rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	auto dsvHandle = mDSHeap->GetCPUDescriptorHandleForHeapStart();
	mCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, 0, nullptr);

	Renderer::TransitionGBuffers(D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	CD3DX12_CPU_DESCRIPTOR_HANDLE gBufferHandle{ mGBufferHeap->GetCPUDescriptorHandleForHeapStart() };
	D3D12_CPU_DESCRIPTOR_HANDLE gBufferHandles[] = {
		gBufferHandle,
		gBufferHandle.Offset(1, rtvDescriptorSize),
		gBufferHandle.Offset(1, rtvDescriptorSize),
		gBufferHandle.Offset(1, rtvDescriptorSize)
	};
	mCommandList->ClearRenderTargetView(gBufferHandles[0], DirectX::Colors::Black, 0, nullptr);
	mCommandList->ClearRenderTargetView(gBufferHandles[1], DirectX::Colors::Black, 0, nullptr);
	mCommandList->ClearRenderTargetView(gBufferHandles[2], DirectX::Colors::Black, 0, nullptr);
	mCommandList->ClearRenderTargetView(gBufferHandles[3], DirectX::Colors::Black, 0, nullptr);

	mCommandList->OMSetRenderTargets(4, gBufferHandles, FALSE, &dsvHandle);

	auto& currentBackBuffer = mRenderTargets[mRTIndex];

	mTextureManager->Bind(mCommandList);
	mMeshRenderManager->RenderGPass(mCommandList, mTextureManager->GetTextureHeapAddress(), mMaterialManager->GetMaterialBufferAddress(), *mMainCameraBuffer.GPUBegin() );
	mMeshRenderManager->Reset();

	if (std::get<static_cast<size_t>(RenderFeature::GRASS)>(mFeatureEnabled)) {
		mGrassRenderer.Render(mCommandList, mMainCameraBuffer.GPUBegin(), mTextureManager->GetTextureHeapAddress(), mMaterialManager->GetMaterialBufferAddress());
	}

	if (std::get<static_cast<size_t>(RenderFeature::PARTICLE)>(mFeatureEnabled)) {
		mParticleManager->RenderSO(mCommandList);
		mParticleManager->RenderGS(mCommandList, mMainCameraBuffer.GPUBegin(), mTextureManager->GetTextureHeapAddress(), mMaterialManager->GetMaterialBufferAddress());
	}

	// Blurring Pass
	mGBuffers[3].Transition(mCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
	mBlurComputeProcessor.DispatchHorzBlur(mDevice, mCommandList, mGBuffers[3].GetResource());

	mGBuffers[3].Transition(mCommandList, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
	mBlurComputeProcessor.DispatchVertBlur(mDevice, mCommandList, mGBuffers[3].GetResource());
	mGBuffers[3].Transition(mCommandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Deffered Rendering Pass 
	mShadowRenderer->TransitionShadowMap(mCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	Renderer::TransitionGBuffers(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	currentBackBuffer.Transition(mCommandList, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ mRTVHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(mRTIndex), rtvDescriptorSize };
	mCommandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::Black, 0, nullptr);
	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	mDefferedRenderer.Render(mCommandList, mShadowRenderer->GetShadowCameraBuffer(0), mLightingManager->GetLightingBuffer());

	

	mTextureManager->Bind(mCommandList);

	mCanvas->Render(mCommandList, mTextureManager->GetTextureHeapAddress());
}

void Renderer::ExecuteRender() {
#ifndef DIRECTWRITE 
	currentBackBuffer.Transition(mCommandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	CheckHR(mCommandList->Close());
	ID3D12CommandList* commandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(1, commandLists);
	Renderer::FlushCommandQueue();

	CheckHR(mSwapChain->Present(0, Config::ALLOW_TEARING ? DXGI_PRESENT_ALLOW_TEARING : NULL));
	mRTIndex = (mRTIndex + 1) % Config::BACKBUFFER_COUNT<UINT>;
#else 
	CheckHR(mCommandList->Close());
	ID3D12CommandList* commandLists[2] = { mCommandList.Get(), nullptr };
	
	if (mExecute) {
		CheckHR(mLoadCommandList->Close());
		commandLists[1] = mLoadCommandList.Get();
	}

	mCommandQueue->ExecuteCommandLists(mExecute ? 2 : 1, commandLists);
	mExecute = false;

	Renderer::FlushCommandQueue();

	mStringRenderer.Render();
	mParticleManager->PostRender();
	mParticleManager->ValidateParticle();

	CheckHR(mSwapChain->Present(0, Config::ALLOW_TEARING ? DXGI_PRESENT_ALLOW_TEARING : NULL));
	mRTIndex = (mRTIndex + 1) % Config::BACKBUFFER_COUNT<UINT>;
#endif
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

ComPtr<IDXGIAdapter1> Renderer::GetBestAdapter() {
	OutputDebugString(L"\n\n====================Selecting Adapter====================\n\n");

	ComPtr<IDXGIAdapter1> bestAdapter;
	size_t maxVRAM = 0;

	std::wstring message{};

	for (UINT i = 0; ; i++) {
		ComPtr<IDXGIAdapter1> adapter;
		if (mFactory->EnumAdapters1(i, &adapter) == DXGI_ERROR_NOT_FOUND) {
			break;
		}

		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		message = std::format(L"Adapter{:^3} : {} | VRAM: {} MB\n", i, desc.Description, desc.DedicatedVideoMemory / (1024 * 1024));
		OutputDebugString(message.c_str());

		if (desc.DedicatedVideoMemory > maxVRAM) {
			maxVRAM = desc.DedicatedVideoMemory;
			bestAdapter = adapter;
		}
	}

	if (bestAdapter) {
		DXGI_ADAPTER_DESC1 bestDesc;
		bestAdapter->GetDesc1(&bestDesc);

		message = std::format(L"Selected Adapter: {} | VRAM: {} MB\n", bestDesc.Description, bestDesc.DedicatedVideoMemory / (1024 * 1024));
		OutputDebugString(message.c_str());
	}
	else {
		OutputDebugStringA("No suitable GPU found.\n");
	}
	OutputDebugString(L"\n=========================================================\n\n");

	return bestAdapter;
}

bool Renderer::CheckMeshShaderSupport() {
	// Shader Model 6.5 이상 확인
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {};
	shaderModel.HighestShaderModel = D3D_SHADER_MODEL_6_5;

	if (FAILED(mDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) ||
		shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_5)
	{
		MessageBoxW(nullptr, L"Shader Model 6.5 미지원", L"Mesh Shader 확인", MB_ICONERROR | MB_OK);
		return false;
	}

	// Mesh Shader Tier 지원 여부 확인
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
	if (FAILED(mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7))) ||
		options7.MeshShaderTier == D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)
	{
		MessageBoxW(nullptr, L"Mesh Shader Tier 미지원", L"Mesh Shader 확인", MB_ICONERROR | MB_OK);
		return false;
	}

	MessageBoxW(nullptr, L"Mesh Shader 지원됨!", L"Mesh Shader 확인", MB_ICONINFORMATION | MB_OK);
	return true;
}

void Renderer::InitDevice() {
	ComPtr<IDXGIAdapter1> adapter = GetBestAdapter();

	CrashExp(adapter != nullptr, "No suitable GPU found.");


	ComPtr<ID3D12Device> base{};
	auto hr = ::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&base));



	if (FAILED(hr)) {
		ComPtr<IDXGIAdapter> warpAdapter{ nullptr };
		CheckHR(mFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		CheckHR(::D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&base)));
	}


	CheckHR(base.As(&mDevice));

	// 메시 셰이더 지원됨 
	// Renderer::CheckMeshShaderSupport(); 
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

	CheckHR(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mLoadAllocator)));
	CheckHR(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mLoadAllocator.Get(), nullptr, IID_PPV_ARGS(&mLoadCommandList)));

	CheckHR(mLoadCommandList->Close());

	ComPtr<ID3D12GraphicsCommandList> base{}; 

	CheckHR(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mAllocator)));
	CheckHR(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mAllocator.Get(), nullptr, IID_PPV_ARGS(&base)));


	CheckHR(base.As(&mCommandList));


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

	D3D12_DESCRIPTOR_HEAP_DESC gBufferDesc{};
	gBufferDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	gBufferDesc.NumDescriptors = Config::GBUFFER_COUNT<UINT>;
	gBufferDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CheckHR(mDevice->CreateDescriptorHeap(&gBufferDesc, IID_PPV_ARGS(mGBufferHeap.GetAddressOf())));

	mGBuffers[0] = Texture(mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	mGBuffers[1] = Texture(mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	mGBuffers[2] = Texture(mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
	mGBuffers[3] = Texture(mDevice, DXGI_FORMAT_R32G32B32A32_FLOAT, Config::WINDOW_WIDTH<UINT64>, Config::WINDOW_HEIGHT<UINT>, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

	D3D12_CPU_DESCRIPTOR_HANDLE gBufferHandle{ mGBufferHeap->GetCPUDescriptorHandleForHeapStart() };

	mDevice->CreateRenderTargetView(mGBuffers[0].GetResource().Get(), nullptr, gBufferHandle);
	gBufferHandle.ptr += rtvDescriptorSize;
	mDevice->CreateRenderTargetView(mGBuffers[1].GetResource().Get(), nullptr, gBufferHandle);
	gBufferHandle.ptr += rtvDescriptorSize;
	mDevice->CreateRenderTargetView(mGBuffers[2].GetResource().Get(), nullptr, gBufferHandle);
	gBufferHandle.ptr += rtvDescriptorSize;
	mDevice->CreateRenderTargetView(mGBuffers[3].GetResource().Get(), nullptr, gBufferHandle);

}

void Renderer::InitDepthStencilBuffer() {
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CheckHR(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(mDSHeap.GetAddressOf())));

	CD3DX12_CLEAR_VALUE clearValue{ DXGI_FORMAT_D24_UNORM_S8_UINT, 0.0f, 0 };
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

void Renderer::InitStringRenderer() {
	mStringRenderer.Initialize(mDevice, mCommandQueue, mRenderTargets);
	Renderer::InitFonts(); 
	TextBlockManager::GetInstance().Initialize(&mStringRenderer);
}

void Renderer::InitFonts() {
	mStringRenderer.LoadExternalFont("NotoSansKR", "Resources/Font/NotoSansKR-Regular-Hestia.otf", L"Noto Sans KR", L"ko-kr");
}

void Renderer::InitCameraBuffer() {
	mMainCameraBuffer = DefaultBuffer(mDevice, sizeof(CameraConstants), 1, true);
}

void Renderer::InitShadowRenderer() {
	mShadowRenderer = std::make_shared<ShadowRenderer>(mDevice, mMainCameraBuffer.CPUBegin());
}

void Renderer::InitParticleManager() {
	mParticleManager = std::make_shared<ParticleManager>(mDevice, mCommandList);
	mParticleManager->SetTerrain(mTerrainHeaderBuffer.GPUBegin(), mTerrainDataBuffer.GPUBegin());
}

void Renderer::InitGrassRender() {
	mGrassRenderer = GrassRenderer(mDevice, mCommandList, mTerrainHeaderBuffer.GPUBegin(), mTerrainDataBuffer.GPUBegin());
}

void Renderer::InitCanvas() {
	mCanvas = std::make_unique<Canvas>(mDevice, mCommandList);
}

void Renderer::InitTerrainBuffer() {
	TerrainHeader header{};
	std::vector<SimpleMath::Vector3> vertices{};

	std::ifstream file("Resources/Binarys/Terrain/TerrainBaked.bin", std::ios::binary);

	file.read(reinterpret_cast<char*>(&header), sizeof(TerrainHeader));


	vertices.resize(header.globalWidth * header.globalHeight);
	file.read(reinterpret_cast<char*>(vertices.data()), vertices.size() * sizeof(SimpleMath::Vector3));


	mTerrainHeaderBuffer = DefaultBuffer(mDevice, mCommandList, sizeof(TerrainHeader), 1, &header, true);
	mTerrainDataBuffer = DefaultBuffer(mDevice, mCommandList, sizeof(SimpleMath::Vector3), header.globalWidth * header.globalHeight,  vertices.data() );
}

void Renderer::InitCoreResources() {
	mMeshRenderManager = std::make_shared<MeshRenderManager>(mDevice);
	mTextureManager = std::make_shared<TextureManager>(mDevice, mCommandList);
	mMaterialManager = std::make_shared<MaterialManager>();
	mLightingManager = std::make_shared<LightingManager>(mDevice, mCommandList);

}

void Renderer::InitDefferedRenderer() {
	mDefferedRenderer = DefferedRenderer(mDevice, mCommandList);

	mDefferedRenderer.RegisterGBufferTexture(mDevice, mGBuffers);
	mDefferedRenderer.RegisterShadowMap(mDevice, mShadowRenderer->GetShadowMapArray());

}

void Renderer::InitBlurComputeProcesser() {
	mBlurComputeProcessor = BlurComputeProcessor(mDevice);

}

void Renderer::TransitionGBuffers(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) {
	D3D12_RESOURCE_BARRIER barriers[]{
		CD3DX12_RESOURCE_BARRIER::Transition(mGBuffers[0].GetResource().Get(), beforeState, afterState),
		CD3DX12_RESOURCE_BARRIER::Transition(mGBuffers[1].GetResource().Get(), beforeState, afterState),
		CD3DX12_RESOURCE_BARRIER::Transition(mGBuffers[2].GetResource().Get(), beforeState, afterState),
		CD3DX12_RESOURCE_BARRIER::Transition(mGBuffers[3].GetResource().Get(), beforeState, afterState)
	};

	mCommandList->ResourceBarrier(4, barriers);
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
