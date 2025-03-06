#pragma once 
#include "../Renderer/Resource/DefaultBuffer.h"
#include "../MeshLoader/Base/MeshData.h"
#include <filesystem>

enum class EmbeddedMeshType : BYTE {
	Plane,
	Cube,
	Sphere,
	SkyBox, 
};


class Mesh {
public:
	Mesh(); 
	// Other Ctors... 
	Mesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, EmbeddedMeshType type, UINT size = 1);
	Mesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const MeshData& meshData);
	Mesh(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const std::filesystem::path& binPath);
	~Mesh();

	Mesh(const Mesh& other);
	Mesh& operator=(const Mesh& other);

	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;
public:
	void Bind(ComPtr<ID3D12GraphicsCommandList> commandList,const std::bitset<8>& attribute) const;
	bool GetIndexed() const;
	UINT GetUnitCount() const; 
private:
	std::vector<DefaultBuffer> mVertexBuffers{};
	DefaultBuffer mIndexBuffer{};

	std::array<D3D12_VERTEX_BUFFER_VIEW, 8> mVertexBufferViews{};
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView{};

	D3D12_PRIMITIVE_TOPOLOGY mPrimitiveTopology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };

	UINT mUnitCount{ 0 };

	std::bitset<8> mAttribute{};

	bool mIndexed{ false };
};







