#pragma once 

#include <vector>
#include <DirectXMath.h>
#include <bitset>
#include <array>

struct MeshData {
	std::vector<DirectX::XMFLOAT3> position{};
	std::vector<DirectX::XMFLOAT3> normal{};
	std::vector<DirectX::XMFLOAT2> texCoord{};
	std::vector<DirectX::XMFLOAT3> tangent{};
	std::vector<DirectX::XMFLOAT3> bitangent{};
	std::vector<std::array<unsigned int, 4>> boneID{};
	std::vector<std::array<float, 4>> boneWeight{};

	std::vector<unsigned int> index{};

	std::bitset<7> vertexAttribute{};
	bool indexed{ false };
};