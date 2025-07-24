#pragma once
#include <windows.h>
#include <iostream>
#include <fstream>

class LibConsole {
public:
    LibConsole();
    ~LibConsole();

public:
    void Init(); 

    void Terminate(); 
private:
    FILE* mOldStdOut = nullptr;
    FILE* mOldStdIn = nullptr;
    FILE* mOldStdErr = nullptr;
    FILE* mConsoleOut = nullptr;
    FILE* mConsoleIn = nullptr;
    FILE* mConsoleErr = nullptr;
};

extern LibConsole gLibConsole; 