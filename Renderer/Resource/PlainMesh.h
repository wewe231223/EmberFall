#pragma once 
#include "../Renderer/Resource/DefaultBuffer.h"

enum class EmbeddedMeshType : BYTE {
	Plane,
	Cube,
	Sphere,
};


class PlainMesh {
public:
	PlainMesh(); 
	// Other Ctors... 
	PlainMesh(ComPtr<ID3D12Device> device, EmbeddedMeshType type, UINT size = 1);
	~PlainMesh();

	PlainMesh(const PlainMesh& other);
	PlainMesh& operator=(const PlainMesh& other);

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