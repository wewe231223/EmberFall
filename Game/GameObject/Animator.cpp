#include "pch.h"
#include "Animator.h"
#include "../EditorInterface/Console/Console.h"
#include "../Utility/Defines.h"
#include <ranges>

#ifdef max 
#undef max
#endif

#pragma region Legacy 
namespace Legacy {
    void Animator::UpdateBoneTransform(double time, std::vector<SimpleMath::Matrix>& boneTransforms) {
        DirectX::SimpleMath::Matrix identity{ DirectX::SimpleMath::Matrix::Identity };

        double tick{ time * mClip.ticksPerSecond };
        double animationTime{ std::fmod(tick, mClip.duration) };

        boneTransforms.resize(mClip.boneOffsetMatrices.size(), DirectX::SimpleMath::Matrix::Identity);

        Animator::ReadNodeHeirarchy(animationTime, mClip.root.get(), identity, mClip, boneTransforms);
    }

    void Animator::ReadNodeHeirarchy(double AnimationTime, BoneNode* node, const SimpleMath::Matrix& ParentTransform, const AnimationClip& animation, std::vector<SimpleMath::Matrix>& boneTransforms) {
        if (!node) return;

        SimpleMath::Matrix nodeTransform{ node->transformation };

        auto animIt = animation.boneAnimations.find(node->name);
        if (animIt != animation.boneAnimations.end()) {
            const BoneAnimation& boneAnim = animIt->second;

            SimpleMath::Vector3 position = InterpolatePosition(AnimationTime, boneAnim);
            SimpleMath::Quaternion rotation = InterpolateRotation(AnimationTime, boneAnim);
            SimpleMath::Vector3 scaling = InterpolateScaling(AnimationTime, boneAnim);

            SimpleMath::Matrix translate{ SimpleMath::Matrix::CreateTranslation(position) };
            SimpleMath::Matrix rotate{ SimpleMath::Matrix::CreateFromQuaternion(rotation) };
            SimpleMath::Matrix scale{ SimpleMath::Matrix::CreateScale(scaling) };

            nodeTransform = (scale * rotate * translate);
        }

        SimpleMath::Matrix globalTransform = nodeTransform * ParentTransform;


        if (animation.boneIndexMap.find(node->name) != animation.boneIndexMap.end()) {
            UINT index = animation.boneIndexMap.at(node->name);
            auto result = animation.boneOffsetMatrices[index] * globalTransform * animation.globalInverseTransform;
            boneTransforms[index] = result.Transpose();
        }

        for (auto& child : node->children) {
            ReadNodeHeirarchy(AnimationTime, child.get(), globalTransform, animation, boneTransforms);
        }
    }




    UINT Animator::FindPosition(double AnimationTime, const BoneAnimation& boneAnim) {
        for (UINT i = 0; i < boneAnim.positionKey.size() - 1; i++) {
            if (AnimationTime < boneAnim.positionKey[i + 1].first) {
                return i;
            }
        }

        Console.Log("Error - Animator::FindPosition ", LogType::Error);
        Crash(false);
        return 0;
    }

    UINT Animator::FindRotation(double AnimationTime, const BoneAnimation& boneAnim) {
        for (UINT i = 0; i < boneAnim.rotationKey.size() - 1; i++) {
            if (AnimationTime < boneAnim.rotationKey[i + 1].first) {
                return i;
            }
        }
        Console.Log("Error - Animator::FindRotation ", LogType::Error);
        Crash(false);
        return 0;
    }

    UINT Animator::FindScaling(double AnimationTime, const BoneAnimation& boneAnim) {
        for (UINT i = 0; i < boneAnim.scalingKey.size() - 1; i++) {
            if (AnimationTime < boneAnim.scalingKey[i + 1].first) {
                return i;
            }
        }
        Console.Log("Error - Animator::FindScaling ", LogType::Error);
        Crash(false);
        return 0;
    }



