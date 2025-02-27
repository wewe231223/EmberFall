#include <functional>
#include "AnimationLoader.h"
#include "../Utility/Crash.h"
#include "../EditorInterface/Console/Console.h"

AnimationClip AnimationLoader::Load(const std::filesystem::path& path, UINT animIndex) {
	Assimp::Importer importer{};
	const aiScene* scene = importer.ReadFile(path.string(), aiProcess_LimitBoneWeights | aiProcess_OptimizeGraph );

	CrashExp(scene != nullptr, "Failed To Load Animation!");
	CrashExp((!(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)), "Failed To Load Animation!");
	CrashExp((scene->mRootNode != nullptr), "Failed To Load Animation!");

	if (!scene or scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE or !scene->mRootNode) {
		Console.Log("Animation Load Failed : {}", LogType::Error, mImporter.GetErrorString());
		Crash("Animation Load Failed");
	}

	CrashExp(scene->mNumAnimations != 0, "Any Animation Data not Found");
	CrashExp(animIndex < scene->mNumAnimations, "Invalid Animation Index");

	aiAnimation* aiAnim = scene->mAnimations[animIndex];
	aiNode* rootNode = scene->mRootNode;

    AnimationClip clip;
    clip.duration = static_cast<float>(aiAnim->mDuration);
    clip.ticksPerSecond = aiAnim->mTicksPerSecond != 0 ? static_cast<float>(aiAnim->mTicksPerSecond) : 25.0f;

	std::unordered_map<std::string, UINT> boneMap{};

	UINT boneIndex{ 0 };
	UINT boneNumbers{ 0 };

	for (UINT i = 0; i < scene->mNumMeshes; ++i) {
		auto mesh = scene->mMeshes[i];
		for (UINT j = 0; j < mesh->mNumBones; ++j) {
			std::string boneName = mesh->mBones[j]->mName.C_Str();
			if (boneMap.find(boneName) == boneMap.end()) {
				boneIndex = boneNumbers;
				boneNumbers++;
				boneMap[boneName] = boneIndex;

				clip.boneOffsetMatrices.emplace_back();
				clip.boneOffsetMatrices[boneIndex] = SimpleMath::Matrix{
					mesh->mBones[j]->mOffsetMatrix.a1, mesh->mBones[j]->mOffsetMatrix.a2, mesh->mBones[j]->mOffsetMatrix.a3, mesh->mBones[j]->mOffsetMatrix.a4,
					mesh->mBones[j]->mOffsetMatrix.b1, mesh->mBones[j]->mOffsetMatrix.b2, mesh->mBones[j]->mOffsetMatrix.b3, mesh->mBones[j]->mOffsetMatrix.b4,
					mesh->mBones[j]->mOffsetMatrix.c1, mesh->mBones[j]->mOffsetMatrix.c2, mesh->mBones[j]->mOffsetMatrix.c3, mesh->mBones[j]->mOffsetMatrix.c4,
					mesh->mBones[j]->mOffsetMatrix.d1, mesh->mBones[j]->mOffsetMatrix.d2, mesh->mBones[j]->mOffsetMatrix.d3, mesh->mBones[j]->mOffsetMatrix.d4
				};
			}
			else {
				boneIndex = boneMap[boneName];
			}

		}
	}

	clip.boneIndexMap = boneMap;
	clip.root = buildHierarchy(rootNode);




    for (UINT i = 0; i < aiAnim->mNumChannels; ++i) {
        const aiNodeAnim* nodeAnim = aiAnim->mChannels[i];
        
        BoneAnimation boneAnim;
        
        boneAnim.boneName = nodeAnim->mNodeName.data;
       
		for (UINT j = 0; j < nodeAnim->mNumPositionKeys; ++j) {
			boneAnim.positionKey.emplace_back(nodeAnim->mPositionKeys[j].mTime, SimpleMath::Vector3(nodeAnim->mPositionKeys[j].mValue.x, nodeAnim->mPositionKeys[j].mValue.y, nodeAnim->mPositionKeys[j].mValue.z));
		}

		for (UINT j = 0; j < nodeAnim->mNumRotationKeys; ++j) {
			boneAnim.rotationKey.emplace_back(nodeAnim->mRotationKeys[j].mTime, SimpleMath::Quaternion(nodeAnim->mRotationKeys[j].mValue.x, nodeAnim->mRotationKeys[j].mValue.y, nodeAnim->mRotationKeys[j].mValue.z, nodeAnim->mRotationKeys[j].mValue.w));
		}

		for (UINT j = 0; j < nodeAnim->mNumScalingKeys; ++j) {
			boneAnim.scalingKey.emplace_back(nodeAnim->mScalingKeys[j].mTime, SimpleMath::Vector3(nodeAnim->mScalingKeys[j].mValue.x, nodeAnim->mScalingKeys[j].mValue.y, nodeAnim->mScalingKeys[j].mValue.z));
		}

        clip.boneAnimations[boneAnim.boneName] = boneAnim;
    }

	auto giTransform = scene->mRootNode->mTransformation.Inverse();

	clip.globalInverseTransform = SimpleMath::Matrix{
		giTransform.a1, giTransform.a2, giTransform.a3, giTransform.a4,
		giTransform.b1, giTransform.b2, giTransform.b3, giTransform.b4,
		giTransform.c1, giTransform.c2, giTransform.c3, giTransform.c4,
		giTransform.d1, giTransform.d2, giTransform.d3, giTransform.d4
	};


    return clip;

}

std::shared_ptr<BoneNode> AnimationLoader::buildHierarchy(const aiNode* node) {
	std::shared_ptr<BoneNode> newNode{ std::make_shared<BoneNode>() };

	newNode->name = node->mName.C_Str();
	XMStoreFloat4x4(&newNode->transformation, DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node->mTransformation)));

	newNode->transformation = SimpleMath::Matrix{
		node->mTransformation.a1, node->mTransformation.a2, node->mTransformation.a3, node->mTransformation.a4,
		node->mTransformation.b1, node->mTransformation.b2, node->mTransformation.b3, node->mTransformation.b4,
		node->mTransformation.c1, node->mTransformation.c2, node->mTransformation.c3, node->mTransformation.c4,
		node->mTransformation.d1, node->mTransformation.d2, node->mTransformation.d3, node->mTransformation.d4
	};

	for (UINT i = 0; i < node->mNumChildren; ++i) {
		std::shared_ptr<BoneNode> child = buildHierarchy(node->mChildren[i]);
		newNode->children.emplace_back(child);
	}
	return newNode;
}