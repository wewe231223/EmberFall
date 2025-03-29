#pragma once 

#include <filesystem>
#include "../MeshLoader/Base/MeshData.h"
#include "../Utility/DirectXInclude.h"
#include <vector>

class TerrainLoader {
	static constexpr int PATCH_LENGTH = 4;
	static constexpr int PATCH_SCALE = 8;

	static constexpr int TILE_SCALE = 5; 
public:
	TerrainLoader() = default;
	~TerrainLoader() = default;
public:
	int GetLength() const;
	MeshData Load(const std::filesystem::path& path, bool patch);
private:
	void CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd);
private:
	std::vector<std::vector<float>> mHeight{};
	int mLength{};
};

class TerrainCollider {
public:
	TerrainCollider() = default;
	~TerrainCollider() = default;


	bool LoadFromFile(const std::filesystem::path& filePath);
	float GetHeight(float x, float z) const;
private:
	std::vector<SimpleMath::Vector3> mGlobalVertices;
	int  mGlobalWidth = 0;  
	int  mGlobalHeight = 0;   
	float mGridSpacing = 0.0f; 
	float mMinX = 0.0f;        
	float mMinZ = 0.0f;        
};