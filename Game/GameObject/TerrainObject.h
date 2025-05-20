#pragma once
#include "../Renderer/Resource/Mesh.h"
#include "../Renderer/Core/Shader.h"
#include "../Utility/Defines.h"


class TerrainSegment {
public:
	TerrainSegment(); 
	TerrainSegment(MeshData& data, Mesh* mesh); 
	
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
	Mesh* mMesh{ nullptr }; 

	// Terrain Segment 의 BB 이다. 
	DirectX::BoundingBox mBoundingBox{};
};

// 여기서 내일 부터 
class TerrainObject {
public:

};