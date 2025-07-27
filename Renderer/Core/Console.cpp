#include <corecrt_wtime.h>
#include <format>
#include <chrono>
#include <algorithm>
#include <iostream>
#include "pch.h"
#include "Console.h"

#include "../Config/Config.h"


ConsoleBase::ConsoleBase() {
	ConsoleBase::Init(); 
}

ConsoleBase::~ConsoleBase() {

}

void ConsoleBase::Init() {
	ConsoleBase::ManageLogFile();

	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);


	std::tm localTime{};
	localtime_s(&localTime, &in_time_t);



	std::string fileName{ "Log/" + std::format("{:04}-{:02}-{:02}_{:02}-{:02}-{:02}_Log.txt",
		localTime.tm_year + 1900,
		localTime.tm_mon + 1,
		localTime.tm_mday,
		localTime.tm_hour,
		localTime.tm_min,
		localTime.tm_sec) };


	mLogFile = std::ofstream{ fileName };

	CrashExp(mLogFile.is_open(), "로그 파일을 생성할 수 없습니다.");

	mLogFile << std::format("Log : {:04}-{:02}-{:02} {:02}:{:02}:{:02}\n",
		localTime.tm_year + 1900,
		localTime.tm_mon + 1,
		localTime.tm_mday,
		localTime.tm_hour,
		localTime.tm_min,
		localTime.tm_sec) << std::endl;

	ConsoleBase::Log("{} 에 로그 파일이 생성되었습니다.", LogType::Info, fileName);
}

void ConsoleBase::Render() {

}



void ConsoleBase::ManageLogFile() {
	if (not std::filesystem::exists(Config::LOG_FILE_PATH) or not std::filesystem::is_directory(Config::LOG_FILE_PATH)) {
		std::filesystem::create_directory(Config::LOG_FILE_PATH);
		return;
	}

	struct CompareFileTime {
		bool operator()(const std::pair<std::filesystem::path, std::filesystem::file_time_type>& a,
			const std::pair<std::filesystem::path, std::filesystem::file_time_type>& b) const {
			return a.second > b.second;
		}
	};

	std::priority_queue<std::pair<std::filesystem::path, std::filesystem::file_time_type>, std::vector<std::pair<std::filesystem::path, std::filesystem::file_time_type>>, CompareFileTime> fileQueue{};

	std::size_t fileCount = 0;
	for (const auto& entry : std::filesystem::directory_iterator(Config::LOG_FILE_PATH)) {
		if (std::filesystem::is_regular_file(entry)) {
			fileQueue.emplace(entry.path(), std::filesystem::last_write_time(entry));
			++fileCount;
		}
	}

	if (fileCount <= 0) {
		return;
	}

	decltype(auto) oldestFile = fileQueue.top();
	fileQueue.pop();

	
	if (fileCount >= Config::LOG_FILE_COUNT_LIMIT<size_t>) {
		ConsoleBase::ConsoleBase::Log("{} 파일이 삭제되었습니다.", LogType::Info, oldestFile.first.string());
		std::filesystem::remove(oldestFile.first);
	}
}


ConsoleBase Console{};