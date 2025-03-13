#pragma once 
#include "../MeshLoader/Base/AnimationData.h"
#include "../MeshLoader/Loader/AnimationLoader.h"
#include "../Config/Config.h"
#include "../Utility/Defines.h"

#ifdef max 
#undef max 
#endif 


namespace Legacy {
	class Animator {
	public:
		Animator() = default;
		Animator(const AnimationClip& clip) : mClip(clip) {}
		~Animator() = default;
	public:
		void UpdateBoneTransform(double time, std::vector<SimpleMath::Matrix>& boneTransforms);
	private:
		void ReadNodeHeirarchy(double AnimationTime, BoneNode* pNode, const SimpleMath::Matrix& ParentTransform, const AnimationClip& animation, std::vector<SimpleMath::Matrix>& boneTransforms);

		UINT FindPosition(double AnimationTime, const BoneAnimation& boneAnim);
		UINT FindRotation(double AnimationTime, const BoneAnimation& boneAnim);
		UINT FindScaling(double AnimationTime, const BoneAnimation& boneAnim);

		SimpleMath::Vector3 InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim);
		SimpleMath::Quaternion InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim);
		SimpleMath::Vector3 InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim);
	private:
		AnimationClip mClip{};
	};
}


class Animator {
public:
	Animator() = default;
	Animator(const AnimationClip* clip) : mClip(clip) {}
	~Animator() = default;
public:
	bool GetActivated() const; 
	void UpdateBoneTransform(double time, BoneTransformBuffer& boneTransforms);
private:
	void ReadNodeHeirarchy(double AnimationTime, BoneNode* pNode, const SimpleMath::Matrix& ParentTransform, std::array<SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms);

	UINT FindPosition(double AnimationTime, const BoneAnimation& boneAnim);
	UINT FindRotation(double AnimationTime, const BoneAnimation& boneAnim);
	UINT FindScaling(double AnimationTime, const BoneAnimation& boneAnim);

	SimpleMath::Vector3 InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim);
	SimpleMath::Quaternion InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim);
	SimpleMath::Vector3 InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim);
private:
	double mCounter{ 0.0 };

	const AnimationClip* mClip{ nullptr };
};

namespace AnimatorGraph {
    struct TransformComponents {
        SimpleMath::Vector3 translation;
        SimpleMath::Quaternion rotation;
        SimpleMath::Vector3 scale;
    };

    inline bool DecomposeMatrix(SimpleMath::Matrix& mat, TransformComponents& outComponents) {
        return mat.Decompose(outComponents.scale, outComponents.rotation, outComponents.translation);
    }

    class Animator {
    public:
        Animator() = default;
        Animator(const std::vector<const AnimationClip*>& clips)
            : mClips(clips), mCurrentClipIndex(0), mTargetClipIndex(0), mTransitioning(false),
            mTransitionTime(0.0), mTransitionDuration(0.1), mCurrentTime(0.0) {}


