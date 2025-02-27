#pragma once 
#include <vector>
#include <DirectXMath.h>
#include <d3dcommon.h>
#include <string>
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
    std::string name;
    DirectX::XMFLOAT4X4 transformation;
    BoneNode* parent;
    std::vector<BoneNode*> children;
};

struct AnimationClip {
    float duration;
    float ticksPerSecond;
    std::unordered_map<std::string, BoneAnimation> boneAnimations;
    BoneNode* root{ nullptr };
};