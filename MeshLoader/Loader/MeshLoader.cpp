#include <ranges>
#include <algorithm>
#include <numeric>
#include "MeshLoader.h"
#include "../Utility/Crash.h"
#include "../EditorInterface/Console/Console.h"

MeshData MeshLoader::Load(const std::filesystem::path& path) {
	MeshData meshData{};

	const aiScene* scene = mImporter.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded);

	CrashExp((!scene or scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or !scene->mRootNode), std::string{ "Failed to load model : " + path.string() }.c_str());

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[i];

		
		if (mesh->HasPositions()) {
			meshData.position.resize(mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.position.emplace_back(mesh->mVertices[vertex].x, mesh->mVertices[vertex].y, mesh->mVertices[vertex].z);
			}
		}

		if (mesh->HasNormals()) {
			meshData.normal.resize(mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.normal.emplace_back(mesh->mNormals[vertex].x, mesh->mNormals[vertex].y, mesh->mNormals[vertex].z);
			}
		}

		if (mesh->HasTextureCoords(0)) {
			meshData.texCoord.resize(mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.texCoord.emplace_back(mesh->mTextureCoords[0][vertex].x, mesh->mTextureCoords[0][vertex].y);
			}
		}

		if (mesh->HasTangentsAndBitangents()) {
			meshData.tangent.resize(mesh->mNumVertices);
			meshData.bitangent.resize(mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.tangent.emplace_back(mesh->mTangents[vertex].x, mesh->mTangents[vertex].y, mesh->mTangents[vertex].z);
				meshData.bitangent.emplace_back(mesh->mBitangents[vertex].x, mesh->mBitangents[vertex].y, mesh->mBitangents[vertex].z);
			}
		}
	
        
        meshData.boneID.resize(mesh->mNumVertices, { 0, 0, 0, 0 });
        meshData.boneWeight.resize(mesh->mNumVertices, { 0.0f, 0.0f, 0.0f, 0.0f });

        
        if (mesh->HasBones()) {
            std::vector<std::vector<std::pair<int, float>>> vertexBoneInfo(mesh->mNumVertices);

            
            for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
                aiBone* bone = mesh->mBones[boneIndex];

                // Bone Weight 순회
                for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
                    aiVertexWeight& vertexWeight = bone->mWeights[weightIndex];

                    int vertexID = vertexWeight.mVertexId;
                    float weight = vertexWeight.mWeight;

                    vertexBoneInfo[vertexID].emplace_back(boneIndex, weight);
                }
            }

            // 이하의 과정은 반드시 필요한 것은 아니다. 
            // 하지만 Assimp 에 aiProcess_LimitBoneWeights 옵션을 적용시켰다고 하더라도, 이가 포맷에 따라 완전히 지원하지 않는다는 보고가 있다. 
            // 이는 런타임에 진행되는 과정이 아니므로, 다소 비효율적일 수 있더라도, 확실히 후처리를 한 뒤 진행한다. 
            // 1. 가장 큰 가중치 4개만을 선택한다 ( 정렬 후 선택 ) 
			// 2. 선택된 가중치들의 합이 1이 되도록 조정한다.
           
            for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
                auto& bones = vertexBoneInfo[vertex];

                std::sort(bones.begin(), bones.end(), [](const auto& a, const auto& b) {
                    return a.second > b.second;
                    });

                auto boneView = bones
                    | std::views::take(4) // 앞에서 4개의 pair 선택
                    | std::views::transform([](const std::pair<int, float>& p) { 
                    return p.second;
                        });

				
                std::ranges::copy(boneView, meshData.boneWeight[vertex].begin());

                float totalWeight{ std::accumulate(boneView.begin(), boneView.end(),0.f) };

                if (totalWeight > 0.0f) {
                    for (size_t j = 0; j < 4; ++j) {
                        meshData.boneWeight[vertex][j] /= totalWeight;
                    }
                }
            }
        }

	}


	return meshData;
}
