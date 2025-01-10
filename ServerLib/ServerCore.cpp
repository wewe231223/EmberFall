#include "pch.h"
#include "ServerCore.h"

#include "Listener.h"
#include "SessionManager.h"
#include "Session.h"
//#include "PacketHandler.h"

ServerCore::ServerCore() { }
    //: mClientManager{ std::make_unique<SessionManager>() }, mPacketHandler{ std::make_unique<PacketHandler>() } { }

ServerCore::~ServerCore() { }

void ServerCore::Start(const std::string& ip, const unsigned short port) {
    //mListener = std::make_unique<Listener>(ip, port);
    //mListener->RegisterAccept();
}

void ServerCore::IOWorker() {
    DWORD receivedByte{ };
    ULONG_PTR completionKey{ };
    OVERLAPPED* overlapped{ nullptr };

    while (true) {
        auto success = ::GetQueuedCompletionStatus(
            INVALID_HANDLE_VALUE, 
            &receivedByte, 
            &completionKey, 
            &overlapped, 
            INFINITE
        );

        OverlappedEx* overlappedEx = reinterpret_cast<OverlappedEx*>(overlapped);
        SessionIdType clientId = static_cast<SessionIdType>(completionKey);

        if (not success) {
            if (IOType::ACCEPT == overlappedEx->type) {
                if (overlappedEx->mOwner.expired()) {
                    // TODO
                }
                continue;
            }
            else {
                // TODO
                //mClientManager->CloseSession(clientId);
                continue;
            }
        }

        std::shared_ptr<INetworkObject> owener = overlappedEx->mOwner.lock();
    }
}