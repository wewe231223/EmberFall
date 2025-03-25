#include "pch.h"
#include "ShadowRenderer.h"
#include "../Utility/Exceptions.h" 

ShadowRenderer::ShadowRenderer(ComPtr<ID3D12Device> device) {
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	CheckHR(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&mShadowDSVHeap)));

	mShadowMap = Texture(device, DXGI_FORMAT_D24_UNORM_S8_UINT, 1000, 1000, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	device->CreateDepthStencilView(mShadowMap.GetResource().Get(), nullptr, mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart());
}

void ShadowRenderer::SetShadowDSV(ComPtr<ID3D12GraphicsCommandList> commandList) {
	mShadowMap.Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	auto dsv = mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(0, nullptr, FALSE, &dsv);
	commandList->ClearDepthStencilView(mShadowDSVHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void ShadowRenderer::TransitionShadowMap(ComPtr<ID3D12GraphicsCommandList> commandList) {
	mShadowMap.Transition(commandList, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
