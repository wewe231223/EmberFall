#include <ranges>
#include <algorithm>
#include <numeric>
#include "MeshLoader.h"
#include "../Utility/Crash.h"
#include "../EditorInterface/Console/Console.h"


// 파일에 기록하고 읽는 거 만들기. 

MeshData MeshLoader::Load(const std::filesystem::path& path) {
	MeshData meshData{};

	const aiScene* scene = mImporter.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_LimitBoneWeights | aiProcess_MakeLeftHanded);

	CrashExp(scene != nullptr, "Failed To Load Model!");
	CrashExp((!(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)), "Failed To Load Model!");
	CrashExp((scene->mRootNode != nullptr), "Failed To Load Model!");


	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[i];

		if (mesh->HasPositions()) {
			meshData.position.reserve(mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.position.emplace_back(mesh->mVertices[vertex].x, mesh->mVertices[vertex].y, mesh->mVertices[vertex].z);
			}
			meshData.vertexAttribute.set(0);
		}

		if (mesh->HasNormals()) {
			meshData.normal.reserve(mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.normal.emplace_back(mesh->mNormals[vertex].x, mesh->mNormals[vertex].y, mesh->mNormals[vertex].z);
			}
			meshData.vertexAttribute.set(1);
		}

		if (mesh->HasTextureCoords(0)) {
			meshData.texCoord.reserve(mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.texCoord.emplace_back(mesh->mTextureCoords[0][vertex].x, mesh->mTextureCoords[0][vertex].y);
			}
			meshData.vertexAttribute.set(2);
		}

		if (mesh->HasTangentsAndBitangents()) {
			meshData.tangent.reserve(mesh->mNumVertices);
			meshData.bitangent.reserve(mesh->mNumVertices);
			for (unsigned int vertex = 0; vertex < mesh->mNumVertices; ++vertex) {
				meshData.tangent.emplace_back(mesh->mTangents[vertex].x, mesh->mTangents[vertex].y, mesh->mTangents[vertex].z);
				meshData.bitangent.emplace_back(mesh->mBitangents[vertex].x, mesh->mBitangents[vertex].y, mesh->mBitangents[vertex].z);
			}
			meshData.vertexAttribute.set(3);
			meshData.vertexAttribute.set(4);
		}
	    
		if (mesh->HasFaces()) {
			meshData.index.reserve(mesh->mNumFaces * 3);
			for (unsigned int face = 0; face < mesh->mNumFaces; ++face) {
				meshData.index.emplace_back(mesh->mFaces[face].mIndices[0]);
				meshData.index.emplace_back(mesh->mFaces[face].mIndices[1]);
				meshData.index.emplace_back(mesh->mFaces[face].mIndices[2]);
			}
			meshData.indexed = true;
		}
		else {
			meshData.indexed = false;
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

				auto boneView = bones | std::views::take(4);  

				auto boneIDView = boneView | std::views::transform([](const auto& bone) { return bone.first; });
				auto boneWeightView = boneView | std::views::transform([](const auto& bone) { return bone.second; });
				
				std::ranges::copy(boneIDView, meshData.boneID[vertex].begin());
                std::ranges::copy(boneWeightView, meshData.boneWeight[vertex].begin());

                float totalWeight{ std::accumulate(boneWeightView.begin(), boneWeightView.end(),0.f) };

                if (totalWeight > 0.0f) {
                    for (size_t j = 0; j < 4; ++j) {
                        meshData.boneWeight[vertex][j] /= totalWeight;
                    }
                }
            }
			meshData.vertexAttribute.set(5);
			meshData.vertexAttribute.set(6);
        }

	}


	return meshData;
}
