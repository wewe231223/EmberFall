#pragma once 
#include "Resource/DefaultBuffer.h"

class PlaneMesh {
public:
	PlaneMesh(); 
	~PlaneMesh();
	// Other Ctors... 

	PlaneMesh(const PlaneMesh& other) = delete;
	PlaneMesh& operator=(const PlaneMesh& other) = delete;

	PlaneMesh(PlaneMesh&& other) noexcept;
	PlaneMesh& operator=(PlaneMesh&& other) noexcept;
public:
	void Bind(ComPtr<ID3D12GraphicsCommandList> commandList) const;
	UINT GetUnitCount() const; 
private:
	std::vector<DefaultBuffer> mVertexBuffers{};
	DefaultBuffer mIndexBuffer{};

	std::vector<D3D12_VERTEX_BUFFER_VIEW> mVertexBufferViews{};
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};

	D3D12_PRIMITIVE_TOPOLOGY mPrimitiveTopology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	bool mIndexed{ false };

	UINT mUnitCount{ 0 };
};