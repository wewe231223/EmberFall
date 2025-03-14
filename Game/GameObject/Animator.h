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
		Animator(const std::vector<const AnimationClip*>& clips);
		~Animator() = default;

	public:
		void UpdateBoneTransform(double deltaTime, BoneTransformBuffer& boneTransforms);
		void TransitionToClip(size_t clipIndex);

	private:
		void ComputeBoneTransforms(const AnimationClip* clip, double animationTime, BoneTransformBuffer& boneTransforms);
		void ReadNodeHeirarchy(double AnimationTime, BoneNode* node, const DirectX::SimpleMath::Matrix& ParentTransform, std::array<DirectX::SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms, const AnimationClip* clip);
		void ReadNodeHeirarchyTransition(double currentAnimTime, double targetAnimTime, double blendFactor, BoneNode* node, const DirectX::SimpleMath::Matrix& ParentTransform, std::array<DirectX::SimpleMath::Matrix, Config::MAX_BONE_COUNT_PER_INSTANCE<>>& boneTransforms);

		unsigned int FindPosition(double AnimationTime, const BoneAnimation& boneAnim);
		unsigned int FindRotation(double AnimationTime, const BoneAnimation& boneAnim);
		unsigned int FindScaling(double AnimationTime, const BoneAnimation& boneAnim);

		DirectX::SimpleMath::Vector3 InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim);
		DirectX::SimpleMath::Quaternion InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim);
		DirectX::SimpleMath::Vector3 InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim);

	private:
		std::vector<const AnimationClip*> mClips{};

		size_t mCurrentClipIndex{ 0 };
		size_t mTargetClipIndex{ 0 };

		bool mTransitioning{ false };

		double mTransitionTime{ 0.0 };
		double mTransitionDuration{ 0.09 }; // 애니메이션 전환 간격을 조정함
		double mCurrentTime{ 0.0 };
	};
};
