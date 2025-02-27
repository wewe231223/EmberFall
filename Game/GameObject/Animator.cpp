#include "pch.h"
#include "Animator.h"

void Animator::ReadNodeHeirarchy(double AnimationTime, BoneNode* node, const DirectX::XMFLOAT4X4& ParentTransform, const AnimationClip& animation, std::vector<DirectX::XMFLOAT4X4>& boneTransforms) {
   /* if (!node) return;

    DirectX::XMMATRIX nodeTransform = DirectX::XMLoadFloat4x4(&node->transformation);

    auto animIt = animation.boneAnimations.find(node->name);
    if (animIt != animation.boneAnimations.end()) {
        const BoneAnimation& boneAnim = animIt->second;
        DirectX::XMFLOAT3 position = InterpolatePosition(AnimationTime, boneAnim);
        DirectX::XMFLOAT4 rotation = InterpolateRotation(AnimationTime, boneAnim);
        DirectX::XMFLOAT3 scaling = InterpolateScaling(AnimationTime, boneAnim);

        DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(scaling.x, scaling.y, scaling.z);
        DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(XMLoadFloat4(&rotation));
        DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

        nodeTransform = scaleMatrix * rotationMatrix * translationMatrix;
    }

    DirectX::XMMATRIX globalTransform = DirectX::XMLoadFloat4x4(&ParentTransform) * nodeTransform;
    DirectX::XMStoreFloat4x4(&boneTransforms[std::distance(animation.hierarchy.begin(), animation.hierarchy.find(node->name))], globalTransform);

    for (BoneNode* child : node->children) {
        ReadNodeHeirarchy(AnimationTime, child, globalTransform, animation, boneTransforms);
    }*/
}
