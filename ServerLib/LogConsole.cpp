#include "pch.h"
#include "LogConsole.h"

LogConsole::LogConsole() {
    mConsoleHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    mPrintThread = std::thread{ [=]() { Worker(); } };
}

LogConsole::~LogConsole() {
    mPrintLoop = false;
    if (mPrintThread.joinable()) {
        mPrintThread.join();
    }
}

void LogConsole::SetConsoleColor(ConsoleColor text, ConsoleColor background) {
    UINT16 colorCode = static_cast<UINT16>(text) + static_cast<UINT16>(background) * 0x10;
    ::SetConsoleTextAttribute(mConsoleHandle, colorCode);
}

void LogConsole::PrintLog(const Log& log) {
    switch (log.level) {
    case DebugLevel::LEVEL_DEBUG:
        SetConsoleColor(ConsoleColor::WHITE, ConsoleColor::BLACK);
        break;

    case DebugLevel::LEVEL_INFO:
        SetConsoleColor(ConsoleColor::GREEN, ConsoleColor::BLACK);
        break;

    case DebugLevel::LEVEL_WARNING:
        SetConsoleColor(ConsoleColor::RED, ConsoleColor::BLACK);
        break;

    case DebugLevel::LEVEL_ERROR:
        SetConsoleColor(ConsoleColor::RED, ConsoleColor::BLACK);
        break;

    case DebugLevel::LEVEL_FATAL:
        SetConsoleColor(ConsoleColor::YELLOW, ConsoleColor::RED);
        break;

    default:
        break;
    }
    
    std::cout << log.text << std::endl;
    SetConsoleColor(ConsoleColor::WHITE, ConsoleColor::BLACK);
}

void LogConsole::Worker() {
    Log log{ };
    while (mPrintLoop) {
        if (false == mLogQueue.try_pop(log)) {
            std::this_thread::yield();
            continue;
        }

        PrintLog(log);
    }
}
