#include <ranges>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include "MeshLoader.h"
#include "../Utility/Crash.h"
#include "../Renderer/Core/Console.h"

#ifndef EXCLUDE_ASSIMP
Assimp::Importer MeshLoader::mImporter{};
#endif 

MeshData MeshLoader::Load(const std::filesystem::path& path, UINT meshIndex) {
    std::string baseName = path.stem().string();
    std::string cacheName = baseName + std::to_string(meshIndex) + ".bin";
    std::filesystem::path cachePath = BinaryDirectory / (cacheName + ".bin");

    MeshData meshData{};
    if (std::filesystem::exists(cachePath)) {
        if (LoadFromBinary(cachePath, meshData)) {
            return meshData;
        }
    }

#ifndef EXCLUDE_ASSIMP
    const aiScene* scene = mImporter.ReadFile(
        path.string(),
        aiProcess_Triangulate |
        aiProcess_CalcTangentSpace |
        aiProcess_ConvertToLeftHanded
    );

    CrashExp(scene && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode, "Failed To Load Model!");

    if (meshIndex < scene->mNumMeshes) {
        aiMesh* mesh = scene->mMeshes[meshIndex];

        if (mesh->HasPositions()) {
            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                meshData.position.emplace_back(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
            }
            meshData.vertexAttribute.set(0);
        }

        if (mesh->HasNormals()) {
            meshData.normal.reserve(meshData.normal.size() + mesh->mNumVertices);
            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                meshData.normal.emplace_back(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
            }
            meshData.vertexAttribute.set(1);
        }

        if (mesh->HasTextureCoords(0)) {
            meshData.texCoord1.reserve(meshData.texCoord1.size() + mesh->mNumVertices);
            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                meshData.texCoord1.emplace_back(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
            }
            meshData.vertexAttribute.set(2);
        }

        if (mesh->HasTextureCoords(1)) {
            meshData.texCoord2.reserve(meshData.texCoord2.size() + mesh->mNumVertices);
            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                meshData.texCoord2.emplace_back(mesh->mTextureCoords[1][v].x, mesh->mTextureCoords[1][v].y);
            }
            meshData.vertexAttribute.set(3);
        }

        if (mesh->HasTangentsAndBitangents()) {
            meshData.tangent.reserve(meshData.tangent.size() + mesh->mNumVertices);
            meshData.bitangent.reserve(meshData.bitangent.size() + mesh->mNumVertices);
            for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                meshData.tangent.emplace_back(mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z);
                meshData.bitangent.emplace_back(mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z);
            }
            meshData.vertexAttribute.set(4);
            meshData.vertexAttribute.set(5);
        }

        if (mesh->HasFaces()) {
            for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
                meshData.index.emplace_back(mesh->mFaces[f].mIndices[0]);
                meshData.index.emplace_back(mesh->mFaces[f].mIndices[1]);
                meshData.index.emplace_back(mesh->mFaces[f].mIndices[2]);
            }
            meshData.indexed = true;
            meshData.unitCount = static_cast<unsigned int>(meshData.index.size());
        }
        else {
            meshData.indexed = false;
            meshData.unitCount = mesh->mNumVertices;
        }

        if (mesh->HasBones()) {
            meshData.boneID.resize(meshData.boneID.size() + mesh->mNumVertices);
            meshData.boneWeight.resize(meshData.boneWeight.size() + mesh->mNumVertices);
            std::unordered_map<std::string, UINT> boneMap;
            UINT boneCount = 0;
            for (UINT b = 0; b < mesh->mNumBones; ++b) {
                std::string bn = mesh->mBones[b]->mName.C_Str();
                UINT bi = boneMap.count(bn) ? boneMap[bn] : (boneMap[bn] = boneCount++);
                for (UINT w = 0; w < mesh->mBones[b]->mNumWeights; ++w) {
                    int vid = mesh->mBones[b]->mWeights[w].mVertexId;
                    float wght = mesh->mBones[b]->mWeights[w].mWeight;
                    for (int k = 0; k < 4; ++k) {
                        if (meshData.boneWeight[vid][k] == 0.f) {
                            meshData.boneID[vid][k] = bi;
                            meshData.boneWeight[vid][k] = wght;
                            break;
                        }
                    }
                }
            }
            meshData.vertexAttribute.set(6);
            meshData.vertexAttribute.set(7);
        }
    }
    else {
        UINT vertexOffset = 0;
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[i];

            if (mesh->HasPositions()) {
                for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                    meshData.position.emplace_back(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
                }
                meshData.vertexAttribute.set(0);
            }

            if (mesh->HasNormals()) {
                meshData.normal.reserve(meshData.normal.size() + mesh->mNumVertices);
                for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                    meshData.normal.emplace_back(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
                }
                meshData.vertexAttribute.set(1);
            }

            if (mesh->HasTextureCoords(0)) {
                meshData.texCoord1.reserve(meshData.texCoord1.size() + mesh->mNumVertices);
                for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                    meshData.texCoord1.emplace_back(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
                }
                meshData.vertexAttribute.set(2);
            }

            if (mesh->HasTextureCoords(1)) {
                meshData.texCoord2.reserve(meshData.texCoord2.size() + mesh->mNumVertices);
                for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                    meshData.texCoord2.emplace_back(mesh->mTextureCoords[1][v].x, mesh->mTextureCoords[1][v].y);
                }
                meshData.vertexAttribute.set(3);
            }

            if (mesh->HasTangentsAndBitangents()) {
                meshData.tangent.reserve(meshData.tangent.size() + mesh->mNumVertices);
                meshData.bitangent.reserve(meshData.bitangent.size() + mesh->mNumVertices);
                for (unsigned int v = 0; v < mesh->mNumVertices; ++v) {
                    meshData.tangent.emplace_back(mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z);
                    meshData.bitangent.emplace_back(mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z);
                }
                meshData.vertexAttribute.set(4);
                meshData.vertexAttribute.set(5);
            }

            if (mesh->HasFaces()) {
                for (unsigned int f = 0; f < mesh->mNumFaces; ++f) {
                    meshData.index.emplace_back(vertexOffset + mesh->mFaces[f].mIndices[0]);
                    meshData.index.emplace_back(vertexOffset + mesh->mFaces[f].mIndices[1]);
                    meshData.index.emplace_back(vertexOffset + mesh->mFaces[f].mIndices[2]);
                }
                vertexOffset += mesh->mNumVertices;
                meshData.indexed = true;
                meshData.unitCount = static_cast<unsigned int>(meshData.index.size());
            }
            else {
                meshData.indexed = false;
                meshData.unitCount = mesh->mNumVertices;
            }

            if (mesh->HasBones()) {
                meshData.boneID.resize(meshData.boneID.size() + mesh->mNumVertices);
                meshData.boneWeight.resize(meshData.boneWeight.size() + mesh->mNumVertices);
                std::unordered_map<std::string, UINT> boneMap;
                UINT boneCount = 0;
                for (UINT b = 0; b < mesh->mNumBones; ++b) {
                    std::string bn = mesh->mBones[b]->mName.C_Str();
                    UINT bi = boneMap.count(bn) ? boneMap[bn] : (boneMap[bn] = boneCount++);
                    for (UINT w = 0; w < mesh->mBones[b]->mNumWeights; ++w) {
                        int vid = mesh->mBones[b]->mWeights[w].mVertexId;
                        float wght = mesh->mBones[b]->mWeights[w].mWeight;
                        for (int k = 0; k < 4; ++k) {
                            if (meshData.boneWeight[vid][k] == 0.f) {
                                meshData.boneID[vid][k] = bi;
                                meshData.boneWeight[vid][k] = wght;
                                break;
                            }
                        }
                    }
                }
                meshData.vertexAttribute.set(6);
                meshData.vertexAttribute.set(7);
            }
        }
    }

    SaveToBinary(meshData, cachePath);
