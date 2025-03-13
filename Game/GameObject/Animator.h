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
    // 로컬 변환 구성요소를 보관하는 구조체
    struct TransformComponents {
        SimpleMath::Vector3 translation;
        SimpleMath::Quaternion rotation;
        SimpleMath::Vector3 scale;
    };

    // (필요 시) 행렬을 구성요소로 분해하는 유틸리티 함수
    inline bool DecomposeMatrix(SimpleMath::Matrix& mat, TransformComponents& outComponents) {
        return mat.Decompose(outComponents.scale, outComponents.rotation, outComponents.translation);
    }

    class Animator {
    public:
        Animator() = default;
        Animator(const std::vector<const AnimationClip*>& clips)
            : mClips(clips), mCurrentClipIndex(0), mTargetClipIndex(0), mTransitioning(false),
            mTransitionTime(0.0), mTransitionDuration(0.001), mCurrentTime(0.0) {}

        // deltaTime: 프레임 간 경과 시간
        void UpdateBoneTransform(double deltaTime, BoneTransformBuffer& boneTransforms) {
            if (mTransitioning) {
                double blendFactor = mTransitionTime / mTransitionDuration;
                // 전환 시 현재 클립은 현재 재생 시간, 대상 클립은 첫 키프레임(0.0)을 기준으로 보간
                ReadNodeHeirarchyTransition(mCurrentTime, 0.0, blendFactor, mClips[mCurrentClipIndex]->root.get(),
                    SimpleMath::Matrix::Identity, boneTransforms.boneTransforms);
                boneTransforms.boneCount = static_cast<UINT>(mClips[mCurrentClipIndex]->boneOffsetMatrices.size());
                mTransitionTime += deltaTime;

                if (mTransitionTime >= mTransitionDuration) {
                    mTransitioning = false;
                    mCurrentClipIndex = mTargetClipIndex;
                    mCurrentTime = 0.0;
                }
            }
            else {
                const AnimationClip* clip = mClips[mCurrentClipIndex];
                mCurrentTime += deltaTime * clip->ticksPerSecond;
                ComputeBoneTransforms(clip, mCurrentTime, boneTransforms);
                if (mCurrentTime * clip->ticksPerSecond >= clip->duration) {
                    // 애니메이션 재생 종료 시 자동으로 0번 클립으로 전환
                    TransitionToClip(0);
                }
            }
        }

        // 전환 요청 함수: 대상 클립 인덱스를 받아 전환 시작
        void TransitionToClip(size_t clipIndex) {
            if (clipIndex < mClips.size() && clipIndex != mCurrentClipIndex) {
                mTargetClipIndex = clipIndex;
                mTransitioning = true;
                mTransitionTime = 0.0;
            }
        }

    private:
        // 기존 애니메이션에 따른 bone transform 계산 함수
        void ComputeBoneTransforms(const AnimationClip* clip, double time, BoneTransformBuffer& boneTransforms) {
            DirectX::SimpleMath::Matrix identity = DirectX::SimpleMath::Matrix::Identity;
            double tick = time * clip->ticksPerSecond;
            double animationTime = std::fmod(tick, clip->duration);
            animationTime = std::clamp(tick, 0.0, static_cast<double>(clip->duration));
            std::fill(std::begin(boneTransforms.boneTransforms),
                std::end(boneTransforms.boneTransforms),
                DirectX::SimpleMath::Matrix::Identity);
            ReadNodeHeirarchy(animationTime, clip->root.get(), identity, boneTransforms.boneTransforms, clip);
            boneTransforms.boneCount = static_cast<UINT>(clip->boneOffsetMatrices.size());
        }

        // 기존 재귀 함수: 각 bone에 대해 키프레임을 보간하여 행렬 계산
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
                auto result = clip->boneOffsetMatrices[node->index] * globalTransform * clip->globalInverseTransform;
                boneTransforms[node->index] = result.Transpose();
            }
            for (auto& child : node->children) {
                ReadNodeHeirarchy(AnimationTime, child.get(), globalTransform, boneTransforms, clip);
            }
        }

        // 전환 시, 현재 클립과 대상 클립의 로컬 Transform 구성요소를 보간하여 계산하는 함수
        // 여기서는 대상 애니메이션의 첫 키프레임(시간 0.0)과 현재 애니메이션의 현재 키프레임을 블렌딩합니다.
        void ReadNodeHeirarchyTransition(double currentAnimTime, double targetAnimTime, double blendFactor,
            BoneNode* node, const SimpleMath::Matrix& ParentTransform,
            std::array<SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms) {
            if (!node) return;
            TransformComponents currentComp, targetComp;
            // 현재 클립에서의 구성요소 계산 (키 보간)
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
            // 대상 클립에서의 구성요소 계산 (항상 첫 키프레임, targetAnimTime = 0.0)
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
            // 개별 구성요소 보간 (이동/스케일: Lerp, 회전: Slerp)
            SimpleMath::Vector3 blendedTranslation = SimpleMath::Vector3::Lerp(currentComp.translation, targetComp.translation, static_cast<float>(blendFactor));
            SimpleMath::Quaternion blendedRotation = SimpleMath::Quaternion::Slerp(currentComp.rotation, targetComp.rotation, static_cast<float>(blendFactor));
            SimpleMath::Vector3 blendedScale = SimpleMath::Vector3::Lerp(currentComp.scale, targetComp.scale, static_cast<float>(blendFactor));
            // 보간된 구성요소로 로컬 행렬 재구성
            SimpleMath::Matrix blendedLocal = SimpleMath::Matrix::CreateScale(blendedScale) *
                SimpleMath::Matrix::CreateFromQuaternion(blendedRotation) *
                SimpleMath::Matrix::CreateTranslation(blendedTranslation);
            SimpleMath::Matrix globalTransform = blendedLocal * ParentTransform;
            // 모든 클립에서 동일한 boneOffsetMatrices 와 globalInverseTransform 사용
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

        // 기존 키프레임 보간 함수들
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
