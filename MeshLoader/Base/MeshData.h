#pragma once 

#include <vector>
#include <DirectXMath.h>
#include <d3dcommon.h>
#include <bitset>
#include <array>

struct MeshData {
	std::vector<DirectX::XMFLOAT3> position{};
	std::vector<DirectX::XMFLOAT3> normal{};
	std::vector<DirectX::XMFLOAT2> texCoord1{};
	std::vector<DirectX::XMFLOAT2> texCoord2{};
	std::vector<DirectX::XMFLOAT3> tangent{};
	std::vector<DirectX::XMFLOAT3> bitangent{};
	std::vector<std::array<int, 4>> boneID{};
	std::vector<std::array<float, 4>> boneWeight{};

	std::vector<unsigned int> index{};

	std::bitset<8> vertexAttribute{};
	unsigned int unitCount{ 0 };

	bool indexed{ false };
	D3D_PRIMITIVE_TOPOLOGY primitiveTopology{ D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
};