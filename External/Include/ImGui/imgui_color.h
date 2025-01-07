#pragma once 
#include <string_view>
#include "imgui.h"

#pragma once 
// 놀라운 사실 : 어느정도 수제가 가미된.. 
constexpr const char* HEX_RED = "#FF0000";
constexpr const char* HEX_GREEN = "#00FF00";
constexpr const char* HEX_BLUE = "#0000FF";
constexpr const char* HEX_WHITE = "#FFFFFF";
constexpr const char* HEX_BLACK = "#000000";
constexpr const char* HEX_YELLOW = "#FFFF00";
constexpr const char* HEX_CYAN = "#00FFFF";
constexpr const char* HEX_MAGENTA = "#FF00FF";
constexpr const char* HEX_GRAY = "#808080";
constexpr const char* HEX_ORANGE = "#FFA500";
constexpr const char* HEX_PURPLE = "#800080";
constexpr const char* HEX_BROWN = "#A52A2A";
constexpr const char* HEX_PINK = "#FFC0CB";
constexpr const char* HEX_LIME = "#00FF00";
constexpr const char* HEX_NAVY = "#000080";
constexpr const char* HEX_TEAL = "#008080";
constexpr const char* HEX_MAROON = "#800000";
constexpr const char* HEX_OLIVE = "#808000";
constexpr const char* HEX_SNOW = "#FFFAFA";
constexpr const char* HEX_GHOST_WHITE = "#F8F8FF";
constexpr const char* HEX_IVORY = "#FFFFF0";
constexpr const char* HEX_FLORAL_WHITE = "#FFFAF0";
constexpr const char* HEX_LINEN = "#FAF0E6";
constexpr const char* HEX_BEIGE = "#F5F5DC";
constexpr const char* HEX_OLD_LACE = "#FDF5E6";
constexpr const char* HEX_WHEAT = "#F5DEB3";
constexpr const char* HEX_MOCCASIN = "#FFE4B5";
constexpr const char* HEX_DARK_ORANGE = "#FF8C00";
constexpr const char* HEX_CORAL = "#FF7F50";
constexpr const char* HEX_TOMATO = "#FF6347";
constexpr const char* HEX_CRIMSON = "#DC143C";
constexpr const char* HEX_FIREBRICK = "#B22222";
constexpr const char* HEX_DARK_RED = "#8B0000";
constexpr const char* HEX_GOLD = "#FFD700";
constexpr const char* HEX_KHAKI = "#F0E68C";
constexpr const char* HEX_DARK_KHAKI = "#BDB76B";
constexpr const char* HEX_LIME_GREEN = "#32CD32";
constexpr const char* HEX_LIGHT_GREEN = "#90EE90";
constexpr const char* HEX_PALE_GREEN = "#98FB98";
constexpr const char* HEX_DARK_GREEN = "#006400";
constexpr const char* HEX_FOREST_GREEN = "#228B22";
constexpr const char* HEX_SEA_GREEN = "#2E8B57";
constexpr const char* HEX_AQUA = "#00FFFF";
constexpr const char* HEX_TURQUOISE = "#40E0D0";
constexpr const char* HEX_MEDIUM_TURQUOISE = "#48D1CC";
constexpr const char* HEX_DARK_TURQUOISE = "#00CED1";
constexpr const char* HEX_LIGHT_SEA_GREEN = "#20B2AA";
constexpr const char* HEX_MEDIUM_BLUE = "#0000CD";
constexpr const char* HEX_DARK_BLUE = "#00008B";
constexpr const char* HEX_MIDNIGHT_BLUE = "#191970";
constexpr const char* HEX_INDIGO = "#4B0082";
constexpr const char* HEX_DARK_MAGENTA = "#8B008B";
constexpr const char* HEX_FUCHSIA = "#FF00FF";
constexpr const char* HEX_VIOLET = "#EE82EE";
constexpr const char* HEX_ORCHID = "#DA70D6";
constexpr const char* HEX_PLUM = "#DDA0DD";
constexpr const char* HEX_LAVENDER = "#E6E6FA";
constexpr const char* HEX_LIGHT_PINK = "#FFB6C1";
constexpr const char* HEX_HOT_PINK = "#FF69B4";
constexpr const char* HEX_DEEP_PINK = "#FF1493";
constexpr const char* HEX_SALMON = "#FA8072";
constexpr const char* HEX_LIGHT_SALMON = "#FFA07A";
constexpr const char* HEX_TAN = "#D2B48C";
constexpr const char* HEX_PERU = "#CD853F";
constexpr const char* HEX_SADDLE_BROWN = "#8B4513";
constexpr const char* HEX_SIENNA = "#A0522D";
constexpr const char* HEX_CHOCOLATE = "#D2691E";
constexpr const char* HEX_SANDY_BROWN = "#F4A460";
constexpr const char* HEX_BURLYWOOD = "#DEB887";
constexpr const char* HEX_GOLDENROD = "#DAA520";
constexpr const char* HEX_DARK_GOLDENROD = "#B8860B";
constexpr const char* HEX_DARK_OLIVE_GREEN = "#556B2F";
constexpr const char* HEX_YELLOW_GREEN = "#9ACD32";
constexpr const char* HEX_OLIVE_DRAB = "#6B8E23";
constexpr const char* HEX_DARK_SEA_GREEN = "#8FBC8F";
constexpr const char* HEX_PALE_TURQUOISE = "#AFEEEE";
constexpr const char* HEX_LIGHT_CYAN = "#E0FFFF";
constexpr const char* HEX_PALE_VIOLET_RED = "#DB7093";
constexpr const char* HEX_THISTLE = "#D8BFD8";
constexpr const char* HEX_HONEYDEW = "#F0FFF0";
constexpr const char* HEX_MINT_CREAM = "#F5FFFA";
constexpr const char* HEX_AZURE = "#F0FFFF";
constexpr const char* HEX_ALICE_BLUE = "#F0F8FF";
constexpr const char* HEX_LAVENDER_BLUSH = "#FFF0F5";
constexpr const char* HEX_MISTY_ROSE = "#FFE4E1";
constexpr const char* HEX_SEASHELL = "#FFF5EE";
constexpr const char* HEX_BISQUE = "#FFE4C4";
constexpr const char* HEX_PEACH_PUFF = "#FFDAB9";
constexpr const char* HEX_LEMON_CHIFFON = "#FFFACD";
constexpr const char* HEX_LIGHT_YELLOW = "#FFFFE0";
constexpr const char* HEX_PALE_GOLDENROD = "#EEE8AA";
constexpr const char* HEX_ANTIQUE_WHITE = "#FAEBD7";
constexpr const char* HEX_PAPAYA_WHIP = "#FFEFD5";
constexpr const char* HEX_BLANCHED_ALMOND = "#FFEBCD";
constexpr const char* HEX_CORNSILK = "#FFF8DC";
constexpr const char* HEX_WHITE_SMOKE = "#F5F5F5";
constexpr const char* HEX_GAINSBORO = "#DCDCDC";
constexpr const char* HEX_LIGHT_GRAY = "#D3D3D3";
constexpr const char* HEX_SILVER = "#C0C0C0";
constexpr const char* HEX_DARK_GRAY = "#A9A9A9";
constexpr const char* HEX_DIM_GRAY = "#696969";
constexpr const char* HEX_LIGHT_SLATE_GRAY = "#778899";
constexpr const char* HEX_SLATE_GRAY = "#708090";
constexpr const char* HEX_DARK_SLATE_GRAY = "#2F4F4F";

