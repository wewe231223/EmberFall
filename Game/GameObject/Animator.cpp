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

    void Animator::SetTransitionDuration(double duration) {
		mTransitionDuration = duration;
    }

    double Animator::GetNormalizedTime() {
		const AnimationClip* clip = mClips[mCurrentClipIndex];
        double tick = mCurrentTime * clip->ticksPerSecond;
        double normTime = std::fmod(tick / clip->duration, 1.0);

        return normTime; 
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

    AnimationParameter::AnimationParameter(const std::string& name, ParameterType type) : name(name), type(type) {
        switch (type) {
        case ParameterType::Bool:
        case ParameterType::Trigger:
            boolValue = false;
            break;
        case ParameterType::Int:
            intValue = 0;
            break;
        case ParameterType::Float:
            floatValue = 0.0f;
            break;
        }
    }

    AnimationParameter::AnimationParameter() : name(""), type(ParameterType::Bool), boolValue(false), intValue(0), floatValue(0.0f) {}

    AnimationGraphController::AnimationGraphController(const std::vector<AnimationState>& states) : mStates(states), mCurrentStateIndex(0) {
        std::vector<const AnimationClip*> clips;
        for (const auto& state : mStates) {
            clips.push_back(state.clip);
        }
        mAnimator = Animator(clips);
    }

    void AnimationGraphController::Update(double deltaTime, BoneTransformBuffer& boneTransforms) {
        EvaluateTransitions();
        mAnimator.UpdateBoneTransform(deltaTime, boneTransforms);
    }

    void AnimationGraphController::AddParameter(const std::string& name, ParameterType type) {
        mParameters[name] = AnimationParameter(name, type);
    }

    void AnimationGraphController::SetBool(const std::string& name, bool value) {
        auto it = mParameters.find(name);
        if (it != mParameters.end() && (it->second.type == ParameterType::Bool || it->second.type == ParameterType::Trigger)) {
            it->second.boolValue = value;
        }
    }

    void AnimationGraphController::SetInt(const std::string& name, int value) {
        auto it = mParameters.find(name);
        if (it != mParameters.end() && it->second.type == ParameterType::Int) {
            it->second.intValue = value;
        }
    }

    void AnimationGraphController::SetFloat(const std::string& name, float value) {
        auto it = mParameters.find(name);
        if (it != mParameters.end() && it->second.type == ParameterType::Float) {
            it->second.floatValue = value;
        }
    }

    void AnimationGraphController::SetTrigger(const std::string& name) {
        auto it = mParameters.find(name);
        if (it != mParameters.end() && it->second.type == ParameterType::Trigger) {
            it->second.boolValue = true;
        }
    }

    void AnimationGraphController::ResetTrigger(const std::string& name) {
        auto it = mParameters.find(name);
        if (it != mParameters.end() && it->second.type == ParameterType::Trigger) {
            it->second.boolValue = false;
        }
    }

    const AnimationParameter* AnimationGraphController::GetParameter(const std::string& name) const {
        auto it = mParameters.find(name);
        if (it != mParameters.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    void AnimationGraphController::EvaluateTransitions() {
        const AnimationState& currentState = mStates[mCurrentStateIndex];
        for (const auto& transition : currentState.transitions) {
            auto paramIt = mParameters.find(transition.parameterName);
            if (paramIt == mParameters.end())
                continue;
            const AnimationParameter& param = paramIt->second;
            bool conditionMet = false;
            switch (param.type) {
            case ParameterType::Bool: {
                bool expected = std::get<bool>(transition.expectedValue);
                if (param.boolValue == expected) {
                    conditionMet = true;
                }
                break;
            }
            case ParameterType::Int: {
                int expected = std::get<int>(transition.expectedValue);
                if (param.intValue == expected) {
                    conditionMet = true;
                }
                break;
            }
            case ParameterType::Float: {
                float expected = std::get<float>(transition.expectedValue);
                if (std::fabs(param.floatValue - expected) < 0.001f) {
                    conditionMet = true;
                }
                break;
            }
            case ParameterType::Trigger: {
                bool expected = std::get<bool>(transition.expectedValue);
                if (param.boolValue == expected && param.boolValue == true) {
                    conditionMet = true;
                    const_cast<AnimationParameter&>(param).boolValue = false;
                }
                break;
            }
			case ParameterType::Always: {
				conditionMet = true;
				break;
            }
            default:
                break;
            }

            if (transition.triggerOnEnd) {
                if (mAnimator.GetNormalizedTime() < 0.99) {
                    conditionMet = false;
                }
            }

            if (conditionMet) {
                size_t targetState = transition.targetStateIndex;
                if (targetState < mStates.size() && targetState != mCurrentStateIndex) {
					mAnimator.SetTransitionDuration(transition.blendDuration);
                    mAnimator.TransitionToClip(targetState);
                    mCurrentStateIndex = targetState;
                    break;
                }
            }
        }
    }

    BoneMaskAnimator::BoneMaskAnimator() = default;

    BoneMaskAnimator::BoneMaskAnimator(const std::vector<const AnimationClip*>& clips, const std::vector<unsigned int>& boneMask) : mClips(clips) {
        mDefaultClip = mClips[0];
        mClipMasked = mDefaultClip;
        mClipNonMasked = mDefaultClip;

        mTransitioningMasked = false;
        mTransitioningNonMasked = false;
        mTransitionTimeMaskedTransition = 0.0;
        mTransitionTimeNonMaskedTransition = 0.0;

        for (unsigned int id : boneMask) {
            mBoneMask.insert(id);
        }

        mRootNode = mDefaultClip->root;
    }

    void BoneMaskAnimator::UpdateBoneTransforms(double deltaTime, BoneTransformBuffer& boneTransforms) {
        if (mTransitioningMasked || mTransitioningNonMasked) {

            double blendFactorMasked = mTransitioningMasked ? (mTransitionTimeMaskedTransition / mTransitionDuration) : 0.0;
            double tickMasked = mCurrentTimeMasked * mClipMasked->ticksPerSecond;
            double normTimeMasked = std::fmod(tickMasked / mClipMasked->duration, 1.0);
            double currentAnimTimeMasked = normTimeMasked * mClipMasked->duration;
            double blendFactorNonMasked = mTransitioningNonMasked ? (mTransitionTimeNonMaskedTransition / mTransitionDuration) : 0.0;
            double tickNonMasked = mCurrentTimeNonMasked * mClipNonMasked->ticksPerSecond;
            double normTimeNonMasked = std::fmod(tickNonMasked / mClipNonMasked->duration, 1.0);
            double currentAnimTimeNonMasked = normTimeNonMasked * mClipNonMasked->duration;

            ReadNodeHeirarchyTransition(currentAnimTimeMasked, currentAnimTimeNonMasked, blendFactorMasked, blendFactorNonMasked, mRootNode.get(), DirectX::SimpleMath::Matrix::Identity, boneTransforms);
            boneTransforms.boneCount = static_cast<unsigned int>(mDefaultClip->boneOffsetMatrices.size());

            if (mTransitioningMasked) {
                mTransitionTimeMaskedTransition += deltaTime;
                if (mTransitionTimeMaskedTransition >= mTransitionDuration) {
                    mTransitioningMasked = false;
                    mClipMasked = mTargetClipMasked;
                    mCurrentTimeMasked = 0.0;
                    mTransitionTimeMaskedTransition = 0.0;
                }
            }
            if (mTransitioningNonMasked) {
                mTransitionTimeNonMaskedTransition += deltaTime;
                if (mTransitionTimeNonMaskedTransition >= mTransitionDuration) {
                    mTransitioningNonMasked = false;
                    mClipNonMasked = mTargetClipNonMasked;
                    mCurrentTimeNonMasked = 0.0;
                    mTransitionTimeNonMaskedTransition = 0.0;
                }
            }
        }
        else {
            mCurrentTimeMasked += deltaTime;
            mCurrentTimeNonMasked += deltaTime;

            double tickMasked = mCurrentTimeMasked * mClipMasked->ticksPerSecond;
            double normTimeMasked = std::fmod(tickMasked / mClipMasked->duration, 1.0);
            double animationTimeMasked = normTimeMasked * mClipMasked->duration;
            double tickNonMasked = mCurrentTimeNonMasked * mClipNonMasked->ticksPerSecond;
            double normTimeNonMasked = std::fmod(tickNonMasked / mClipNonMasked->duration, 1.0);
            double animationTimeNonMasked = normTimeNonMasked * mClipNonMasked->duration;

            ComputeBoneTransforms(animationTimeMasked, animationTimeNonMasked, boneTransforms);

            boneTransforms.boneCount = static_cast<unsigned int>(mDefaultClip->boneOffsetMatrices.size());
        }
    }

    void BoneMaskAnimator::TransitionMaskedToClip(size_t clipIndex) {
        if (clipIndex < mClips.size() && mClipMasked != mClips[clipIndex]) {
            mTargetClipMasked = mClips[clipIndex];
            mTransitioningMasked = true;
            mTransitionTimeMaskedTransition = 0.0;
        }
    }

    void BoneMaskAnimator::TransitionNonMaskedToClip(size_t clipIndex) {
        if (clipIndex < mClips.size() && mClipNonMasked != mClips[clipIndex]) {
            mTargetClipNonMasked = mClips[clipIndex];
            mTransitioningNonMasked = true;
            mTransitionTimeNonMaskedTransition = 0.0;
        }
    }

    void BoneMaskAnimator::SetTransitionDuration(double duration) { 
        mTransitionDuration = duration; 
    }

    double BoneMaskAnimator::GetNormalizedTimeNonMasked() {
        double tickNonMasked = mCurrentTimeNonMasked * mClipNonMasked->ticksPerSecond;
        double normTimeNonMasked = std::fmod(tickNonMasked / mClipNonMasked->duration, 1.0);
        return normTimeNonMasked;
    }

    double BoneMaskAnimator::GetNormalizedTimeMasked() {
        double tickMasked = mCurrentTimeMasked * mClipMasked->ticksPerSecond;
        double normTimeMasked = std::fmod(tickMasked / mClipMasked->duration, 1.0);
        return normTimeMasked;
    }

    void BoneMaskAnimator::ComputeBoneTransforms(double animTimeMasked, double animTimeNonMasked, BoneTransformBuffer& boneTransforms) {
        ReadNodeHeirarchy(animTimeMasked, animTimeNonMasked, mRootNode.get(), DirectX::SimpleMath::Matrix::Identity, boneTransforms);
    }

    void BoneMaskAnimator::ReadNodeHeirarchyTransition(double currentAnimTimeMasked, double currentAnimTimeNonMasked, double blendFactorMasked, double blendFactorNonMasked, BoneNode* node, const DirectX::SimpleMath::Matrix& parentTransform, BoneTransformBuffer& boneTransforms) {
        if (!node) return;

        bool useMasked = (mBoneMask.find(node->index) != mBoneMask.end());
        const AnimationClip* currentClip = useMasked ? mClipMasked : mClipNonMasked;
        const AnimationClip* targetClip = useMasked ? mTargetClipMasked : mTargetClipNonMasked;

        double animTime = useMasked ? currentAnimTimeMasked : currentAnimTimeNonMasked;
        double blendFactor = useMasked ? blendFactorMasked : blendFactorNonMasked;

        TransformComponents currentComp;
        auto itCurrent = currentClip->boneAnimations.find(node->index);
        if (itCurrent != currentClip->boneAnimations.end()) {
            const BoneAnimation& currentBoneAnim = itCurrent->second;
            currentComp.translation = InterpolatePosition(animTime, currentBoneAnim);
            currentComp.rotation = InterpolateRotation(animTime, currentBoneAnim);
            currentComp.scale = InterpolateScaling(animTime, currentBoneAnim);
        }
        else {
            DecomposeMatrix(node->transformation, currentComp);
        }

        TransformComponents targetComp;

        auto itTarget = targetClip->boneAnimations.find(node->index);
        if (itTarget != targetClip->boneAnimations.end()) {
            const BoneAnimation& targetBoneAnim = itTarget->second;
            double normTime = animTime / currentClip->duration;
            double targetAnimTime = 0.0;
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
        DirectX::SimpleMath::Matrix blendedNodeTransform = DirectX::SimpleMath::Matrix::CreateScale(blendedScale) * DirectX::SimpleMath::Matrix::CreateFromQuaternion(blendedRotation) * DirectX::SimpleMath::Matrix::CreateTranslation(blendedTranslation);
        DirectX::SimpleMath::Matrix globalTransform = blendedNodeTransform * parentTransform;
        
        if (node->index != std::numeric_limits<unsigned int>::max()) {
            auto result = currentClip->boneOffsetMatrices[node->index] * globalTransform * currentClip->globalInverseTransform;
            boneTransforms.boneTransforms[node->index] = result.Transpose();
        }

        for (auto& child : node->children) {
            ReadNodeHeirarchyTransition(currentAnimTimeMasked, currentAnimTimeNonMasked, blendFactorMasked, blendFactorNonMasked, child.get(), globalTransform, boneTransforms);
        }
    }

    void BoneMaskAnimator::ReadNodeHeirarchy(double animTimeMasked, double animTimeNonMasked, BoneNode* node, const DirectX::SimpleMath::Matrix& parentTransform, BoneTransformBuffer& boneTransforms) {
        if (!node) return;

        bool useMasked = (mBoneMask.find(node->index) != mBoneMask.end());
        const AnimationClip* clip = useMasked ? mClipMasked : mClipNonMasked;
        double animTime = useMasked ? animTimeMasked : animTimeNonMasked;
        DirectX::SimpleMath::Matrix nodeTransform = node->transformation;
        auto it = clip->boneAnimations.find(node->index);

        if (it != clip->boneAnimations.end()) {
            const BoneAnimation& boneAnim = it->second;
            DirectX::SimpleMath::Vector3 pos = InterpolatePosition(animTime, boneAnim);
            DirectX::SimpleMath::Quaternion rot = InterpolateRotation(animTime, boneAnim);
            DirectX::SimpleMath::Vector3 scale = InterpolateScaling(animTime, boneAnim);
            nodeTransform = DirectX::SimpleMath::Matrix::CreateScale(scale) * DirectX::SimpleMath::Matrix::CreateFromQuaternion(rot) * DirectX::SimpleMath::Matrix::CreateTranslation(pos);
        }

        DirectX::SimpleMath::Matrix globalTransform = nodeTransform * parentTransform;

        if (node->index != std::numeric_limits<unsigned int>::max()) {
            auto result = clip->boneOffsetMatrices[node->index] * globalTransform * clip->globalInverseTransform;
            boneTransforms.boneTransforms[node->index] = result.Transpose();
        }

        for (auto& child : node->children) {
            ReadNodeHeirarchy(animTimeMasked, animTimeNonMasked, child.get(), globalTransform, boneTransforms);
        }
    }

    unsigned int BoneMaskAnimator::FindPosition(double AnimationTime, const BoneAnimation& boneAnim) {
        for (unsigned int i = 0; i < boneAnim.positionKey.size() - 1; i++) {
            if (AnimationTime < boneAnim.positionKey[i + 1].first) {
                return i;
            }
        }
        return static_cast<unsigned int>(boneAnim.positionKey.size() - 2);
    }

    unsigned int BoneMaskAnimator::FindRotation(double AnimationTime, const BoneAnimation& boneAnim) {
        for (unsigned int i = 0; i < boneAnim.rotationKey.size() - 1; i++) {
            if (AnimationTime < boneAnim.rotationKey[i + 1].first) {
                return i;
            }
        }
        return static_cast<unsigned int>(boneAnim.rotationKey.size() - 2);
    }

    unsigned int BoneMaskAnimator::FindScaling(double AnimationTime, const BoneAnimation& boneAnim) {
        for (unsigned int i = 0; i < boneAnim.scalingKey.size() - 1; i++) {
            if (AnimationTime < boneAnim.scalingKey[i + 1].first) {
                return i;
            }
        }
        return static_cast<unsigned int>(boneAnim.scalingKey.size() - 2);
    }

    DirectX::SimpleMath::Vector3 BoneMaskAnimator::InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim) {
        if (boneAnim.positionKey.size() == 1) {
            return boneAnim.positionKey[0].second;
        }
        unsigned int posIndex = FindPosition(AnimationTime, boneAnim);
        unsigned int nextIndex = posIndex + 1;

        float deltaTime = static_cast<float>(boneAnim.positionKey[nextIndex].first - boneAnim.positionKey[posIndex].first);
        float factor = static_cast<float>(AnimationTime - boneAnim.positionKey[posIndex].first) / deltaTime;

        DirectX::SimpleMath::Vector3 start = boneAnim.positionKey[posIndex].second;
        DirectX::SimpleMath::Vector3 delta = boneAnim.positionKey[nextIndex].second - start;

        return start + factor * delta;
    }

    DirectX::SimpleMath::Quaternion BoneMaskAnimator::InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim) {
        if (boneAnim.rotationKey.size() == 1) {
            return boneAnim.rotationKey[0].second;
        }
        unsigned int rotIndex = FindRotation(AnimationTime, boneAnim);
        unsigned int nextIndex = rotIndex + 1;

        float deltaTime = static_cast<float>(boneAnim.rotationKey[nextIndex].first - boneAnim.rotationKey[rotIndex].first);
        float factor = static_cast<float>(AnimationTime - boneAnim.rotationKey[rotIndex].first) / deltaTime;

        DirectX::SimpleMath::Quaternion start = boneAnim.rotationKey[rotIndex].second;
        DirectX::SimpleMath::Quaternion end = boneAnim.rotationKey[nextIndex].second;

        return DirectX::SimpleMath::Quaternion::Slerp(start, end, factor);
    }

    DirectX::SimpleMath::Vector3 BoneMaskAnimator::InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim) {
        if (boneAnim.scalingKey.size() == 1) {
            return boneAnim.scalingKey[0].second;
        }
        unsigned int scaleIndex = FindScaling(AnimationTime, boneAnim);
        unsigned int nextIndex = scaleIndex + 1;

        float deltaTime = static_cast<float>(boneAnim.scalingKey[nextIndex].first - boneAnim.scalingKey[scaleIndex].first);
        float factor = static_cast<float>(AnimationTime - boneAnim.scalingKey[scaleIndex].first) / deltaTime;

        DirectX::SimpleMath::Vector3 start = boneAnim.scalingKey[scaleIndex].second;
        DirectX::SimpleMath::Vector3 delta = boneAnim.scalingKey[nextIndex].second - start;

        return start + factor * delta;
    }

    BoneMaskAnimationGraphController::BoneMaskAnimationGraphController(const std::vector<const AnimationClip*>& clips, const std::vector<UINT>& boneMask, const std::vector<BoneMaskAnimationState>& states) : mAnimator(clips, boneMask), mStates(states), mCurrentStateIndex(0) {
       
    }

    void BoneMaskAnimationGraphController::Update(double deltaTime, BoneTransformBuffer& boneTransforms) {
        EvaluateTransitions();
        mAnimator.UpdateBoneTransforms(deltaTime, boneTransforms);
    }

    void BoneMaskAnimationGraphController::AddParameter(const std::string& name, ParameterType type) {
        mParameters[name] = AnimationParameter(name, type);
    }

    void BoneMaskAnimationGraphController::SetBool(const std::string& name, bool value) {
        auto it = mParameters.find(name);
        if (it != mParameters.end() && (it->second.type == ParameterType::Bool || it->second.type == ParameterType::Trigger)) {
            it->second.boolValue = value;
        }
    }

    void BoneMaskAnimationGraphController::SetInt(const std::string& name, int value) {
        auto it = mParameters.find(name);
        if (it != mParameters.end() && it->second.type == ParameterType::Int) {
            it->second.intValue = value;
        }
    }

    void BoneMaskAnimationGraphController::SetFloat(const std::string& name, float value) {
        auto it = mParameters.find(name);
        if (it != mParameters.end() && it->second.type == ParameterType::Float) {
            it->second.floatValue = value;
        }
    }

    void BoneMaskAnimationGraphController::SetTrigger(const std::string& name) {
        auto it = mParameters.find(name);
        if (it != mParameters.end() && it->second.type == ParameterType::Trigger) {
            it->second.boolValue = true;
        }
    }

    void BoneMaskAnimationGraphController::ResetTrigger(const std::string& name) {
        auto it = mParameters.find(name);
        if (it != mParameters.end() && it->second.type == ParameterType::Trigger) {
            it->second.boolValue = false;
        }
    }

    const AnimationParameter* BoneMaskAnimationGraphController::GetParameter(const std::string& name) const {
        auto it = mParameters.find(name);
        if (it != mParameters.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    void BoneMaskAnimationGraphController::EvaluateTransitions() {
        const BoneMaskAnimationState& currentState = mStates[mCurrentStateIndex];
        double normTimeMasked = mAnimator.GetNormalizedTimeMasked();
        double normTimeNonMasked = mAnimator.GetNormalizedTimeNonMasked();

        for (const auto& transition : currentState.transitions) {

            auto paramIt = mParameters.find(transition.parameterName);
            if (paramIt == mParameters.end())
                continue;

            const AnimationParameter& param = paramIt->second;
            bool conditionMet = false;
            switch (param.type) {
            case ParameterType::Bool: {
                bool expected = std::get<bool>(transition.expectedValue);
                if (param.boolValue == expected) {
                    conditionMet = true;
                }
                break;
            }
            case ParameterType::Int: {
                int expected = std::get<int>(transition.expectedValue);
                if (param.intValue == expected) {
                    conditionMet = true;
                }
                break;
            }
            case ParameterType::Float: {
                float expected = std::get<float>(transition.expectedValue);
                if (std::fabs(param.floatValue - expected) < 0.001f) {
                    conditionMet = true;
                }
                break;
            }
            case ParameterType::Trigger: {
                bool expected = std::get<bool>(transition.expectedValue);
                if (param.boolValue == expected && param.boolValue == true) {
                    conditionMet = true;
                    const_cast<AnimationParameter&>(param).boolValue = false;
                }
                break;
            }
            case ParameterType::Always: {
				conditionMet = true;
				break;
            }
            default:
                break;
            }

            if (transition.triggerOnEnd) {
                if (normTimeMasked < 0.99 and normTimeNonMasked < 0.99) {
                    conditionMet = false;
                }
            }
            
            if (conditionMet) {
                size_t targetState = transition.targetStateIndex;
                if (targetState < mStates.size() && targetState != mCurrentStateIndex) {
                    const BoneMaskAnimationState& target = mStates[targetState];
                    mAnimator.SetTransitionDuration(transition.blendDuration);
                    mAnimator.TransitionMaskedToClip(target.maskedClipIndex);
                    mAnimator.TransitionNonMaskedToClip(target.nonMaskedClipIndex);
                    mCurrentStateIndex = targetState;
                    break;
                }
            }
        }
    }

}