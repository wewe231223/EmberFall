#include <fstream>
#include <vector>
#include <filesystem>
#include <cmath>
#include <cassert>
#include "../MeshLoader/Loader/TerrainLoader.h"
#include "../Utility/DirectXInclude.h" 

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

struct TerrainHeader {
    int gridWidth;     
    int gridHeight;    
    float gridSpacing; 
    float minX;        
    float minZ;        
};

bool SimulateGlobalTessellationAndWriteFile(const std::filesystem::path& heightMapPath, const std::filesystem::path& outputFilePath) {
    TerrainLoader loader;
    MeshData meshData = loader.Load(heightMapPath, true);

    int PATCH_LENGTH = 4;
	int PATCH_SCALE = 8;

   
    int patchSize = PATCH_LENGTH * PATCH_SCALE;

    int patchesX = (loader.GetLength() - patchSize) / patchSize + 1;
    int patchesZ = (loader.GetLength() - patchSize) / patchSize + 1;

    const int tessFactor = 16;             
    const int tessGridSize = tessFactor + 1;  
    int totalPatches = patchesX * patchesZ;

    std::vector<std::vector<SimpleMath::Vector3>> tessPatches(totalPatches);

    for (int patch = 0; patch < totalPatches; ++patch) {
        auto beginIter = meshData.position.begin() + patch * 25;
        std::vector<SimpleMath::Vector3> controlPoints(beginIter, beginIter + 25);
        std::vector<SimpleMath::Vector3> tessVerts = SimulateTessellationForPatch(controlPoints);
        tessPatches[patch] = std::move(tessVerts);
    }

    std::vector<std::vector<SimpleMath::Vector3>> tessPatchesOrdered(totalPatches);
    for (int r = 0; r < patchesZ; ++r) {
        int srcRow = patchesZ - 1 - r; 
        for (int c = 0; c < patchesX; ++c) {
            int srcIndex = srcRow * patchesX + c;
            int destIndex = r * patchesX + c;
            tessPatchesOrdered[destIndex] = std::move(tessPatches[srcIndex]);
        }
    }

    int globalWidth = patchesX * (tessGridSize - 1) + 1;
    int globalHeight = patchesZ * (tessGridSize - 1) + 1;
    std::vector<SimpleMath::Vector3> globalVertices(globalWidth * globalHeight);

    for (int pr = 0; pr < patchesZ; ++pr) {
        for (int pc = 0; pc < patchesX; ++pc) {
            int patchIdx = pr * patchesX + pc;
            const auto& patchVerts = tessPatchesOrdered[patchIdx];
            for (int i = 0; i < tessGridSize; ++i) {
                for (int j = 0; j < tessGridSize; ++j) {

                    int globalRow = pr * (tessGridSize - 1) + i;
                    int globalCol = pc * (tessGridSize - 1) + j;

                    globalVertices[globalRow * globalWidth + globalCol] = patchVerts[i * tessGridSize + j];
                }
            }
        }
    }

    float minX = globalVertices[0].x;
    float minZ = globalVertices[0].z;

    float gridSpacing = (globalWidth > 1) ? (globalVertices[1].x - globalVertices[0].x) : 1.0f;

    TerrainHeader header{ globalWidth, globalHeight, gridSpacing, minX, minZ };

    std::ofstream outFile(outputFilePath, std::ios::binary);
    if (!outFile) {
        return false;
    }

    outFile.write(reinterpret_cast<const char*>(&header), sizeof(header));
    outFile.write(reinterpret_cast<const char*>(globalVertices.data()), globalVertices.size() * sizeof(SimpleMath::Vector3));

    return true;
}