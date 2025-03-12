#pragma once 

#include <filesystem>
#include "../MeshLoader/Base/MeshData.h"
#include "../Utility/DirectXInclude.h"
#include <vector>

struct TessellatedVertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
};

class TerrainLoader {
	static constexpr int PATCH_LENGTH = 4;
	static constexpr int PATCH_SCALE = 1;
public:
	TerrainLoader() = default;
	~TerrainLoader() = default;
public:
	MeshData Load(const std::filesystem::path& path, bool patch);
	float GetHeightAt(float x, float z) const;
	float GetHeightAtL(float x, float z) const;
private:
	void CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd);
	float BezierSum(float t, float p0, float p1, float p2, float p3) const;
private:
	std::vector<std::vector<float>> mHeight{};
	std::vector<DirectX::XMFLOAT3> mControlPoints{};
	int mLength{};
};