    SimpleMath::Vector3 Animator::InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim) {
        if (boneAnim.positionKey.size() == 1) {
            return boneAnim.positionKey[0].second;
        }

        UINT positionIndex = FindPosition(AnimationTime, boneAnim);
        UINT nextPositionIndex = (positionIndex + 1);

        float deltaTime = static_cast<float>(boneAnim.positionKey[nextPositionIndex].first - boneAnim.positionKey[positionIndex].first);
        float factor = static_cast<float>(AnimationTime - boneAnim.positionKey[positionIndex].first) / deltaTime;

        SimpleMath::Vector3 start = boneAnim.positionKey[positionIndex].second;
        SimpleMath::Vector3 delta = boneAnim.positionKey[nextPositionIndex].second - boneAnim.positionKey[positionIndex].second;


        return start + factor * delta;
    }

    SimpleMath::Quaternion Animator::InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim) {
        if (boneAnim.rotationKey.size() == 1) {
            return boneAnim.rotationKey[0].second;
        }

        UINT rotationIndex = FindRotation(AnimationTime, boneAnim);
        UINT nextRotationIndex = (rotationIndex + 1);

        float deltaTime = static_cast<float>(boneAnim.rotationKey[nextRotationIndex].first - boneAnim.rotationKey[rotationIndex].first);
        float factor = static_cast<float>(AnimationTime - boneAnim.rotationKey[rotationIndex].first) / deltaTime;

        SimpleMath::Quaternion start = boneAnim.rotationKey[rotationIndex].second;
        SimpleMath::Quaternion end = boneAnim.rotationKey[nextRotationIndex].second;

        return SimpleMath::Quaternion::Slerp(start, end, factor);
    }

    SimpleMath::Vector3 Animator::InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim) {
        if (boneAnim.scalingKey.size() == 1) {
            return boneAnim.scalingKey[0].second;
        }

        UINT scalingIndex = FindScaling(AnimationTime, boneAnim);
        UINT nextScalingIndex = (scalingIndex + 1);

        float deltaTime = static_cast<float>(boneAnim.scalingKey[nextScalingIndex].first - boneAnim.scalingKey[scalingIndex].first);
        float factor = static_cast<float>(AnimationTime - boneAnim.scalingKey[scalingIndex].first) / deltaTime;

        SimpleMath::Vector3 start = boneAnim.scalingKey[scalingIndex].second;
        SimpleMath::Vector3 delta = boneAnim.scalingKey[nextScalingIndex].second - boneAnim.scalingKey[scalingIndex].second;

        return start + factor * delta;
    }
}
#pragma endregion

bool Animator::GetActivated() const {
    return mClip != nullptr;
}

void Animator::UpdateBoneTransform(double time, BoneTransformBuffer& boneTransforms) {
    DirectX::SimpleMath::Matrix identity{ DirectX::SimpleMath::Matrix::Identity };

    double tick{ time * mClip->ticksPerSecond };
    double animationTime{ std::fmod(tick, mClip->duration) };
	animationTime = std::clamp(tick, 0.0, static_cast<double>(mClip->duration));

	std::fill(std::begin(boneTransforms.boneTransforms), std::end(boneTransforms.boneTransforms), DirectX::SimpleMath::Matrix::Identity);

    Animator::ReadNodeHeirarchy(animationTime, mClip->root.get(), identity, boneTransforms.boneTransforms);
	boneTransforms.boneCount = static_cast<UINT>(mClip->boneOffsetMatrices.size());
}

void Animator::ReadNodeHeirarchy(double AnimationTime, BoneNode* node, const SimpleMath::Matrix& ParentTransform, std::array<SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms) {
    if (!node) return;

    SimpleMath::Matrix nodeTransform{ node->transformation };

    auto animIt = mClip->boneAnimations.find(node->index);
    if (animIt != mClip->boneAnimations.end()) {
        const BoneAnimation& boneAnim = animIt->second;

        SimpleMath::Vector3 position = InterpolatePosition(AnimationTime, boneAnim);
        SimpleMath::Quaternion rotation = InterpolateRotation(AnimationTime, boneAnim);
        SimpleMath::Vector3 scaling = InterpolateScaling(AnimationTime, boneAnim);

        SimpleMath::Matrix translate{ SimpleMath::Matrix::CreateTranslation(position) };
        SimpleMath::Matrix rotate{ SimpleMath::Matrix::CreateFromQuaternion(rotation) };
        SimpleMath::Matrix scale{ SimpleMath::Matrix::CreateScale(scaling) };

        nodeTransform = (scale * rotate * translate);
    }

    SimpleMath::Matrix globalTransform = nodeTransform * ParentTransform;

    if (node->index != std::numeric_limits<UINT>::max()) {
        auto result = mClip->boneOffsetMatrices[node->index] * globalTransform * mClip->globalInverseTransform;
        boneTransforms[node->index] = result.Transpose();
    }

    for (auto& child : node->children) {
        ReadNodeHeirarchy(AnimationTime, child.get(), globalTransform, boneTransforms);
    }
}

