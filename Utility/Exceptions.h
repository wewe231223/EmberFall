#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Exeptions.h
// 2025.01.05 김승범   - HRESULT 의 값을 토대로 에러 메세지를 얻어와 MessageBox 를 띄우는 함수를 정의함. 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <source_location>
#include <optional>
#include <string>
#include <format>
#include <comdef.h>

inline std::wstring to_wstring(const std::string& str)
{
	return std::wstring{ str.begin(), str.end() };
}

using HRESULT = long;
[[maybe_unused]]
inline std::optional<std::wstring> CheckHR(const HRESULT& hr, const std::source_location& location = std::source_location::current()) {
	std::optional<std::wstring> result{ std::nullopt };

	if (FAILED(hr)) {
		_com_error err{ hr };
		result = std::format(L"Error Location : {} : {}\n{} \n\n Error : \n{}",
			to_wstring(location.file_name()),
			location.line(),
			to_wstring(location.function_name()),
			err.ErrorMessage());
		::MessageBoxW(nullptr, result.value().c_str(), L"Error", 0x00000010L | 0x00000000L);
		PostQuitMessage(0);
	} 

	return result;
}
