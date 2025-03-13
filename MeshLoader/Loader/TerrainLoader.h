#pragma once 

#include <filesystem>
#include "../MeshLoader/Base/MeshData.h"
#include "../Utility/DirectXInclude.h"
#include <vector>

class TerrainLoader {
	static constexpr int PATCH_LENGTH = 4;
	static constexpr int PATCH_SCALE = 8;
public:
	TerrainLoader() = default;
	~TerrainLoader() = default;
public:
	MeshData Load(const std::filesystem::path& path, bool patch);
private:
	void CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd);
private:
	std::vector<std::vector<float>> mHeight{};
	int mLength{};
};


struct TessellatedPatchHeader {
	int gridWidth;    
	int gridHeight;   
	float gridSpacing; 
	float minX;       // 패치의 좌측 하단 x 
	float minZ;       // 패치의 좌측 하단 z
};

struct TessellatedPatch {
	TessellatedPatchHeader header;
	std::vector<SimpleMath::Vector3> vertices; 
};


class TerrainCollider {
public:
	TerrainCollider() = default;
	~TerrainCollider() = default;
public:
	bool LoadFromFile(const std::filesystem::path& filePath);
	float GetHeight(float x, float z) const;

private:
	std::vector<TessellatedPatch> mPatches;
};