UINT Animator::FindPosition(double AnimationTime, const BoneAnimation& boneAnim) {
    for (UINT i = 0; i < boneAnim.positionKey.size() - 1; i++) {
        if (AnimationTime < boneAnim.positionKey[i + 1].first) {
            return i;
        }
    }
    return static_cast<UINT>(boneAnim.positionKey.size() - 2);
}

UINT Animator::FindRotation(double AnimationTime, const BoneAnimation& boneAnim) {
    for (UINT i = 0; i < boneAnim.rotationKey.size() - 1; i++) {
        if (AnimationTime < boneAnim.rotationKey[i + 1].first) {
            return i;
        }
    }

    return static_cast<UINT>(boneAnim.rotationKey.size() - 2);
}

UINT Animator::FindScaling(double AnimationTime, const BoneAnimation& boneAnim) {
    for (UINT i = 0; i < boneAnim.scalingKey.size() - 1; i++) {
        if (AnimationTime < boneAnim.scalingKey[i + 1].first) {
            return i;
        }
    }

    return static_cast<UINT>(boneAnim.scalingKey.size() - 2);
}

SimpleMath::Vector3 Animator::InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim) {
    if (boneAnim.positionKey.size() == 1) {
        return boneAnim.positionKey[0].second;
    }

    UINT positionIndex = FindPosition(AnimationTime, boneAnim);
    UINT nextPositionIndex = (positionIndex + 1);

    float deltaTime = static_cast<float>(boneAnim.positionKey[nextPositionIndex].first - boneAnim.positionKey[positionIndex].first);
    float factor = static_cast<float>(AnimationTime - boneAnim.positionKey[positionIndex].first) / deltaTime;

    SimpleMath::Vector3 start = boneAnim.positionKey[positionIndex].second;
    SimpleMath::Vector3 delta = boneAnim.positionKey[nextPositionIndex].second - boneAnim.positionKey[positionIndex].second;

    return start + factor * delta;
}

SimpleMath::Quaternion Animator::InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim) {
    if (boneAnim.rotationKey.size() == 1) {
        return boneAnim.rotationKey[0].second;
    }

    UINT rotationIndex = FindRotation(AnimationTime, boneAnim);
    UINT nextRotationIndex = (rotationIndex + 1);

    float deltaTime = static_cast<float>(boneAnim.rotationKey[nextRotationIndex].first - boneAnim.rotationKey[rotationIndex].first);
    float factor = static_cast<float>(AnimationTime - boneAnim.rotationKey[rotationIndex].first) / deltaTime;

    SimpleMath::Quaternion start = boneAnim.rotationKey[rotationIndex].second;
    SimpleMath::Quaternion end = boneAnim.rotationKey[nextRotationIndex].second;

    return SimpleMath::Quaternion::Slerp(start, end, factor);
}

SimpleMath::Vector3 Animator::InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim) {
    if (boneAnim.scalingKey.size() == 1) {
        return boneAnim.scalingKey[0].second;
    }

    UINT scalingIndex = FindScaling(AnimationTime, boneAnim);
    UINT nextScalingIndex = (scalingIndex + 1);

    float deltaTime = static_cast<float>(boneAnim.scalingKey[nextScalingIndex].first - boneAnim.scalingKey[scalingIndex].first);
    float factor = static_cast<float>(AnimationTime - boneAnim.scalingKey[scalingIndex].first) / deltaTime;

    SimpleMath::Vector3 start = boneAnim.scalingKey[scalingIndex].second;
    SimpleMath::Vector3 delta = boneAnim.scalingKey[nextScalingIndex].second - boneAnim.scalingKey[scalingIndex].second;

    return start + factor * delta;
}

