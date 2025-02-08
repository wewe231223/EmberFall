#pragma once 
#include "Resource/DefaultBuffer.h"

class PlainMesh {
public:
	PlainMesh(); 
	~PlainMesh();
	// Other Ctors... 

	PlainMesh(const PlainMesh& other) = delete;
	PlainMesh& operator=(const PlainMesh& other) = delete;

	PlainMesh(PlainMesh&& other) noexcept;
	PlainMesh& operator=(PlainMesh&& other) noexcept;
public:
	void Bind(ComPtr<ID3D12GraphicsCommandList> commandList) const;
	bool GetIndexed() const;
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