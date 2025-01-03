#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Config.h
// 2025.01.03 김승범   - 윈도우 창 크기 와 같은 프로그램의 상수를 저장할 장소를 생성함.  
//                      
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "IdentityConfig.h"

#ifdef 김승범컴퓨터 
#define FHD
#elif defined(김성준컴퓨터) 
#define HD
#endif 

#ifdef FHD
struct Config {
	template<typename T = int> 
	constexpr static T WINDOW_WIDTH{ static_cast<T>(1920) };

	template<typename T = int> 
	constexpr static T WINDOW_HEIGHT{ static_cast<T>(1080) };

	template<typename T = int> 
	constexpr static T EDITOR_WINDOW_WIDTH{ static_cast<T>(Config::WINDOW_WIDTH<T> / 3) };

	template<typename T = int>
	constexpr static T EDITOR_WINDOW_HEIGHT{ Config::WINDOW_HEIGHT<T> };
};
#endif 

#ifdef HD
struct Config {
	template<typename T = int>
	constexpr static T WINDOW_WIDTH{ static_cast<T>(1280) };

	template<typename T = int>
	constexpr static T WINDOW_HEIGHT{ static_cast<T>(720) };

	template<typename T = int>
	constexpr static T EDITOR_WINDOW_WIDTH{ static_cast<T>(Config::WINDOW_WIDTH<T> / 3) };

	template<typename T = int>
	constexpr static T EDITOR_WINDOW_HEIGHT{ Config::WINDOW_HEIGHT<T> };
};
#endif 