#pragma once 
#include <type_traits>
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