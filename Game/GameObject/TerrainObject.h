#pragma once
#include "../Renderer/Resource/Mesh.h"
#include "../Renderer/Core/Shader.h"
#include "../Utility/Defines.h"
#include "../Game/Scene/Camera.h"
#include "../Renderer/Manager/RenderManager.h"


class TerrainSegment {
public:
	TerrainSegment(); 
	TerrainSegment(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList,const MeshData& data); 
	
	~TerrainSegment();

	TerrainSegment(const TerrainSegment&) = default;
	TerrainSegment& operator=(const TerrainSegment&) = default;

	TerrainSegment(TerrainSegment&&) = default;
	TerrainSegment& operator=(TerrainSegment&&) = default;
public:
	DirectX::BoundingBox& GetBB(); 

	Mesh* GetMesh() const; 

private:
	// Shader, Material 와 같은 Context 는 TerrainObject 에서 관리한다. 
	// 이는 Segment 가 하나의 지형 조각을 그리는 데 의미가 있음을 시사한다 
	std::shared_ptr<Mesh> mMesh{ nullptr }; 

	// Terrain Segment 의 BB 이다. 
	DirectX::BoundingBox mBoundingBox{};
};

// 여기서 내일 부터 
class TerrainObject {
public:
	TerrainObject() = default;
	TerrainObject(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, const std::filesystem::path& heightmap);
	
	~TerrainObject() = default;

	TerrainObject(const TerrainObject&) = default;
	TerrainObject& operator=(const TerrainObject&) = default;

	TerrainObject(TerrainObject&&) = default;
	TerrainObject& operator=(TerrainObject&&) = default;
public:
	void SetMaterial(MaterialIndex idx);

	void Update(Camera& camera, std::shared_ptr<RenderManager> mgr); 

private: 
	std::vector<TerrainSegment> mSegments{}; 

	std::shared_ptr<GraphicsShaderBase> mTerrainShader{ nullptr };

	ModelContext mModelContext{};
}; 