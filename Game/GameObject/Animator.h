#pragma once 
#include "../MeshLoader/Base/AnimationData.h"
#include "../MeshLoader/Loader/AnimationLoader.h"

class Animator {
public:
	Animator() = default;
	~Animator() = default;
public:
private:
	void ReadNodeHeirarchy(double AnimationTime, BoneNode* pNode,const SimpleMath::Matrix& ParentTransform,const AnimationClip& animation, std::vector<SimpleMath::Matrix>& boneTransforms);

	SimpleMath::Vector3 InterpolatePosition(double AnimationTime, const BoneAnimation& boneAnim);
	SimpleMath::Quaternion InterpolateRotation(double AnimationTime, const BoneAnimation& boneAnim);
	SimpleMath::Vector3 InterpolateScaling(double AnimationTime, const BoneAnimation& boneAnim);

private:
	AnimationClip mClip{};

};