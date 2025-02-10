#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////
//
// LogConsole.h
// 
// 2025 - 02 - 11 : Log 출력을 위한 클래스
//                  Log 출력은 따로 쓰레드를 하나 더 생성해서 실행
// 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class DebugLevel : BYTE {
    LEVEL_FATAL,
    LEVEL_ERROR,
    LEVEL_WARNING,
    LEVEL_DEBUG,
    LEVEL_INFO,
};

enum class ConsoleColor : short { // 상수로 사용하기 위해 enum으로 변경
    BLACK,
    DARK_BLUE,
    DARK_GREEN,
    DARK_SKYBLUE,
    DARK_RED,
    DARK_VOILET,
    DAKR_YELLOW,
    GRAY,
    DARK_GRAY,
    BLUE,
    GREEN,
    SKYBLUE,
    RED,
    VIOLET,
    YELLOW,
    WHITE,
};

struct Log {
    DebugLevel level;
    std::string text;
};

class LogConsole {
private:
    inline static std::array<std::string, 5> LOG_HEADERS{
        "[ FATAL ] ",
        "[ ERROR ] ",
        "[ WARN  ] ",
        "[ DEBUG ] ",
        "[ INFO  ] ",
    };

public:
    LogConsole();
    ~LogConsole();

public:
    template <typename... Args>
    void PushLog(DebugLevel level, std::format_string<Args...> format, Args&&... args) {
        mLogQueue.push(Log{ level, LOG_HEADERS[static_cast<BYTE>(level)] + std::format(format, std::forward<Args>(args)...) });
    }

private:
    void SetConsoleColor(ConsoleColor text, ConsoleColor background);
    void PrintLog(const Log& log);
    void Worker();

private:
    volatile bool mPrintLoop{ true };
    HANDLE mConsoleHandle{ };
    std::thread mPrintThread{ };
    Concurrency::concurrent_queue<Log> mLogQueue;
};

