#pragma once 
#include <type_traits>
#include <tuple>
#include "../Utility/DirectXInclude.h"
template <typename T>
concept HasIndex = requires {
	{ T::index } -> std::convertible_to<size_t>;
};

using MaterialIndex = size_t;

struct PlainModelContext {
	SimpleMath::Matrix world;
	MaterialIndex material;
};

template<typename T>
constexpr size_t GetCBVSize() {
	return (sizeof(T) + 255) & ~255;
}