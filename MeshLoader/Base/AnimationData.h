#pragma once 
#include <vector>
#include <DirectXMath.h>
#include <d3dcommon.h>
#include <string>
#include <memory>
#include <unordered_map>

struct BoneKeyFrame {
    float time;
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 rotation;
    DirectX::XMFLOAT3 scaling;
};

struct BoneAnimation {
    std::string boneName;

    std::vector<std::pair<double, DirectX::XMFLOAT3>> positionKey{};
	std::vector<std::pair<double, DirectX::XMFLOAT4>> rotationKey{};
	std::vector<std::pair<double, DirectX::XMFLOAT3>> scalingKey{};
};

struct BoneNode {
    std::string name; // <-----------------------------------------------  1) 이 이름을 활용해서 
    DirectX::XMFLOAT4X4 transformation;
    std::vector<std::shared_ptr<BoneNode>> children;
};

struct AnimationClip {
    float duration{ 0.0 };
    float ticksPerSecond{ 0.f };

    std::vector<DirectX::XMFLOAT4X4> boneOffsetMatrices{};
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