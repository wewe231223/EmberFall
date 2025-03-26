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

struct CameraConstants {
    SimpleMath::Matrix view;
    SimpleMath::Matrix proj;
    SimpleMath::Matrix viewProj;
    SimpleMath::Vector3 cameraPosition;
};

using MaterialIndex = UINT;

struct ModelContext {
	SimpleMath::Matrix world;
    SimpleMath::Vector3 BBCenter{}; 
	SimpleMath::Vector3 BBextents{};
	UINT material;
};

struct AnimationModelContext {
	SimpleMath::Matrix world;
	SimpleMath::Vector3 BBCenter{};
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


enum class StringColor : DWORD {
    AliceBlue, AntiqueWhite, Aqua, Aquamarine, Azure,
    Beige, Bisque, Black, BlanchedAlmond, Blue,
    BlueViolet, Brown, BurlyWood, CadetBlue, Chartreuse,
    Chocolate, Coral, CornflowerBlue, Cornsilk, Crimson,
    Cyan, DarkBlue, DarkCyan, DarkGoldenrod, DarkGray,
    DarkGreen, DarkKhaki, DarkMagenta, DarkOliveGreen, DarkOrange,
    DarkOrchid, DarkRed, DarkSalmon, DarkSeaGreen, DarkSlateBlue,
    DarkSlateGray, DarkTurquoise, DarkViolet, DeepPink, DeepSkyBlue,
    DimGray, DodgerBlue, Firebrick, FloralWhite, ForestGreen,
    Fuchsia, Gainsboro, GhostWhite, Gold, Goldenrod,
    Gray, Green, GreenYellow, Honeydew, HotPink,
    IndianRed, Indigo, Ivory, Khaki, Lavender,
    LavenderBlush, LawnGreen, LemonChiffon, LightBlue, LightCoral,
    LightCyan, LightGoldenrodYellow, LightGreen, LightGray, LightPink,
    LightSalmon, LightSeaGreen, LightSkyBlue, LightSlateGray, LightSteelBlue,
    LightYellow, Lime, LimeGreen, Linen, Magenta,
    Maroon, MediumAquamarine, MediumBlue, MediumOrchid, MediumPurple,
    MediumSeaGreen, MediumSlateBlue, MediumSpringGreen, MediumTurquoise, MediumVioletRed,
    MidnightBlue, MintCream, MistyRose, Moccasin, NavajoWhite,
    Navy, OldLace, Olive, OliveDrab, Orange,
    OrangeRed, Orchid, PaleGoldenrod, PaleGreen, PaleTurquoise,
    PaleVioletRed, PapayaWhip, PeachPuff, Peru, Pink,
    Plum, PowderBlue, Purple, Red, RosyBrown,
    RoyalBlue, SaddleBrown, Salmon, SandyBrown, SeaGreen,
    Seashell, Sienna, Silver, SkyBlue, SlateBlue,
    SlateGray, Snow, SpringGreen, SteelBlue, Tan,
    Teal, Thistle, Tomato, Turquoise, Violet,
    Wheat, White, WhiteSmoke, Yellow, YellowGreen,
    END
};

enum class StringFontType : WORD {
	NotoSans, END 
};