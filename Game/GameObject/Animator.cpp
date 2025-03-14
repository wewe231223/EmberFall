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
	animationTime = std::clamp(animationTime, 0.0, static_cast<double>(mClip->duration));

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

namespace AnimatorGraph {

    Animator::Animator(const std::vector<const AnimationClip*>& clips) : mClips(clips) {}

    void Animator::UpdateBoneTransform(double deltaTime, BoneTransformBuffer& boneTransforms) {
        const AnimationClip* clip = mClips[mCurrentClipIndex];
        if (mTransitioning) {
            double blendFactor = mTransitionTime / mTransitionDuration;

            double tick = mCurrentTime * clip->ticksPerSecond;
            double normTime = std::fmod(tick / clip->duration, 1.0);
            double currentAnimTime = normTime * clip->duration;

            ReadNodeHeirarchyTransition(currentAnimTime, 0.0, blendFactor, clip->root.get(), DirectX::SimpleMath::Matrix::Identity, boneTransforms.boneTransforms);
            boneTransforms.boneCount = static_cast<UINT>(clip->boneOffsetMatrices.size());
            mTransitionTime += deltaTime;

            if (mTransitionTime >= mTransitionDuration) {
                mTransitioning = false;
                mCurrentClipIndex = mTargetClipIndex;
                mCurrentTime = 0.0;
            }
        }
        else {
            mCurrentTime += deltaTime;

            double tick = mCurrentTime * clip->ticksPerSecond;
            double normTime = std::fmod(tick / clip->duration, 1.0);
            double animationTime = normTime * clip->duration;

            if (normTime >= 0.99) {
                TransitionToClip(0);
            }
            ComputeBoneTransforms(clip, animationTime, boneTransforms);
        }
    }

    void Animator::TransitionToClip(size_t clipIndex) {
        if (clipIndex < mClips.size() && clipIndex != mCurrentClipIndex) {
            mTargetClipIndex = clipIndex;
            mTransitioning = true;
            mTransitionTime = 0.0;
        }
    }

    void Animator::ComputeBoneTransforms(const AnimationClip* clip, double animationTime, BoneTransformBuffer& boneTransforms) {
        DirectX::SimpleMath::Matrix identity = DirectX::SimpleMath::Matrix::Identity;
        ReadNodeHeirarchy(animationTime, clip->root.get(), identity, boneTransforms.boneTransforms, clip);
        boneTransforms.boneCount = static_cast<UINT>(clip->boneOffsetMatrices.size());
    }

    void Animator::ReadNodeHeirarchy(double AnimationTime, BoneNode* node, const DirectX::SimpleMath::Matrix& ParentTransform, std::array<DirectX::SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms, const AnimationClip* clip) {
        if (!node) return;
        DirectX::SimpleMath::Matrix nodeTransform = node->transformation;

        auto anim = clip->boneAnimations.find(node->index);
        if (anim != clip->boneAnimations.end()) {
            const BoneAnimation& boneAnim = anim->second;
            DirectX::SimpleMath::Vector3 pos = InterpolatePosition(AnimationTime, boneAnim);
            DirectX::SimpleMath::Quaternion rot = InterpolateRotation(AnimationTime, boneAnim);
            DirectX::SimpleMath::Vector3 scale = InterpolateScaling(AnimationTime, boneAnim);
            nodeTransform = DirectX::SimpleMath::Matrix::CreateScale(scale) * DirectX::SimpleMath::Matrix::CreateFromQuaternion(rot) * DirectX::SimpleMath::Matrix::CreateTranslation(pos);
        }

        DirectX::SimpleMath::Matrix globalTransform = nodeTransform * ParentTransform;

        if (node->index != std::numeric_limits<UINT>::max()) {
            auto result = clip->boneOffsetMatrices[node->index] * globalTransform * clip->globalInverseTransform;
            boneTransforms[node->index] = result.Transpose();
        }
        for (auto& child : node->children) {
            ReadNodeHeirarchy(AnimationTime, child.get(), globalTransform, boneTransforms, clip);
        }
    }

