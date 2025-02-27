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

    std::unordered_map<std::string, BoneNode*> nodeMap;
   
    std::function<BoneNode* (const aiNode*)> buildHierarchy = [&](const aiNode* node) -> BoneNode* {
        BoneNode* newNode = new BoneNode();
        newNode->name = node->mName.C_Str();
        XMStoreFloat4x4(&newNode->transformation, DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node->mTransformation)));
        nodeMap[newNode->name] = newNode;
        for (UINT i = 0; i < node->mNumChildren; ++i) {
            BoneNode* child = buildHierarchy(node->mChildren[i]);
            child->parent = newNode;
            newNode->children.push_back(child);
        }
        return newNode;
        };

	clip.root = buildHierarchy(rootNode);

    for (UINT i = 0; i < aiAnim->mNumChannels; ++i) {
        const aiNodeAnim* nodeAnim = aiAnim->mChannels[i];
        
        BoneAnimation boneAnim;
        
        boneAnim.boneName = nodeAnim->mNodeName.data;
       
		for (UINT j = 0; j < nodeAnim->mNumPositionKeys; ++j) {
			boneAnim.positionKey.emplace_back(nodeAnim->mPositionKeys[j].mTime, DirectX::XMFLOAT3(nodeAnim->mPositionKeys[j].mValue.x, nodeAnim->mPositionKeys[j].mValue.y, nodeAnim->mPositionKeys[j].mValue.z));
		}

		for (UINT j = 0; j < nodeAnim->mNumRotationKeys; ++j) {
			boneAnim.rotationKey.emplace_back(nodeAnim->mRotationKeys[j].mTime, DirectX::XMFLOAT4(nodeAnim->mRotationKeys[j].mValue.x, nodeAnim->mRotationKeys[j].mValue.y, nodeAnim->mRotationKeys[j].mValue.z, nodeAnim->mRotationKeys[j].mValue.w));
		}

		for (UINT j = 0; j < nodeAnim->mNumScalingKeys; ++j) {
			boneAnim.scalingKey.emplace_back(nodeAnim->mScalingKeys[j].mTime, DirectX::XMFLOAT3(nodeAnim->mScalingKeys[j].mValue.x, nodeAnim->mScalingKeys[j].mValue.y, nodeAnim->mScalingKeys[j].mValue.z));
		}

        clip.boneAnimations[boneAnim.boneName] = boneAnim;
    }

    return clip;

}

