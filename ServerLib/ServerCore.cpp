#include "pch.h"
#include "ServerCore.h"

#include "IOCPCore.h"
#include "Listener.h"
#include "SessionManager.h"
#include "Session.h"

ServerCore::ServerCore() { }

ServerCore::~ServerCore() { }

void ServerCore::Start(const unsigned short port, size_t workerThreadNum) {
    WSADATA data{ };
    if (0 != ::WSAStartup(MAKEWORD(2, 2), &data)) {
        return;
    }

    gIocpCore->Init(workerThreadNum);
    mListener = std::make_unique<Listener>(port);
    gIocpCore->RegisterSocket(mListener);
    mListener->RegisterAccept();

    for (size_t i = 0; i < workerThreadNum; ++i) {
        mWorkerThreads.emplace_back(
            [=]() 
            {
                gIocpCore->IOWorker();
            }
        );
    }
}

void ServerCore::End() {
    for (auto& thread : mWorkerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    ::WSACleanup();
}
