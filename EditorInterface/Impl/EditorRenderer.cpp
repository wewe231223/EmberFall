#include "pch.h"
#include "EditorRenderer.h"
#include "../Utility/Exceptions.h"
#include "../Utility/Enumerate.h"
#include "../Config/Config.h"
#include "../External/Include/DirectXTK12/d3dx12.h"
#include "../External/Include/ImGui/imgui.h"
#include "../External/Include/ImGui/imgui_impl_dx12.h"
#include "../External/Include/ImGui/imgui_impl_win32.h"
#include "../External/Include/ImGui/imgui_internal.h"
#include "../Utility/Crash.h"

#include "../EditorInterface/Console/Console.h"

EditorRenderer::EditorRenderer() {

}

EditorRenderer::~EditorRenderer() {
	EditorRenderer::FlushCommandQueue();
	EditorRenderer::TerminateIMGUI();
}

void EditorRenderer::Initialize(HWND hWnd) {
	mEditorWindow = hWnd;

	EditorRenderer::InitFactory();
	EditorRenderer::InitDevice();
	EditorRenderer::InitCommandQueue();
	EditorRenderer::InitFence();
	EditorRenderer::InitSwapChain();
	EditorRenderer::InitRenderTargets();
	EditorRenderer::InitCommandList();
	EditorRenderer::InitIMGUI();

	Console.Log("정보 메세지 예시입니다.", LogType::Info);
	Console.Log("경고 메세지 예시입니다.", LogType::Warning);
	Console.Log("에러 메세지 예시입니다.", LogType::Error);

}

void EditorRenderer::Render() {
	EditorRenderer::ResetCommandList();

	auto currentBackBuffer = mRenderTargets[mRTIndex].Get();
	CD3DX12_RESOURCE_BARRIER barrier{ CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET) };

	mCommandList->ResourceBarrier(1, &barrier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ mRTVHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(mRTIndex), mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) };
	mCommandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::CornflowerBlue, 0, nullptr);

	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	RECT r{};
	::GetWindowRect(mEditorWindow, &r);

	//D3D12_VIEWPORT viewport{};
	//viewport.TopLeftX = 0;
	//viewport.TopLeftY = 0;
	//viewport.Width = static_cast<float>(r.right - r.left);
	//viewport.Height = static_cast<float>(r.bottom - r.top);
	//viewport.MinDepth = 0.f;
	//viewport.MaxDepth = 1.f;

	//D3D12_RECT scissorRect{};
	//scissorRect.left = 0;
	//scissorRect.top = 0;
	//scissorRect.right = r.right - r.left;
	//scissorRect.bottom = r.bottom - r.top;


	D3D12_VIEWPORT viewport{};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = Config::EDITOR_WINDOW_WIDTH<float>;
	viewport.Height = Config::EDITOR_WINDOW_HEIGHT<float>;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = Config::EDITOR_WINDOW_WIDTH<LONG>;
	scissorRect.bottom = Config::EDITOR_WINDOW_HEIGHT<LONG>;

	mCommandList->RSSetViewports(1, &viewport);
	mCommandList->RSSetScissorRects(1, &scissorRect);

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// IMGUI 그리기... 
	Console.Render();

	ImGui::Render();
	mCommandList->SetDescriptorHeaps(1, mIMGUIHeap.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mCommandList.Get());

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCommandList->ResourceBarrier(1, &barrier);

	CheckHR(mCommandList->Close());
	ID3D12CommandList* commandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(1, commandLists);

	CheckHR(mSwapChain->Present(0, Config::ALLOW_TEARING ? DXGI_PRESENT_ALLOW_TEARING : NULL));

	mRTIndex = (mRTIndex + 1) % Config::BACKBUFFER_COUNT<UINT>;

	EditorRenderer::FlushCommandQueue();
}

