#include "pch.h"
#include "Animator.h"

void Animator::ReadNodeHeirarchy(double AnimationTime, BoneNode* node,const SimpleMath::Matrix& ParentTransform,const AnimationClip& animation, std::vector<SimpleMath::Matrix>& boneTransforms) {
    if (!node) return;

    SimpleMath::Matrix nodeTransform{ node->transformation };

    auto animIt = animation.boneAnimations.find(node->name);
    if (animIt != animation.boneAnimations.end()) {
        const BoneAnimation& boneAnim = animIt->second;

        //DirectX::XMFLOAT3 position = InterpolatePosition(AnimationTime, boneAnim);
        //DirectX::XMFLOAT4 rotation = InterpolateRotation(AnimationTime, boneAnim);
        //DirectX::XMFLOAT3 scaling = InterpolateScaling(AnimationTime, boneAnim);

        //DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scaling.x, scaling.y, scaling.z);
        //DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));
        //DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

        //nodeTransform = scaleMatrix * rotationMatrix * translationMatrix;
    }

    SimpleMath::Matrix globalTransform = ParentTransform * nodeTransform;

	UINT index = animation.boneIndexMap.at(node->name);

	boneTransforms[index] = animation.globalInverseTransform * globalTransform * animation.boneOffsetMatrices[index];

    for (auto& child : node->children) {
        ReadNodeHeirarchy(AnimationTime, child.get(), globalTransform, animation, boneTransforms);
    }
}
