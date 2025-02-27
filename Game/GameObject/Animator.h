#pragma once 
#include "../MeshLoader/Base/AnimationData.h"
#include "../MeshLoader/Loader/AnimationLoader.h"

class Animator {
public:
	Animator() = default;
	~Animator() = default;
public:
private:
	void ReadNodeHeirarchy(double AnimationTime, BoneNode* pNode, const DirectX::XMFLOAT4X4& ParentTransform, const AnimationClip& animation, std::vector<DirectX::XMFLOAT4X4>& boneTransforms);
private:
	AnimationClip mClip{};

};