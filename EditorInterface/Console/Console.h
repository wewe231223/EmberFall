#pragma once 
#include "../Utility/InputBuffer.h"

class ConsoleClass {
public:
	static constexpr size_t CONSOLE_BUFFER_SIZE{ 4096 };
public:
	ConsoleClass();
	~ConsoleClass();

	ConsoleClass(const ConsoleClass& other) = delete;
	ConsoleClass& operator=(const ConsoleClass& other) = delete;

	ConsoleClass(ConsoleClass&& other) = delete;
	ConsoleClass& operator=(ConsoleClass&& other) = delete;
public:
	
public:
private:
	InputBuffer<std::string, CONSOLE_BUFFER_SIZE> mConsoleBuffer{};
};

extern ConsoleClass Console;