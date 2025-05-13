#pragma once 
#include "../External/Include/ImGui/imgui.h"
#include "../External/Include/ImGui/imgui_impl_dx12.h"
#include "../External/Include/ImGui/imgui_impl_win32.h"
#include "../External/Include/ImGui/imgui_internal.h"

class IMGUIRenderer {
public:
	IMGUIRenderer() = default;
	~IMGUIRenderer() = default;

	IMGUIRenderer(const IMGUIRenderer& other) = default;
	IMGUIRenderer& operator=(const IMGUIRenderer& other) = default;

	IMGUIRenderer(IMGUIRenderer&& other) = default;
	IMGUIRenderer& operator=(IMGUIRenderer&& other) = default;

public:
	void Initialize(HWND hWnd, ComPtr<ID3D12Device> device);

	void BeginRender();
	void EndRender(ComPtr<ID3D12GraphicsCommandList> commandList);
private:
	ComPtr<ID3D12DescriptorHeap> mDescriptorHeap{};
};