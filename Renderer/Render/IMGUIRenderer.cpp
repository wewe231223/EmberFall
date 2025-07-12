#include "pch.h"
#include "IMGUIRenderer.h"
#include "../Utility/Exceptions.h"
#include "../Config/Config.h"


void IMGUIRenderer::Initialize(HWND hWnd, ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CheckHR(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mDescriptorHeap)));

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

	auto imWin32 = ImGui_ImplWin32_Init(hWnd);
	auto imDx12 = ImGui_ImplDX12_Init(
		device.Get(),
		Config::BACKBUFFER_COUNT<UINT>,
		Config::RENDER_TARGET_FORMAT,
		mDescriptorHeap.Get(),
		mDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		mDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
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

void IMGUIRenderer::BeginRender() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void IMGUIRenderer::EndRender(ComPtr<ID3D12GraphicsCommandList> commandList) {
	ImGui::Render();
	commandList->SetDescriptorHeaps(1, mDescriptorHeap.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
}
