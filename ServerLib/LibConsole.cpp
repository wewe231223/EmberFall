#include "pch.h"
#include "LibConsole.h"

LibConsole::LibConsole() {

}

LibConsole::~LibConsole() {

}

void LibConsole::Init() {
    if (FALSE == AllocConsole()) {
        MessageBox(nullptr, L"AllocConsole Failure", L"", MB_OK);
        return;
    }

    mOldStdOut = stdout;
    mOldStdIn = stdin;
    mOldStdErr = stderr;

    freopen_s(&mConsoleOut, "CONOUT$", "w", stdout);
    freopen_s(&mConsoleIn, "CONIN$", "r", stdin);
    freopen_s(&mConsoleErr, "CONOUT$", "w", stderr);

    std::ios::sync_with_stdio();
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
}

void LibConsole::Terminate() {
    if (mConsoleOut) {
        fclose(mConsoleOut);
    }

    if (mConsoleIn) {
        fclose(mConsoleIn);
    }

    if (mConsoleErr) {
        fclose(mConsoleErr);
    }

    FreeConsole();
}


LibConsole gLibConsole{};
