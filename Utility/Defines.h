#pragma once 
#include <type_traits>
#include <tuple>
#include <bitset>
#include <array>
#include "../Utility/DirectXInclude.h"
template <typename T>
concept HasIndex = requires {
	{ T::index } -> std::convertible_to<size_t>;
};

using MaterialIndex = UINT;

struct ModelContext {
	SimpleMath::Matrix world;
	SimpleMath::Vector3 BBextents{};
	UINT material;
};

struct AnimationModelContext {
	SimpleMath::Matrix world;
	SimpleMath::Vector3 BBextents{};
	UINT material;
	UINT boneIndexStart{ 0 };
};

struct BoneTransformBuffer {
	std::array<SimpleMath::Matrix, 150>	boneTransforms;
	UINT boneCount;
};

template<typename T>
constexpr size_t GetCBVSize() {
	return (sizeof(T) + 255) & ~255;
}

constexpr size_t GetCBVSize(size_t size) {
	return (size + 255) & ~255;
}

/*
B 가 A 의 부분집합인지 확인한다. 
매개변수의 순서에 주목
*/
template<size_t N>
bool IsSubSet(const std::bitset<N>& a, const std::bitset<N>& b) {
	return (a & b) == b;
}


enum class StringColor : DWORD{
	red, green, blue, yellow, white, black, END
};