constexpr ImVec4 MakeRGBAFromHex(std::string_view hexColor)
{
	auto hexCharToInt = [](char c) constexpr -> int {
		return  (c >= '0' && c <= '9') ? (c - '0') :
			(c >= 'A' && c <= 'F') ? (c - 'A' + 10) :
			(c >= 'a' && c <= 'f') ? (c - 'a' + 10) : 0;
		};
	// '#'가 포함된 경우 제거
	if (hexColor.front() == '#') {
		hexColor.remove_prefix(1);
	}

	// 입력 길이가 유효한지 확인 (6 또는 8)
	if (hexColor.length() != 6 && hexColor.length() != 8) {
		return ImVec4(0.0f, 0.0f, 0.0f, 1.0f);  // 기본값 (검은색)
	}

	int r = (hexCharToInt(hexColor[0]) << 4) | hexCharToInt(hexColor[1]);
	int g = (hexCharToInt(hexColor[2]) << 4) | hexCharToInt(hexColor[3]);
	int b = (hexCharToInt(hexColor[4]) << 4) | hexCharToInt(hexColor[5]);
	int a = 255;  // 기본적으로 알파는 255(불투명)

	if (hexColor.length() == 8) {
		a = (hexCharToInt(hexColor[6]) << 4) | hexCharToInt(hexColor[7]);
	}
	return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}


