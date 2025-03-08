#pragma once 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Serializer.h
// 2025.01.16 김승범   - 메모리에 있는 내용을 이진 파일로 직렬화 하는 기능을 가진 클래스를 정의함  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <type_traits>
#include <concepts>
#include <ranges> 
#include "Crash.h"

class Serializer {
public:
    explicit Serializer(const std::string& fileName) {
        std::string filePath = fileName.ends_with(".bin") ? fileName : fileName + ".bin";
        mFile.open(filePath, std::ios::binary);
        CrashExp(mFile.is_open(), "Failed to make file");
    }

    ~Serializer() = default;

	Serializer& operator=(const Serializer&) = default;
	Serializer(const Serializer&) = default;

	Serializer(Serializer&&) = default;
	Serializer& operator=(Serializer&&) = default;
public:
    template <typename T>
        requires std::is_trivial_v<T>&& std::is_standard_layout_v<T>
    void Write(const T& data) {
        mFile.write(reinterpret_cast<const char*>(&data), sizeof(T));
    }

    template <typename CharT, typename Traits, typename Allocator>
    void Write(const std::basic_string<CharT, Traits, Allocator>& str) {
        size_t size = str.size();
        Write(size);
        mFile.write(reinterpret_cast<const char*>(str.data()), size * sizeof(CharT));
    }

    template <std::ranges::range Range>
        requires (!std::same_as<Range, std::string>) 
    void Write(const Range& container) {
        size_t size = std::ranges::distance(container);
        Write(size); 
        for (const auto& element : container | std::views::all) {
            Write(element); 
        }
    }
private:
    std::ofstream mFile{};
};
