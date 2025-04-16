#include "pch.h"
#include "Canvas.h"

Canvas::Canvas(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList) {
	std::vector<DirectX::XMFLOAT2> vertices{
		{0.0f, 0.0f},  // Bottom-left
		{1.0f, 0.0f},  // Bottom-right
		{1.0f, 1.0f},  // Top-right
		{0.0f, 1.0f},  // Top-left
	};

	std::vector<UINT> indices{
		0, 1, 3,
		1, 2, 3
	};

	mVertexBuffer = DefaultBuffer(device, commandList, sizeof(DirectX::XMFLOAT2), 4, vertices.data());
	mVertexBufferView.BufferLocation = *mVertexBuffer.GPUBegin();
	mVertexBufferView.SizeInBytes = static_cast<UINT>(vertices.size() * sizeof(DirectX::XMFLOAT2));
	mVertexBufferView.StrideInBytes = sizeof(DirectX::XMFLOAT2);

	mIndexBuffer = DefaultBuffer(device, commandList, sizeof(UINT), 6, indices.data());
	mIndexBufferView.BufferLocation = *mIndexBuffer.GPUBegin();
	mIndexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(UINT));
	mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	mIndexBufferView.SizeInBytes = static_cast<UINT>(indices.size() * sizeof(UINT));

	mConstantBuffer = DefaultBuffer(device, sizeof(ModelContext2D), UI_ELEMENT_COUNT<size_t>);

	mShader = std::make_unique<UIShader>(); 
	mShader->CreateShader(device); 
}

void Canvas::AppendContext(const ModelContext2D& context) {

}

void Canvas::Render(ComPtr<ID3D12GraphicsCommandList> commandList, D3D12_GPU_DESCRIPTOR_HANDLE tex) {

}
