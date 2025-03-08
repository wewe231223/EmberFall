#pragma once 
#include "../MeshLoader/Base/AnimationData.h"
#include "../MeshLoader/Loader/AnimationLoader.h"
#include "../Config/Config.h"
#include "../Utility/Defines.h"

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
