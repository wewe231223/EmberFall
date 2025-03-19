#include <fstream>
#include <vector>
#include <filesystem>
#include <cmath>
#include <cassert>
#include "../MeshLoader/Loader/TerrainLoader.h"
#include "../Utility/DirectXInclude.h" 

using namespace SimpleMath;


static float BernsteinBasis(int i, float t) {
    switch (i) {
    case 0: return (1.0f - t) * (1.0f - t) * (1.0f - t) * (1.0f - t);
    case 1: return 4.0f * t * (1.0f - t) * (1.0f - t) * (1.0f - t);
    case 2: return 6.0f * t * t * (1.0f - t) * (1.0f - t);
    case 3: return 4.0f * t * t * t * (1.0f - t);
    case 4: return t * t * t * t;
    default: return 0.0f;
    }
}

static SimpleMath::Vector3 EvaluateBiQuarticBezierPatch(const SimpleMath::Vector3 control[5][5], float u, float v) {
    SimpleMath::Vector3 result(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 5; ++i) {
        float bu = BernsteinBasis(i, u);
        for (int j = 0; j < 5; ++j) {
            float bv = BernsteinBasis(j, v);
            result += control[i][j] * (bu * bv);
        }
    }
    return result;
}

static std::vector<::DirectX::SimpleMath::Vector3> SimulateTessellationForPatch(const std::vector<SimpleMath::Vector3>& patchControlPoints) {
    const int tessFactor = 16; // 고정 테셀레이션 팩터: 16분할 → 17×17 정점
    const int gridSize = tessFactor + 1; // 17

    SimpleMath::Vector3 bezier[5][5];
    for (int i = 0; i < 5; ++i) {
        int flippedRow = 4 - i; // 반전된 행 인덱스
        for (int j = 0; j < 5; ++j) {
            bezier[flippedRow][j] = patchControlPoints[i * 5 + j];
        }
    }

    std::vector<SimpleMath::Vector3> tessellated;
    tessellated.reserve(static_cast<::std::size_t>(gridSize * gridSize));

    for (int i = 0; i <= tessFactor; ++i) {
        float u = static_cast<float>(i) / static_cast<float>(tessFactor);
        for (int j = 0; j <= tessFactor; ++j) {
            float v = static_cast<float>(j) / static_cast<float>(tessFactor);
            SimpleMath::Vector3 pos = EvaluateBiQuarticBezierPatch(bezier, u, v);
            tessellated.push_back(pos);
        }
    }
    return tessellated;
}

bool SimulateTessellationAndWriteFile(const std::filesystem::path& heightMapPath, const std::filesystem::path& outputFilePath) {
    TerrainLoader loader;
    MeshData meshData = loader.Load(heightMapPath, true);

    // 각 패치 당 컨트롤 포인트 개수: 25개 (5×5)
    const int patchControlPointCount = 25;
    std::size_t numPatches = meshData.position.size() / static_cast<size_t>(patchControlPointCount);

    const int tessFactor = 16;
    const int tessGridSize = tessFactor + 1; // 17

    std::vector<TessellatedPatch> patches;
    patches.reserve(numPatches);

    for (size_t patchIndex = 0; patchIndex < numPatches; ++patchIndex) {
        auto patchBegin = meshData.position.begin() + patchIndex * patchControlPointCount;
        std::vector<SimpleMath::Vector3> controlPoints(patchBegin, patchBegin + patchControlPointCount);

        // CPU에서 테셀레이션 시뮬레이션
        std::vector<SimpleMath::Vector3> tessVertices = SimulateTessellationForPatch(controlPoints);

        float minX = controlPoints[0].x;
        float maxX = controlPoints[0].x;
        float minZ = controlPoints[0].z;
        float maxZ = controlPoints[0].z;
        for (const auto& pt : controlPoints)
        {
            if (pt.x < minX) { minX = pt.x; }
            if (pt.x > maxX) { maxX = pt.x; }
            if (pt.z < minZ) { minZ = pt.z; }
            if (pt.z > maxZ) { maxZ = pt.z; }
        }
        // gridSpacing: tessellated 패치 내 x 방향 간격 = z 방향 간격
        float gridSpacing = (maxX - minX) / static_cast<float>(tessGridSize - 1);

        TessellatedPatch patch;
        patch.header.gridWidth = tessGridSize;
        patch.header.gridHeight = tessGridSize;
        patch.header.gridSpacing = gridSpacing;
        patch.header.minX = minX;
        patch.header.minZ = minZ;
        patch.vertices = std::move(tessVertices);

        patches.push_back(std::move(patch));
    }

    // 파일 포맷:
    // 각 패치별 헤더: int gridWidth, int gridHeight, float gridSpacing, float minX, float minZ
    // 그 후, 각 패치의 tessellated vertex 배열 
    std::ofstream outFile(outputFilePath, std::ios::binary);
    if (!outFile) {
        return false;
    }

    int patchCount = static_cast<int>(patches.size());
    outFile.write(reinterpret_cast<const char*>(&patchCount), sizeof(patchCount));

    for (const auto& patch : patches) {
        outFile.write(reinterpret_cast<const char*>(&patch.header.gridWidth), sizeof(patch.header.gridWidth));
        outFile.write(reinterpret_cast<const char*>(&patch.header.gridHeight), sizeof(patch.header.gridHeight));
        outFile.write(reinterpret_cast<const char*>(&patch.header.gridSpacing), sizeof(patch.header.gridSpacing));
        outFile.write(reinterpret_cast<const char*>(&patch.header.minX), sizeof(patch.header.minX));
        outFile.write(reinterpret_cast<const char*>(&patch.header.minZ), sizeof(patch.header.minZ));
    }

    for (const auto& patch : patches) {
        outFile.write(reinterpret_cast<const char*>(patch.vertices.data()),
            patch.vertices.size() * sizeof(SimpleMath::Vector3));
    }

    return true;
}