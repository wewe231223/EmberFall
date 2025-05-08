#pragma once 

#include <filesystem>
#include "../MeshLoader/Base/MeshData.h"
#include "../Utility/DirectXInclude.h"
#include "../Utility/Defines.h"
#include <vector>

class TerrainLoader {
	static constexpr int PATCH_LENGTH = 4;
	static constexpr int PATCH_SCALE = 8;

	static constexpr int TILE_SCALE = 15; 
public:
	TerrainLoader() = default;
	~TerrainLoader() = default;
public:
	int GetLength() const;
	MeshData Load(const std::filesystem::path& path, bool patch);
private:
	SimpleMath::Vector3 CalculateNormal(int z, int x) const;
	void CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd);
private:
	std::vector<std::vector<float>> mHeight{};
	int mLength{};
};

class TerrainCollider {
public:
	TerrainCollider() = default;
	~TerrainCollider() = default;
public:
	bool LoadFromFile(const std::filesystem::path& filePath);
	float GetHeight(float x, float z) const;

	TerrainHeader& GetHeader();
	std::vector<SimpleMath::Vector3>& GetData();
private:
	std::vector<SimpleMath::Vector3> mGlobalVertices;
	TerrainHeader mHeader{}; 
};