#else 
    CrashExp(false, "Assimp is excluded from the build! Cannot load mesh data from file");
#endif 

    return meshData;
}

bool MeshLoader::SaveToBinary(const MeshData& meshData, const std::filesystem::path& binaryPath) {
    std::ofstream ofs(binaryPath, std::ios::binary);
    CrashExp(ofs.is_open(), "Failed to open binary file for writing!");

    ofs.write(reinterpret_cast<const char*>(&meshData.indexed), sizeof(meshData.indexed));
    ofs.write(reinterpret_cast<const char*>(&meshData.unitCount), sizeof(meshData.unitCount));

    uint32_t attrMask = static_cast<uint32_t>(meshData.vertexAttribute.to_ulong());
    ofs.write(reinterpret_cast<const char*>(&attrMask), sizeof(attrMask));

    auto writeVector = [&ofs](const auto& vec) {
        uint64_t count = vec.size();
        ofs.write(reinterpret_cast<const char*>(&count), sizeof(count));
        if (count) {
            ofs.write(reinterpret_cast<const char*>(vec.data()), sizeof(vec[0]) * count);
        }
        };

    writeVector(meshData.position);
    writeVector(meshData.normal);
    writeVector(meshData.texCoord1);
    writeVector(meshData.texCoord2);
    writeVector(meshData.tangent);
    writeVector(meshData.bitangent);
    writeVector(meshData.boneID);
    writeVector(meshData.boneWeight);
    writeVector(meshData.index);

    return true;
}

bool MeshLoader::LoadFromBinary(const std::filesystem::path& binaryPath, MeshData& outMeshData) {
    std::ifstream ifs(binaryPath, std::ios::binary);
    CrashExp(ifs.is_open(), "Failed to open binary file for reading!");

    ifs.read(reinterpret_cast<char*>(&outMeshData.indexed), sizeof(outMeshData.indexed));
    ifs.read(reinterpret_cast<char*>(&outMeshData.unitCount), sizeof(outMeshData.unitCount));

    uint32_t attrMask = 0;
    ifs.read(reinterpret_cast<char*>(&attrMask), sizeof(attrMask));
    outMeshData.vertexAttribute = decltype(outMeshData.vertexAttribute)(attrMask);

    auto readVector = [&ifs](auto& vec) {
        uint64_t count = 0;
        ifs.read(reinterpret_cast<char*>(&count), sizeof(count));
        vec.resize(count);
        if (count) {
            ifs.read(reinterpret_cast<char*>(vec.data()), sizeof(vec[0]) * count);
        }
        };

    readVector(outMeshData.position);
    readVector(outMeshData.normal);
    readVector(outMeshData.texCoord1);
    readVector(outMeshData.texCoord2);
    readVector(outMeshData.tangent);
    readVector(outMeshData.bitangent);
    readVector(outMeshData.boneID);
    readVector(outMeshData.boneWeight);
    readVector(outMeshData.index);

    return true;
}
