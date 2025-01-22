#pragma once 
#include <type_traits>

template <typename T>
concept HasIndex = requires {
	{ T::index } -> std::convertible_to<size_t>;
};