        void UpdateBoneTransform(double deltaTime, BoneTransformBuffer& boneTransforms) {
            const AnimationClip* clip = mClips[mCurrentClipIndex];
            if (mTransitioning) {
                double blendFactor = mTransitionTime / mTransitionDuration;

                double tick = mCurrentTime * clip->ticksPerSecond;
                double normTime = std::fmod(tick / clip->duration, 1.0);
                double currentAnimTime = normTime * clip->duration; 


                ReadNodeHeirarchyTransition(currentAnimTime, 0.0, blendFactor,
                    clip->root.get(), SimpleMath::Matrix::Identity, boneTransforms.boneTransforms);
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


        void TransitionToClip(size_t clipIndex) {
            if (clipIndex < mClips.size() && clipIndex != mCurrentClipIndex) {
                mTargetClipIndex = clipIndex;
                mTransitioning = true;
                mTransitionTime = 0.0;
            }
        }

    private:

        void ComputeBoneTransforms(const AnimationClip* clip, double animationTime, BoneTransformBuffer& boneTransforms) {
            DirectX::SimpleMath::Matrix identity = DirectX::SimpleMath::Matrix::Identity;
            std::fill(std::begin(boneTransforms.boneTransforms),
                std::end(boneTransforms.boneTransforms),
                DirectX::SimpleMath::Matrix::Identity);
            ReadNodeHeirarchy(animationTime, clip->root.get(), identity, boneTransforms.boneTransforms, clip);
            boneTransforms.boneCount = static_cast<UINT>(clip->boneOffsetMatrices.size());
        }


        void ReadNodeHeirarchy(double AnimationTime, BoneNode* node, const SimpleMath::Matrix& ParentTransform,
            std::array<SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms,
            const AnimationClip* clip) {
            if (!node) return;
            SimpleMath::Matrix nodeTransform = node->transformation;
            auto animIt = clip->boneAnimations.find(node->index);
            if (animIt != clip->boneAnimations.end()) {
                const BoneAnimation& boneAnim = animIt->second;
                SimpleMath::Vector3 pos = InterpolatePosition(AnimationTime, boneAnim);
                SimpleMath::Quaternion rot = InterpolateRotation(AnimationTime, boneAnim);
                SimpleMath::Vector3 scale = InterpolateScaling(AnimationTime, boneAnim);
                nodeTransform = SimpleMath::Matrix::CreateScale(scale) *
                    SimpleMath::Matrix::CreateFromQuaternion(rot) *
                    SimpleMath::Matrix::CreateTranslation(pos);
            }
            SimpleMath::Matrix globalTransform = nodeTransform * ParentTransform;
            if (node->index != std::numeric_limits<UINT>::max()) {
                auto result = clip->boneOffsetMatrices[node->index] *
                    globalTransform *
                    clip->globalInverseTransform;
                boneTransforms[node->index] = result.Transpose();
            }
            for (auto& child : node->children) {
                ReadNodeHeirarchy(AnimationTime, child.get(), globalTransform, boneTransforms, clip);
            }
        }

        void ReadNodeHeirarchyTransition(double currentAnimTime, double targetAnimTime, double blendFactor,
            BoneNode* node, const SimpleMath::Matrix& ParentTransform,
            std::array<SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms) {
            if (!node) return;
            TransformComponents currentComp, targetComp;

            auto itCurrent = mClips[mCurrentClipIndex]->boneAnimations.find(node->index);
            if (itCurrent != mClips[mCurrentClipIndex]->boneAnimations.end()) {
                const BoneAnimation& currentBoneAnim = itCurrent->second;
                currentComp.translation = InterpolatePosition(currentAnimTime, currentBoneAnim);
                currentComp.rotation = InterpolateRotation(currentAnimTime, currentBoneAnim);
                currentComp.scale = InterpolateScaling(currentAnimTime, currentBoneAnim);
            }
            else {
                DecomposeMatrix(node->transformation, currentComp);
            }

            auto itTarget = mClips[mTargetClipIndex]->boneAnimations.find(node->index);
            if (itTarget != mClips[mTargetClipIndex]->boneAnimations.end()) {
                const BoneAnimation& targetBoneAnim = itTarget->second;
                targetComp.translation = InterpolatePosition(targetAnimTime, targetBoneAnim);
                targetComp.rotation = InterpolateRotation(targetAnimTime, targetBoneAnim);
                targetComp.scale = InterpolateScaling(targetAnimTime, targetBoneAnim);
            }
            else {
                DecomposeMatrix(node->transformation, targetComp);
            }

            SimpleMath::Vector3 blendedTranslation = SimpleMath::Vector3::Lerp(currentComp.translation, targetComp.translation, static_cast<float>(blendFactor));
            SimpleMath::Quaternion blendedRotation = SimpleMath::Quaternion::Slerp(currentComp.rotation, targetComp.rotation, static_cast<float>(blendFactor));
            SimpleMath::Vector3 blendedScale = SimpleMath::Vector3::Lerp(currentComp.scale, targetComp.scale, static_cast<float>(blendFactor));

            SimpleMath::Matrix blendedLocal = SimpleMath::Matrix::CreateScale(blendedScale) *
                SimpleMath::Matrix::CreateFromQuaternion(blendedRotation) *
                SimpleMath::Matrix::CreateTranslation(blendedTranslation);
            SimpleMath::Matrix globalTransform = blendedLocal * ParentTransform;
            if (node->index != std::numeric_limits<UINT>::max()) {
                auto result = mClips[mCurrentClipIndex]->boneOffsetMatrices[node->index] *
                    globalTransform *
                    mClips[mCurrentClipIndex]->globalInverseTransform;
                boneTransforms[node->index] = result.Transpose();
            }
            for (auto& child : node->children) {
                ReadNodeHeirarchyTransition(currentAnimTime, targetAnimTime, blendFactor, child.get(), globalTransform, boneTransforms);
            }
        }


        UINT FindPosition(double AnimationTime, const BoneAnimation& boneAnim) {
            for (UINT i = 0; i < boneAnim.positionKey.size() - 1; i++) {
                if (AnimationTime < boneAnim.positionKey[i + 1].first)
                    return i;
            }
            return static_cast<UINT>(boneAnim.positionKey.size() - 2);
        }

        UINT FindRotation(double AnimationTime, const BoneAnimation& boneAnim) {
            for (UINT i = 0; i < boneAnim.rotationKey.size() - 1; i++) {
                if (AnimationTime < boneAnim.rotationKey[i + 1].first)
                    return i;
            }
            return static_cast<UINT>(boneAnim.rotationKey.size() - 2);
        }

        UINT FindScaling(double AnimationTime, const BoneAnimation& boneAnim) {
            for (UINT i = 0; i < boneAnim.scalingKey.size() - 1; i++) {
                if (AnimationTime < boneAnim.scalingKey[i + 1].first)
                    return i;
            }
            return static_cast<UINT>(boneAnim.scalingKey.size() - 2);
        }

        SimpleMath::Vector3 InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim) {
            if (boneAnim.positionKey.size() == 1)
                return boneAnim.positionKey[0].second;
            UINT posIndex = FindPosition(AnimationTime, boneAnim);
            UINT nextIndex = posIndex + 1;
            float deltaTime = static_cast<float>(boneAnim.positionKey[nextIndex].first - boneAnim.positionKey[posIndex].first);
            float factor = static_cast<float>(AnimationTime - boneAnim.positionKey[posIndex].first) / deltaTime;
            SimpleMath::Vector3 start = boneAnim.positionKey[posIndex].second;
            SimpleMath::Vector3 delta = boneAnim.positionKey[nextIndex].second - start;
            return start + factor * delta;
        }

        SimpleMath::Quaternion InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim) {
            if (boneAnim.rotationKey.size() == 1)
                return boneAnim.rotationKey[0].second;
            UINT rotIndex = FindRotation(AnimationTime, boneAnim);
            UINT nextIndex = rotIndex + 1;
            float deltaTime = static_cast<float>(boneAnim.rotationKey[nextIndex].first - boneAnim.rotationKey[rotIndex].first);
            float factor = static_cast<float>(AnimationTime - boneAnim.rotationKey[rotIndex].first) / deltaTime;
            SimpleMath::Quaternion start = boneAnim.rotationKey[rotIndex].second;
            SimpleMath::Quaternion end = boneAnim.rotationKey[nextIndex].second;
            return SimpleMath::Quaternion::Slerp(start, end, factor);
        }

        SimpleMath::Vector3 InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim) {
            if (boneAnim.scalingKey.size() == 1)
                return boneAnim.scalingKey[0].second;
            UINT scaleIndex = FindScaling(AnimationTime, boneAnim);
            UINT nextIndex = scaleIndex + 1;
            float deltaTime = static_cast<float>(boneAnim.scalingKey[nextIndex].first - boneAnim.scalingKey[scaleIndex].first);
            float factor = static_cast<float>(AnimationTime - boneAnim.scalingKey[scaleIndex].first) / deltaTime;
            SimpleMath::Vector3 start = boneAnim.scalingKey[scaleIndex].second;
            SimpleMath::Vector3 delta = boneAnim.scalingKey[nextIndex].second - start;
            return start + factor * delta;
        }

    private:
        std::vector<const AnimationClip*> mClips;
        size_t mCurrentClipIndex;
        size_t mTargetClipIndex;
        bool mTransitioning;
        double mTransitionTime;
        double mTransitionDuration;
        double mCurrentTime;
    };





};