constexpr ImVec4 F4_RED = MakeRGBAFromHex(HEX_RED);
constexpr ImVec4 F4_GREEN = MakeRGBAFromHex(HEX_GREEN);
constexpr ImVec4 F4_BLUE = MakeRGBAFromHex(HEX_BLUE);
constexpr ImVec4 F4_WHITE = MakeRGBAFromHex(HEX_WHITE);
constexpr ImVec4 F4_BLACK = MakeRGBAFromHex(HEX_BLACK);
constexpr ImVec4 F4_YELLOW = MakeRGBAFromHex(HEX_YELLOW);
constexpr ImVec4 F4_CYAN = MakeRGBAFromHex(HEX_CYAN);
constexpr ImVec4 F4_MAGENTA = MakeRGBAFromHex(HEX_MAGENTA);
constexpr ImVec4 F4_GRAY = MakeRGBAFromHex(HEX_GRAY);
constexpr ImVec4 F4_ORANGE = MakeRGBAFromHex(HEX_ORANGE);
constexpr ImVec4 F4_PURPLE = MakeRGBAFromHex(HEX_PURPLE);
constexpr ImVec4 F4_BROWN = MakeRGBAFromHex(HEX_BROWN);
constexpr ImVec4 F4_PINK = MakeRGBAFromHex(HEX_PINK);
constexpr ImVec4 F4_LIME = MakeRGBAFromHex(HEX_LIME);
constexpr ImVec4 F4_NAVY = MakeRGBAFromHex(HEX_NAVY);
constexpr ImVec4 F4_TEAL = MakeRGBAFromHex(HEX_TEAL);
constexpr ImVec4 F4_MAROON = MakeRGBAFromHex(HEX_MAROON);
constexpr ImVec4 F4_OLIVE = MakeRGBAFromHex(HEX_OLIVE);
constexpr ImVec4 F4_SNOW = MakeRGBAFromHex(HEX_SNOW);
constexpr ImVec4 F4_GHOST_WHITE = MakeRGBAFromHex(HEX_GHOST_WHITE);
constexpr ImVec4 F4_IVORY = MakeRGBAFromHex(HEX_IVORY);
constexpr ImVec4 F4_FLORAL_WHITE = MakeRGBAFromHex(HEX_FLORAL_WHITE);
constexpr ImVec4 F4_LINEN = MakeRGBAFromHex(HEX_LINEN);
constexpr ImVec4 F4_BEIGE = MakeRGBAFromHex(HEX_BEIGE);
constexpr ImVec4 F4_OLD_LACE = MakeRGBAFromHex(HEX_OLD_LACE);
constexpr ImVec4 F4_WHEAT = MakeRGBAFromHex(HEX_WHEAT);
constexpr ImVec4 F4_MOCCASIN = MakeRGBAFromHex(HEX_MOCCASIN);
constexpr ImVec4 F4_DARK_ORANGE = MakeRGBAFromHex(HEX_DARK_ORANGE);
constexpr ImVec4 F4_CORAL = MakeRGBAFromHex(HEX_CORAL);
constexpr ImVec4 F4_TOMATO = MakeRGBAFromHex(HEX_TOMATO);
constexpr ImVec4 F4_CRIMSON = MakeRGBAFromHex(HEX_CRIMSON);
constexpr ImVec4 F4_FIREBRICK = MakeRGBAFromHex(HEX_FIREBRICK);
constexpr ImVec4 F4_DARK_RED = MakeRGBAFromHex(HEX_DARK_RED);
constexpr ImVec4 F4_GOLD = MakeRGBAFromHex(HEX_GOLD);
constexpr ImVec4 F4_KHAKI = MakeRGBAFromHex(HEX_KHAKI);
constexpr ImVec4 F4_DARK_KHAKI = MakeRGBAFromHex(HEX_DARK_KHAKI);
constexpr ImVec4 F4_LIME_GREEN = MakeRGBAFromHex(HEX_LIME_GREEN);
constexpr ImVec4 F4_LIGHT_GREEN = MakeRGBAFromHex(HEX_LIGHT_GREEN);
constexpr ImVec4 F4_PALE_GREEN = MakeRGBAFromHex(HEX_PALE_GREEN);
constexpr ImVec4 F4_DARK_GREEN = MakeRGBAFromHex(HEX_DARK_GREEN);
constexpr ImVec4 F4_FOREST_GREEN = MakeRGBAFromHex(HEX_FOREST_GREEN);
constexpr ImVec4 F4_SEA_GREEN = MakeRGBAFromHex(HEX_SEA_GREEN);
constexpr ImVec4 F4_AQUA = MakeRGBAFromHex(HEX_AQUA);
constexpr ImVec4 F4_TURQUOISE = MakeRGBAFromHex(HEX_TURQUOISE);
constexpr ImVec4 F4_MEDIUM_TURQUOISE = MakeRGBAFromHex(HEX_MEDIUM_TURQUOISE);
constexpr ImVec4 F4_DARK_TURQUOISE = MakeRGBAFromHex(HEX_DARK_TURQUOISE);
constexpr ImVec4 F4_LIGHT_SEA_GREEN = MakeRGBAFromHex(HEX_LIGHT_SEA_GREEN);
constexpr ImVec4 F4_MEDIUM_BLUE = MakeRGBAFromHex(HEX_MEDIUM_BLUE);
constexpr ImVec4 F4_DARK_BLUE = MakeRGBAFromHex(HEX_DARK_BLUE);
constexpr ImVec4 F4_MIDNIGHT_BLUE = MakeRGBAFromHex(HEX_MIDNIGHT_BLUE);
constexpr ImVec4 F4_INDIGO = MakeRGBAFromHex(HEX_INDIGO);
constexpr ImVec4 F4_DARK_MAGENTA = MakeRGBAFromHex(HEX_DARK_MAGENTA);
constexpr ImVec4 F4_FUCHSIA = MakeRGBAFromHex(HEX_FUCHSIA);
constexpr ImVec4 F4_VIOLET = MakeRGBAFromHex(HEX_VIOLET);
constexpr ImVec4 F4_ORCHID = MakeRGBAFromHex(HEX_ORCHID);
constexpr ImVec4 F4_PLUM = MakeRGBAFromHex(HEX_PLUM);
constexpr ImVec4 F4_LAVENDER = MakeRGBAFromHex(HEX_LAVENDER);
constexpr ImVec4 F4_LIGHT_PINK = MakeRGBAFromHex(HEX_LIGHT_PINK);
constexpr ImVec4 F4_HOT_PINK = MakeRGBAFromHex(HEX_HOT_PINK);
constexpr ImVec4 F4_DEEP_PINK = MakeRGBAFromHex(HEX_DEEP_PINK);
constexpr ImVec4 F4_SALMON = MakeRGBAFromHex(HEX_SALMON);
constexpr ImVec4 F4_LIGHT_SALMON = MakeRGBAFromHex(HEX_LIGHT_SALMON);
constexpr ImVec4 F4_TAN = MakeRGBAFromHex(HEX_TAN);
constexpr ImVec4 F4_PERU = MakeRGBAFromHex(HEX_PERU);
constexpr ImVec4 F4_SADDLE_BROWN = MakeRGBAFromHex(HEX_SADDLE_BROWN);
constexpr ImVec4 F4_SIENNA = MakeRGBAFromHex(HEX_SIENNA);
constexpr ImVec4 F4_CHOCOLATE = MakeRGBAFromHex(HEX_CHOCOLATE);
constexpr ImVec4 F4_SANDY_BROWN = MakeRGBAFromHex(HEX_SANDY_BROWN);
constexpr ImVec4 F4_BURLYWOOD = MakeRGBAFromHex(HEX_BURLYWOOD);
constexpr ImVec4 F4_GOLDENROD = MakeRGBAFromHex(HEX_GOLDENROD);
constexpr ImVec4 F4_DARK_GOLDENROD = MakeRGBAFromHex(HEX_DARK_GOLDENROD);
constexpr ImVec4 F4_DARK_OLIVE_GREEN = MakeRGBAFromHex(HEX_DARK_OLIVE_GREEN);
constexpr ImVec4 F4_YELLOW_GREEN = MakeRGBAFromHex(HEX_YELLOW_GREEN);
constexpr ImVec4 F4_OLIVE_DRAB = MakeRGBAFromHex(HEX_OLIVE_DRAB);
constexpr ImVec4 F4_DARK_SEA_GREEN = MakeRGBAFromHex(HEX_DARK_SEA_GREEN);
constexpr ImVec4 F4_PALE_TURQUOISE = MakeRGBAFromHex(HEX_PALE_TURQUOISE);
constexpr ImVec4 F4_LIGHT_CYAN = MakeRGBAFromHex(HEX_LIGHT_CYAN);
constexpr ImVec4 F4_PALE_VIOLET_RED = MakeRGBAFromHex(HEX_PALE_VIOLET_RED);
constexpr ImVec4 F4_THISTLE = MakeRGBAFromHex(HEX_THISTLE);
constexpr ImVec4 F4_HONEYDEW = MakeRGBAFromHex(HEX_HONEYDEW);
constexpr ImVec4 F4_MINT_CREAM = MakeRGBAFromHex(HEX_MINT_CREAM);
constexpr ImVec4 F4_AZURE = MakeRGBAFromHex(HEX_AZURE);
constexpr ImVec4 F4_ALICE_BLUE = MakeRGBAFromHex(HEX_ALICE_BLUE);
constexpr ImVec4 F4_LAVENDER_BLUSH = MakeRGBAFromHex(HEX_LAVENDER_BLUSH);
constexpr ImVec4 F4_MISTY_ROSE = MakeRGBAFromHex(HEX_MISTY_ROSE);
constexpr ImVec4 F4_SEASHELL = MakeRGBAFromHex(HEX_SEASHELL);
constexpr ImVec4 F4_BISQUE = MakeRGBAFromHex(HEX_BISQUE);
constexpr ImVec4 F4_PEACH_PUFF = MakeRGBAFromHex(HEX_PEACH_PUFF);
constexpr ImVec4 F4_LEMON_CHIFFON = MakeRGBAFromHex(HEX_LEMON_CHIFFON);
constexpr ImVec4 F4_LIGHT_YELLOW = MakeRGBAFromHex(HEX_LIGHT_YELLOW);
constexpr ImVec4 F4_PALE_GOLDENROD = MakeRGBAFromHex(HEX_PALE_GOLDENROD);
constexpr ImVec4 F4_ANTIQUE_WHITE = MakeRGBAFromHex(HEX_ANTIQUE_WHITE);
constexpr ImVec4 F4_PAPAYA_WHIP = MakeRGBAFromHex(HEX_PAPAYA_WHIP);
constexpr ImVec4 F4_BLANCHED_ALMOND = MakeRGBAFromHex(HEX_BLANCHED_ALMOND);
constexpr ImVec4 F4_CORNSILK = MakeRGBAFromHex(HEX_CORNSILK);
constexpr ImVec4 F4_WHITE_SMOKE = MakeRGBAFromHex(HEX_WHITE_SMOKE);
constexpr ImVec4 F4_GAINSBORO = MakeRGBAFromHex(HEX_GAINSBORO);
constexpr ImVec4 F4_LIGHT_GRAY = MakeRGBAFromHex(HEX_LIGHT_GRAY);
constexpr ImVec4 F4_SILVER = MakeRGBAFromHex(HEX_SILVER);
constexpr ImVec4 F4_DARK_GRAY = MakeRGBAFromHex(HEX_DARK_GRAY);
constexpr ImVec4 F4_DIM_GRAY = MakeRGBAFromHex(HEX_DIM_GRAY);
constexpr ImVec4 F4_LIGHT_SLATE_GRAY = MakeRGBAFromHex(HEX_LIGHT_SLATE_GRAY);
constexpr ImVec4 F4_SLATE_GRAY = MakeRGBAFromHex(HEX_SLATE_GRAY);
constexpr ImVec4 F4_DARK_SLATE_GRAY = MakeRGBAFromHex(HEX_DARK_SLATE_GRAY);
