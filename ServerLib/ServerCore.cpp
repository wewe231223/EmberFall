#include "pch.h"
#include "ServerCore.h"

#include "IOCPCore.h"
#include "Listener.h"
#include "SessionManager.h"
#include "Session.h"
//#include "PacketHandler.h"

ServerCore::ServerCore() { }

ServerCore::~ServerCore() { }

void ServerCore::Start(const std::string& ip, const unsigned short port, size_t workerThreadNum) {
    gIocpCore->Init(workerThreadNum);
    mListener = std::make_unique<Listener>(ip, port);
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
}
