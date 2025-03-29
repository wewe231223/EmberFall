#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Config.h
// 2025.01.03 김승범   - 윈도우 창 크기 와 같은 프로그램의 상수를 저장할 장소를 생성함.  
// 2025.01.05 김승범   - 한글 폰트 로딩을 위한 경로를 저장함. 
//                      
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <d3dcommon.h>
#include <dxgiformat.h>
#include "IdentityConfig.h"

#ifdef 김승범컴퓨터 
#define FHD
#elif defined(김성준컴퓨터) 
#define HD
#elif defined(정영기컴퓨터) 
#define HD
#endif 


struct Config {
#ifdef FHD
	template<typename T = int> 
	constexpr static T WINDOW_WIDTH{ static_cast<T>(1920) };

	template<typename T = int> 
	constexpr static T WINDOW_HEIGHT{ static_cast<T>(1080) };
#elif defined(HD)
	template<typename T = int>
	constexpr static T WINDOW_WIDTH{ static_cast<T>(1280) };

	template<typename T = int>
	constexpr static T WINDOW_HEIGHT{ static_cast<T>(720) };
#endif
	template<typename T = int> 
	constexpr static T EDITOR_WINDOW_WIDTH{ static_cast<T>(Config::WINDOW_WIDTH<T> / 2) };

	template<typename T = int>
	constexpr static T EDITOR_WINDOW_HEIGHT{ Config::WINDOW_HEIGHT<T> };

	constexpr static bool WINDOWED{ true };
	constexpr static bool AUTOMATIC_CLOSE{ WINDOWED ? false : true };

	constexpr static const char* LOG_FILE_PATH{ "Log/" };
	template <typename T = int>
	constexpr static T LOG_FILE_COUNT_LIMIT{ static_cast<T>(10) };

	template<typename T = int> 
	constexpr static T BACKBUFFER_COUNT{ static_cast<T>(3) };

	template<typename T = UINT> 
	constexpr static T MAX_TEXTURE_COUNT{ static_cast<T>(1024) };

	template<typename T = size_t> 
	constexpr static T GBUFFER_COUNT{ static_cast<T>(3) };

	template<typename T = size_t> 
	constexpr static T MAX_BONE_COUNT_PER_INSTANCE{ static_cast<T>(100) };

	constexpr static bool ALLOW_TEARING{ true };
	constexpr static D3D_FEATURE_LEVEL DIRECTX_FEATURE_LEVEL{ D3D_FEATURE_LEVEL_11_0 };
	constexpr static DXGI_FORMAT RENDER_TARGET_FORMAT{ DXGI_FORMAT_R8G8B8A8_UNORM };

	constexpr static bool IMGUI_DARK_THEME{ true };
	constexpr static bool IMGUI_KOREAN{ true };

	constexpr static const char* IMGUI_KOREAN_FONT_PATH{ "Resources/Font/NotoSansKR-Regular-Hestia.otf" };

	constexpr static bool DEFAULT_REVERSE_Z{ false };
};


