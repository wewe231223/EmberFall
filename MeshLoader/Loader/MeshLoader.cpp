#include <ranges>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include "MeshLoader.h"
#include "../Utility/Crash.h"
#include "../EditorInterface/Console/Console.h"

Assimp::Importer MeshLoader::mImporter{};
// 파일에 기록하고 읽는 거 만들기. 

MeshData MeshLoader::Load(const std::filesystem::path& path) {
	MeshData meshData{};

	const aiScene* scene = mImporter.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded  );

	CrashExp(scene != nullptr, "Failed To Load Model!");
	CrashExp((!(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)), "Failed To Load Model!");
	CrashExp((scene->mRootNode != nullptr), "Failed To Load Model!");

	if (!scene or scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or !scene->mRootNode) {
		Console.Log("ModelLoad Failed : {}",LogType::Error, mImporter.GetErrorString());
		Crash("ModelLoad Failed");
	}

	UINT vertexOffset{ 0 };

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[i];
		
		if (mesh->HasPositions()) {
			//meshData.position.reserve(meshData.position.size() + mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.position.emplace_back(mesh->mVertices[vertex].x, mesh->mVertices[vertex].y, mesh->mVertices[vertex].z);
			}
			meshData.vertexAttribute.set(0);
		}

		if (mesh->HasNormals()) {
			meshData.normal.reserve(meshData.normal.size() + mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.normal.emplace_back(mesh->mNormals[vertex].x, mesh->mNormals[vertex].y, mesh->mNormals[vertex].z);
			}
			meshData.vertexAttribute.set(1);
		}

		if (mesh->HasTextureCoords(0)) {
			meshData.texCoord1.reserve(meshData.texCoord1.size() + mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.texCoord1.emplace_back(mesh->mTextureCoords[0][vertex].x, mesh->mTextureCoords[0][vertex].y);
			}
			meshData.vertexAttribute.set(2);
		}

		if (mesh->HasTextureCoords(1)) {
			meshData.texCoord2.reserve(meshData.texCoord2.size() + mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.texCoord2.emplace_back(mesh->mTextureCoords[1][vertex].x, mesh->mTextureCoords[1][vertex].y);
			}
			meshData.vertexAttribute.set(3);
		}

		if (mesh->HasTangentsAndBitangents()) {
			meshData.tangent.reserve(meshData.tangent.size() + mesh->mNumVertices);
			meshData.bitangent.reserve(meshData.bitangent.size() + mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.tangent.emplace_back(mesh->mTangents[vertex].x, mesh->mTangents[vertex].y, mesh->mTangents[vertex].z);
				meshData.bitangent.emplace_back(mesh->mBitangents[vertex].x, mesh->mBitangents[vertex].y, mesh->mBitangents[vertex].z);
			}
			meshData.vertexAttribute.set(4);
			meshData.vertexAttribute.set(5);
		}
	    
		if (mesh->HasFaces()) {
		
			for (unsigned int face = 0; face < mesh->mNumFaces; ++face) {
				meshData.index.emplace_back(vertexOffset + mesh->mFaces[face].mIndices[0]);
				meshData.index.emplace_back(vertexOffset + mesh->mFaces[face].mIndices[1]);
				meshData.index.emplace_back(vertexOffset + mesh->mFaces[face].mIndices[2]);
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

			std::unordered_map<std::string, UINT> boneMap{};

			UINT boneNumbers{ 0 };
			
			for (UINT b = 0; b < mesh->mNumBones; ++b) {
				UINT boneIndex{ 0 };
				std::string boneName = mesh->mBones[b]->mName.C_Str();
				
				if (boneMap.find(boneName) == boneMap.end()) {
					boneIndex = boneNumbers;
					boneNumbers++;
					boneMap[boneName] = boneIndex;
				}
				else {
					boneIndex = boneMap[boneName];
				}

					
				for (UINT j = 0; j < mesh->mBones[b]->mNumWeights; ++j) {
					int vertexID = static_cast<int>(mesh->mBones[b]->mWeights[j].mVertexId);
					float weight = mesh->mBones[b]->mWeights[j].mWeight;

					for (auto k = 0; k < 4; ++k) {
						if (meshData.boneWeight[vertexID][k] == 0.0f) {
							meshData.boneID[vertexID][k] = boneIndex;
							meshData.boneWeight[vertexID][k] = weight;
							break;
						}
					}
					
				}

			}

			meshData.vertexAttribute.set(6);
			meshData.vertexAttribute.set(7);
        }

	}


	return meshData;
}
