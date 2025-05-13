////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Console.h
// 2025.01.06 김승범   - 프로그램의 전체적인 내부 정보 확인을 위한 Console 클래스를 정의함. 
// 2025.01.07 김승범	  - Console 에 메세지를 넣는 Log 함수를 정의하고, Console 클래스의 이름을 변경함. 
// 2025.01.08 김승범   - Console 클래스의 이름을 ConsoleBase 로 변경함.
//						Render 함수와 Log 함수에 mutex 를 사용하여 안전하게 동시성을 지원하도록 변경하였다. 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once 
#include <corecrt_wtime.h>
#include <fstream>
#include <format>
#include <chrono>
#include <mutex>
#include <filesystem>
#include <queue>
#include "../Utility/CircularBuffer.h"

enum class LogType : unsigned char {
	Info,
	Warning,
	Error,
};

class ConsoleBase {
	static constexpr size_t CONSOLE_BUFFER_SIZE{ 4096 };
public:
	ConsoleBase();
	~ConsoleBase();

	ConsoleBase(const ConsoleBase& other) = delete;
	ConsoleBase& operator=(const ConsoleBase& other) = delete;

	ConsoleBase(ConsoleBase&& other) = delete;
	ConsoleBase& operator=(ConsoleBase&& other) = delete;
public:
	void Init(); 

	template<typename... Args>
	void Log(std::format_string<Args...> fmt, LogType type, Args&&... args) {
		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::tm local_time{};
		localtime_s(&local_time, &now_time);

		switch (type) {
		case LogType::Info:
		{
			std::lock_guard lock{ mConsoleLock };
			decltype(auto) msg = mBuffer.EmplaceBack(
				std::make_pair(
					std::make_pair(std::format("[ INFO - {:02}:{:02}:{:02} ] ", local_time.tm_hour, local_time.tm_min, local_time.tm_sec), std::format(fmt, std::forward<Args>(args)...)), type)
			);

			mLogFile << msg.first.first << " : " << msg.first.second << std::endl;
			
		}
			break; 
		case LogType::Warning:
		{
			std::lock_guard lock{ mConsoleLock };
			decltype(auto) msg = mBuffer.EmplaceBack(
				std::make_pair(
					std::make_pair(std::format("[ WARN - {:02}:{:02}:{:02} ] ", local_time.tm_hour, local_time.tm_min, local_time.tm_sec), std::format(fmt, std::forward<Args>(args)...)), type)
			);

			mLogFile << msg.first.first << " : " << msg.first.second << std::endl;
		}
			break;
		case LogType::Error:
		{
			std::lock_guard lock{ mConsoleLock };
			decltype(auto) msg = mBuffer.EmplaceBack(
				std::make_pair(
					std::make_pair(std::format("[ ERROR - {:02}:{:02}:{:02} ] ", local_time.tm_hour, local_time.tm_min, local_time.tm_sec), std::format(fmt, std::forward<Args>(args)...)), type)
			);

			mLogFile << msg.first.first << " : " << msg.first.second << std::endl;
		}
			break;
		default:
			Crash("Invalid LogType");
			break;
		}
	}
public:
	void Render();
private:
	void ManageLogFile();
private:
	CircularBuffer<std::pair<std::pair<std::string, std::string>,LogType>, CONSOLE_BUFFER_SIZE> mBuffer{};
	std::ofstream mLogFile{};
	std::mutex mConsoleLock{};
};

extern ConsoleBase Console;