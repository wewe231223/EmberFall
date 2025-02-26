#include <fstream>
#include <ranges>
#include "TerrainLoader.h"
#include "../Utility/Crash.h"

#ifdef max 
#undef max
#endif // max

//MeshData TerrainLoader::Load(const std::filesystem::path& path, bool patch) {
//	MeshData meshData{};
//
//	CrashExp((path.extension() == ".raw"), "Height map should be .raw File");
//
//	// 테셀레이션 패치 
//	if (patch) {
//		size_t size = std::filesystem::file_size(path);
//		mLength = static_cast<int>(::sqrt(size));
//		int half = mLength / 2;
//
//		std::ifstream file{ path, std::ios::binary };
//
//		mHeight.resize(mLength, std::vector<float>(mLength));
//		std::vector<BYTE> data(mLength);
//
//		for (auto& line : mHeight) {
//			file.read(reinterpret_cast<char*>(data.data()), mLength * sizeof(BYTE));
//			for (size_t i = 0; auto& dot : line) {
//				dot = static_cast<float>(data[i++]);
//				dot /= 3.f;
//
//			}
//		}
//
//		auto filter = std::views::filter([=](int x) noexcept {return (x % PATCH_LENGTH) == 0; });
//
//		for (const auto& pz : std::views::iota(PATCH_LENGTH, mLength) | std::views::reverse ) {
//			if (pz % PATCH_LENGTH != 0) continue;
//			for (const auto& px : std::views::iota(0, mLength - PATCH_LENGTH) | filter) {
//				CreatePatch(meshData, pz, pz - PATCH_LENGTH, px, px + PATCH_LENGTH);
//			}
//		}
//
//		
//
//		meshData.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
//		meshData.indexed = false;
//		meshData.unitCount = static_cast<UINT>(meshData.position.size());
//		meshData.vertexAttribute.set(0);
//		meshData.vertexAttribute.set(2);
//		meshData.vertexAttribute.set(3);
//	} 
//	// 일반 
//	else {
//	
//	}
//	
//
//	return meshData;
//}
//
//
//void TerrainLoader::CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd) {
//	for (int z : std::views::iota(zEnd, zStart + 1) | std::views::reverse) {
//		for (int x : std::views::iota(xStart, xEnd + 1)) {
//			const DirectX::XMFLOAT2 uv0{ static_cast<float>(x) / (mLength - 1), 1.f - static_cast<float>(z) / (mLength - 1)};
//			const DirectX::XMFLOAT2 uv1{ static_cast<float>(x - xStart) / PATCH_LENGTH, static_cast<float>(zStart - z) / PATCH_LENGTH };
//
//			float nz = static_cast<float>(z - mLength / 2);
//			float nx = static_cast<float>(x - mLength / 2);
//
//			data.position.emplace_back(nx, mHeight[z][x], nz);
//			data.texCoord1.emplace_back(uv0);
//			data.texCoord2.emplace_back(uv1);
//
//		}
//	}
//
//}

MeshData TerrainLoader::Load(const std::filesystem::path& path, bool patch) {
    MeshData meshData{};

    CrashExp((path.extension() == ".raw"), "Height map should be .raw File");

    if (patch) {
        size_t size = std::filesystem::file_size(path);
        mLength = static_cast<int>(::sqrt(size));
        int half = mLength / 2;

        std::ifstream file{ path, std::ios::binary };

        mHeight.resize(mLength, std::vector<float>(mLength));
        std::vector<BYTE> data(mLength);

        for (auto& line : mHeight) {
            file.read(reinterpret_cast<char*>(data.data()), mLength * sizeof(BYTE));
            for (size_t i = 0; auto & dot : line) {
                dot = static_cast<float>(data[i++]);
            }
        }

        int patchSize = PATCH_LENGTH * PATCH_SCALE;
        for (int pz = mLength - patchSize; pz >= 0; pz -= patchSize) {
            for (int px = 0; px <= mLength - patchSize; px += patchSize) {
                CreatePatch(meshData, pz + patchSize, pz, px, px + patchSize);
            }
        }

        meshData.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
        meshData.indexed = false;
        meshData.unitCount = static_cast<UINT>(meshData.position.size());
        meshData.vertexAttribute.set(0);
        meshData.vertexAttribute.set(2);
        meshData.vertexAttribute.set(3);
    }
    else {
        // 일반 처리 (변경 없음)
    }

    return meshData;
}



void TerrainLoader::CreatePatch(MeshData& data, int zStart, int zEnd, int xStart, int xEnd) {

    float stepSize = static_cast<float>(zStart - zEnd) / PATCH_LENGTH;

    for (int i = 0; i <= PATCH_LENGTH; ++i) {
        for (int j = 0; j <= PATCH_LENGTH; ++j) {
            int z = std::clamp(static_cast<int>(zStart - i * stepSize), 0, mLength - 1);
            int x = std::clamp(static_cast<int>(xStart + j * stepSize), 0, mLength - 1);

            const DirectX::XMFLOAT2 uv0{ static_cast<float>(x) / (mLength - 1), 1.f - static_cast<float>(z) / (mLength - 1) };
            const DirectX::XMFLOAT2 uv1{ static_cast<float>(j) / PATCH_LENGTH, static_cast<float>(i) / PATCH_LENGTH };

            float nz = static_cast<float>(z - mLength / 2);
            float nx = static_cast<float>(x - mLength / 2);

            data.position.emplace_back(nx, mHeight[z][x], nz);
            data.texCoord1.emplace_back(uv0);
            data.texCoord2.emplace_back(uv1);
        }
    }
}