    void Animator::ReadNodeHeirarchyTransition(double currentAnimTime, double targetAnimTime, double blendFactor, BoneNode* node, const DirectX::SimpleMath::Matrix& ParentTransform, std::array<DirectX::SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms) {
        if (!node) return;
        TransformComponents currentComp{};
        TransformComponents targetComp{};

        auto current = mClips[mCurrentClipIndex]->boneAnimations.find(node->index);
        if (current != mClips[mCurrentClipIndex]->boneAnimations.end()) {
            const BoneAnimation& currentBoneAnim = current->second;
            currentComp.translation = InterpolatePosition(currentAnimTime, currentBoneAnim);
            currentComp.rotation = InterpolateRotation(currentAnimTime, currentBoneAnim);
            currentComp.scale = InterpolateScaling(currentAnimTime, currentBoneAnim);
        }
        else {
            DecomposeMatrix(node->transformation, currentComp);
        }

        auto target = mClips[mTargetClipIndex]->boneAnimations.find(node->index);
        if (target != mClips[mTargetClipIndex]->boneAnimations.end()) {
            const BoneAnimation& targetBoneAnim = target->second;
            targetComp.translation = InterpolatePosition(targetAnimTime, targetBoneAnim);
            targetComp.rotation = InterpolateRotation(targetAnimTime, targetBoneAnim);
            targetComp.scale = InterpolateScaling(targetAnimTime, targetBoneAnim);
        }
        else {
            DecomposeMatrix(node->transformation, targetComp);
        }

        DirectX::SimpleMath::Vector3 blendedTranslation = DirectX::SimpleMath::Vector3::Lerp(currentComp.translation, targetComp.translation, static_cast<float>(blendFactor));
        DirectX::SimpleMath::Quaternion blendedRotation = DirectX::SimpleMath::Quaternion::Slerp(currentComp.rotation, targetComp.rotation, static_cast<float>(blendFactor));
        DirectX::SimpleMath::Vector3 blendedScale = DirectX::SimpleMath::Vector3::Lerp(currentComp.scale, targetComp.scale, static_cast<float>(blendFactor));

        DirectX::SimpleMath::Matrix blendednodeTransform = DirectX::SimpleMath::Matrix::CreateScale(blendedScale) * DirectX::SimpleMath::Matrix::CreateFromQuaternion(blendedRotation) * DirectX::SimpleMath::Matrix::CreateTranslation(blendedTranslation);
        DirectX::SimpleMath::Matrix globalTransform = blendednodeTransform * ParentTransform;

        if (node->index != std::numeric_limits<UINT>::max()) {
            auto result = mClips[mCurrentClipIndex]->boneOffsetMatrices[node->index] * globalTransform * mClips[mCurrentClipIndex]->globalInverseTransform;
            boneTransforms[node->index] = result.Transpose();
        }

        for (auto& child : node->children) {
            ReadNodeHeirarchyTransition(currentAnimTime, targetAnimTime, blendFactor, child.get(), globalTransform, boneTransforms);
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

    DirectX::SimpleMath::Vector3 Animator::InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim) {
        if (boneAnim.positionKey.size() == 1) {
            return boneAnim.positionKey[0].second;
        }

        UINT posIndex = FindPosition(AnimationTime, boneAnim);
        UINT nextIndex = posIndex + 1;

        float deltaTime = static_cast<float>(boneAnim.positionKey[nextIndex].first - boneAnim.positionKey[posIndex].first);
        float factor = static_cast<float>(AnimationTime - boneAnim.positionKey[posIndex].first) / deltaTime;

        DirectX::SimpleMath::Vector3 start = boneAnim.positionKey[posIndex].second;
        DirectX::SimpleMath::Vector3 delta = boneAnim.positionKey[nextIndex].second - start;

        return start + factor * delta;
    }

    DirectX::SimpleMath::Quaternion Animator::InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim) {
        if (boneAnim.rotationKey.size() == 1) {
            return boneAnim.rotationKey[0].second;
        }

        UINT rotIndex = FindRotation(AnimationTime, boneAnim);
        UINT nextIndex = rotIndex + 1;

        float deltaTime = static_cast<float>(boneAnim.rotationKey[nextIndex].first - boneAnim.rotationKey[rotIndex].first);
        float factor = static_cast<float>(AnimationTime - boneAnim.rotationKey[rotIndex].first) / deltaTime;

        DirectX::SimpleMath::Quaternion start = boneAnim.rotationKey[rotIndex].second;
        DirectX::SimpleMath::Quaternion end = boneAnim.rotationKey[nextIndex].second;

        return DirectX::SimpleMath::Quaternion::Slerp(start, end, factor);
    }

    DirectX::SimpleMath::Vector3 Animator::InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim) {
        if (boneAnim.scalingKey.size() == 1) {
            return boneAnim.scalingKey[0].second;
        }

        UINT scaleIndex = FindScaling(AnimationTime, boneAnim);
        UINT nextIndex = scaleIndex + 1;

        float deltaTime = static_cast<float>(boneAnim.scalingKey[nextIndex].first - boneAnim.scalingKey[scaleIndex].first);
        float factor = static_cast<float>(AnimationTime - boneAnim.scalingKey[scaleIndex].first) / deltaTime;

        DirectX::SimpleMath::Vector3 start = boneAnim.scalingKey[scaleIndex].second;
        DirectX::SimpleMath::Vector3 delta = boneAnim.scalingKey[nextIndex].second - start;

        return start + factor * delta;
    }

}