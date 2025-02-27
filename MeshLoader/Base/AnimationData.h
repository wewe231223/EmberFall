#pragma once 
#include <vector>
#include <DirectXMath.h>
#include <d3dcommon.h>
#include <string>
#include <memory>
#include <unordered_map>
#include "../Utility/DirectXInclude.h"

struct BoneAnimation {
    std::string boneName;

    std::vector<std::pair<double, SimpleMath::Vector3>> positionKey{};
	std::vector<std::pair<double, SimpleMath::Quaternion>> rotationKey{};
	std::vector<std::pair<double, SimpleMath::Vector3>> scalingKey{};
};

struct BoneNode {
    std::string name; // <-----------------------------------------------  1) 이 이름을 활용해서 
    SimpleMath::Matrix transformation;
    std::vector<std::shared_ptr<BoneNode>> children;
};

struct AnimationClip {
    float duration{ 0.0 };
    float ticksPerSecond{ 0.f };

	DirectX::SimpleMath::Matrix globalInverseTransform;

    std::vector<SimpleMath::Matrix> boneOffsetMatrices{};
    std::unordered_map<std::string, UINT> boneIndexMap{};           // 2) 여기서 index 를 통해 boneoffset matrix 를 얻는다. 
	std::unordered_map<std::string, BoneAnimation> boneAnimations{};// 3) 이 이름을 활용해서 bone animation 을 얻는다.

    std::shared_ptr<BoneNode> root{ nullptr };
};


// ---------------------------------------------------------------------------------------------------------------------------
struct NBoneNode {
    UINT index; // <-----------------------------------------------  1) 이 이름을 활용해서 
    DirectX::XMFLOAT4X4 transformation;
    std::vector<std::shared_ptr<BoneNode>> children;
};

struct NAnimationClip {
    float duration{ 0.0 };
    float ticksPerSecond{ 0.f };

    std::vector<DirectX::XMFLOAT4X4> boneOffsetMatrices{};
    std::unordered_map<UINT, BoneAnimation> boneAnimations{};// 3) 이 이름을 활용해서 bone animation 을 얻는다.

    std::shared_ptr<BoneNode> root{ nullptr };
};
// ---------------------------------------------------------------------------------------------------------------------------