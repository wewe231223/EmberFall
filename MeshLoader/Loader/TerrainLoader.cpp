#include <fstream>
#include <ranges>
#include "TerrainLoader.h"
#include "../Utility/Crash.h"

#ifdef max 
#undef max
#endif // max

MeshData TerrainLoader::Load(const std::filesystem::path& path, bool patch) {
	MeshData meshData{};

	CrashExp((path.extension() == ".raw"), "Height map should be .raw File");

	// 테셀레이션 패치 
	if (patch) {
		size_t size = std::filesystem::file_size(path);
		mLength = static_cast<int>(::sqrt(size));
		int half = mLength / 2;

		std::ifstream file{ path, std::ios::binary };

		mHeight.resize(mLength, std::vector<float>(mLength));
		std::vector<BYTE> data(mLength);

		for (auto& line : mHeight) {
			file.read(reinterpret_cast<char*>(data.data()), mLength * sizeof(BYTE));
			for (size_t i = 0; auto& dot : line) {
				dot = static_cast<float>(data[i++]);
				dot /= 3.f;

			}
		}

		auto filter = std::views::filter([=](int x) noexcept {return (x % PATCH_LENGTH) == 0; });

		for (const auto& pz : std::views::iota(PATCH_LENGTH, mLength) | std::views::reverse ) {
			if (pz % PATCH_LENGTH != 0) continue;
			for (const auto& px : std::views::iota(0, mLength - PATCH_LENGTH) | filter) {
				CreatePatch(meshData, pz, pz - PATCH_LENGTH, px, px + PATCH_LENGTH);
			}
		}

	} 
	// 일반 
	else {
	
	}
	
	meshData.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
	meshData.indexed = false;
	meshData.unitCount = static_cast<UINT>(meshData.position.size());
	meshData.vertexAttribute.set(0);
	meshData.vertexAttribute.set(2);
	meshData.vertexAttribute.set(3);

	return meshData;
}

void TerrainLoader::CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd) {
	for (int z : std::views::iota(zEnd, zStart + 1) | std::views::reverse) {
		for (int x : std::views::iota(xStart, xEnd + 1)) {
			const DirectX::XMFLOAT2 uv0{ static_cast<float>(x) / (mLength - 1), 1.f - static_cast<float>(z) / (mLength - 1)};
			const DirectX::XMFLOAT2 uv1{ static_cast<float>(x - xStart) / PATCH_LENGTH, static_cast<float>(zStart - z) / PATCH_LENGTH };

			float nz = static_cast<float>(z - mLength / 2);
			float nx = static_cast<float>(x - mLength / 2);

			data.position.emplace_back(nx, mHeight[z][x], nz);
			data.texCoord1.emplace_back(uv0);
			data.texCoord2.emplace_back(uv1);

		}
	}

}