void EditorRenderer::InitFactory() {
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

void EditorRenderer::InitDevice() {
	auto hr = ::D3D12CreateDevice(nullptr, Config::DIRECTX_FEATURE_LEVEL, IID_PPV_ARGS(&mDevice));

	if (FAILED(hr)) {
		ComPtr<IDXGIAdapter> warpAdapter{ nullptr };
		CheckHR(mFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		CheckHR(::D3D12CreateDevice(warpAdapter.Get(), Config::DIRECTX_FEATURE_LEVEL, IID_PPV_ARGS(&mDevice)));
	}

}

void EditorRenderer::InitCommandQueue() {
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CheckHR(mDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&mCommandQueue)));
}

void EditorRenderer::InitFence() {
	CheckHR(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
}

void EditorRenderer::InitSwapChain() {
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

void EditorRenderer::InitRenderTargets() {
	for (const auto [index, renderTarget] : Enumerate(mRenderTargets)) {
		CheckHR(mSwapChain->GetBuffer(static_cast<UINT>(index), IID_PPV_ARGS(renderTarget.GetAddressOf())));
	}

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = Config::BACKBUFFER_COUNT<UINT>;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	CheckHR(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mRTVHeap)));

	auto rtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE handle{ mRTVHeap->GetCPUDescriptorHandleForHeapStart() };
	
	for (auto& renderTarget : mRenderTargets) {
		mDevice->CreateRenderTargetView(renderTarget.Get(), nullptr, handle);
		handle.ptr += rtvDescriptorSize;
	}
}

void EditorRenderer::InitCommandList() {
	CheckHR(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mAllocator)));
	CheckHR(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mAllocator.Get(), nullptr, IID_PPV_ARGS(&mCommandList)));

	mCommandList->Close();
}

void EditorRenderer::InitIMGUI() {
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CheckHR(mDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mIMGUIHeap)));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	if (Config::IMGUI_DARK_THEME) {
		ImGui::StyleColorsDark();
	}
	else {
		ImGui::StyleColorsLight();
	}

	auto imWin32 = ImGui_ImplWin32_Init(mEditorWindow);
	auto imDx12 = ImGui_ImplDX12_Init(
		mDevice.Get(), 
		Config::BACKBUFFER_COUNT<UINT>, 
		Config::RENDER_TARGET_FORMAT, 
		mIMGUIHeap.Get(), 
		mIMGUIHeap->GetCPUDescriptorHandleForHeapStart(), 
		mIMGUIHeap->GetGPUDescriptorHandleForHeapStart()
	);

	if (not imWin32 or not imDx12) {
		::abort();
	}

	if (Config::IMGUI_KOREAN) {
		ImFontConfig fontConfig{};
		fontConfig.OversampleH = 3;
		fontConfig.OversampleV = 1;
		fontConfig.PixelSnapH = true;
		fontConfig.MergeMode = false;
		io.Fonts->AddFontFromFileTTF(Config::IMGUI_KOREAN_FONT_PATH, 24.f, &fontConfig, io.Fonts->GetGlyphRangesKorean());
		io.Fonts->Build();
	}


}

void EditorRenderer::ResetCommandList() {
	CheckHR(mAllocator->Reset());
	CheckHR(mCommandList->Reset(mAllocator.Get(), nullptr));
}

void EditorRenderer::FlushCommandQueue() {
	++mFenceValue;
	CheckHR(mCommandQueue->Signal(mFence.Get(), mFenceValue));

	if (mFence->GetCompletedValue() < mFenceValue) {
		HANDLE eventHandle{ ::CreateEvent(nullptr, FALSE, FALSE, nullptr) };
		CrashExp(eventHandle != nullptr, "Event can not be nullptr");

		mFence->SetEventOnCompletion(mFenceValue, eventHandle);

		::WaitForSingleObject(eventHandle, INFINITE);
		::CloseHandle(eventHandle);
	}
}

void EditorRenderer::TerminateIMGUI() {
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	if (mIMGUIHeap) mIMGUIHeap.Reset();
}
