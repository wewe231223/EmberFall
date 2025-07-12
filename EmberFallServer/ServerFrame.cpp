#include "pch.h"
#include "ServerFrame.h"
#include "GameTimer.h"
#include "GameObject.h"
#include "BoundingBoxImporter.h"
#include "Input.h"

#include "PlayerScript.h"
#include "GameSession.h"
#include "ObjectManager.h"
#include "Resources.h"
#include "GameRoom.h"

#include "SessionEbr.h"

ServerFrame::ServerFrame() {}

ServerFrame::~ServerFrame() { 
    Done();
}

std::shared_ptr<InputManager> ServerFrame::GetInputManager() const {
    return mInputManager;
}

bool ServerFrame::StartServer() {
    WSADATA data{ };
    if (0 != ::WSAStartup(MAKEWORD(2, 2), &data)) {
        return false;
    }

    mListener.Init();
    mIocpCore.Init(mWorkerThreadNum);

    auto listenSocket = mListener.GetListenSocket();
    mIocpCore.RegisterSocket(listenSocket, SYSTEM_ID);
    mListener.RegisterAccept();

    return true;
}

void ServerFrame::Run() {
    ResourceManager::LoadEnvFromFile("../Resources/Binarys/Collider/ServerEnvironmentBB.bin");
    ResourceManager::LoadEntityFromFile("../Resources/Binarys/Collider/Entitybb.bin");
    ResourceManager::LoadAnimationFromFile("../Resources/Binarys/Collider/AnimationInfo.bin");

    StartServer();

    mInputManager = std::make_shared<InputManager>();
    gGameRoomManager->InitGameRooms();

    mWorkerThreadNum = 0 == mWorkerThreadNum ? std::thread::hardware_concurrency() : mWorkerThreadNum;
    auto workerThreadNum = mWorkerThreadNum;
    for (int i = 0; i < workerThreadNum; ++i) {
        mWorkerThreads.emplace_back(std::thread{ [=]() { IoThread(); } });
    }

    mTimerThread = std::thread{ [=]() { TimerThread(); } };
}

void ServerFrame::Done() {
    if (mTimerThread.joinable()) {
        mTimerThread.join();
    }

    for (auto& thread : mWorkerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    mListener.Close();

    ::WSACleanup();
}

GameSession* ServerFrame::GetSession(SessionIdType id) {
    return mSessionManager.GetSession(id);
}

void ServerFrame::CheckSessionsHeartBeat(std::vector<SessionIdType>& sessions) {
    mSessionManager.CheckSessionsHeartBeat(sessions);
}

void ServerFrame::CloseSession(SessionIdType id) {
    return mSessionManager.CloseSession(id);
}

void ServerFrame::Send(SessionIdType to, OverlappedSend* packet) {
    mSessionManager.Send(to, packet);
}

void ServerFrame::PQCS(int32_t transfferedBytes, ULONG_PTR completionKey, OverlappedEx* overlapped) {
    ::PostQueuedCompletionStatus(mIocpCore.GetHandle(), transfferedBytes, completionKey, overlapped);
}

void ServerFrame::AddTimerEvent(NetworkObjectIdType id, SysClock::duration delay, IoType eventType, ExtraInfo info) {
    auto insertPair = std::make_pair(SysClock::now() + delay, TimerEvent{ id, eventType, info });

    std::lock_guard timerEventGuard{ mTimerMapLock };
    mTimerEvents.insert(insertPair);
}

bool ServerFrame::IsGameRoomEvent(IoType type) const {
    return static_cast<uint8_t>(type) & GAME_ROOM_EVENT;
}

void ServerFrame::IoThread() {
    InitTls();

#ifdef DEBUG
    static SessionIdType lastErrorClient{ INVALID_SESSION_ID };
#endif
    DWORD receivedByte{ };
    ULONG_PTR completionKey{ };
    OVERLAPPED* overlapped{ nullptr };

    SessionEbrGuard sessionGuard{ gSessionEbr, lThreadId };
    while (true) {
        auto success = ::GetQueuedCompletionStatus(
            mIocpCore.GetHandle(),
            &receivedByte,
            &completionKey,
            &overlapped,
            INFINITE
        );

        OverlappedEx* overlappedEx = reinterpret_cast<OverlappedEx*>(overlapped);
        if (nullptr == overlappedEx) {
            continue;
        }

        auto ioType = overlappedEx->type;
        SessionIdType clientId = static_cast<SessionIdType>(completionKey);
        if (not success) {
            if (IoType::ACCEPT == ioType) {
                gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Accept Error!!");
                gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "{}", NetworkUtil::WSAErrorMessage());
                Crash("Accept Error");
            }
            else if (IoType::CONNECT == ioType) {
                gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Connect Error!!");
                MessageBoxA(nullptr, NetworkUtil::WSAErrorMessage().c_str(), "", MB_OK);
                Crash("Connect Error");
            }

            if (IoType::SEND == ioType) {
                FbsPacketFactory::ReleasePacketBuf(reinterpret_cast<OverlappedSend*>(overlappedEx));
#ifdef DEBUG
                if (lastErrorClient != clientId) {
                    gLogConsole->PushLog(DebugLevel::LEVEL_FATAL, "Client[{}] Error Send", static_cast<int32_t>(clientId));
                }

                lastErrorClient = clientId;
#endif
            }

            auto reuseTarget = mSessionManager.GetSession(clientId);
            if (nullptr == reuseTarget) {
                continue;
            }

            mSessionManager.CloseSession(clientId);
            gSessionEbr.PushPointer(reuseTarget);
            continue;
        }

        if ((IoType::SEND == ioType or IoType::RECV == ioType) and 0 >= receivedByte) {
            if (IoType::SEND == ioType) {
                FbsPacketFactory::ReleasePacketBuf(reinterpret_cast<OverlappedSend*>(overlappedEx));
            }

            auto reuseTarget = mSessionManager.GetSession(clientId);
            if (nullptr == reuseTarget) {
                continue;
            }

            mSessionManager.CloseSession(clientId);
            gSessionEbr.PushPointer(reuseTarget);
            continue;
        }

        switch (ioType) {
        case IoType::RECV:
        {
            auto session = mSessionManager.GetSession(clientId);
            if (nullptr == session) {
                break;
            }

            session->ProcessRecv(receivedByte);
        }
        break;

        case IoType::SEND:
        {
            FbsPacketFactory::ReleasePacketBuf(reinterpret_cast<OverlappedSend*>(overlappedEx));
        }
        break;

        case IoType::ACCEPT:
        {
            auto overlappedAccept = reinterpret_cast<OverlappedAccept*>(overlappedEx);
            //auto session = new GameSession{ overlappedAccept->connectedSocket };
            auto sesison = gSessionEbr.PopPointer<GameSession>(overlappedAccept->connectedSocket);

            auto [id, result] = mSessionManager.AddSession(overlappedAccept);
            if (SYSTEM_ID == id or nullptr == result) {
                gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Client Connect Failure");
                mListener.RegisterAccept();
                break;
            }

            mIocpCore.RegisterSocket(reinterpret_cast<SOCKET>(result->GetHandle()), result->GetId());
            //result->RegisterRecv();

            mListener.RegisterAccept();
        }
        break;

        case IoType::SCENE_TRANSITION_COUNTDOWN:
        {
            auto roomIdx = std::get<int64_t>(overlappedEx->extraInfo);
            gGameRoomManager->GetRoom(roomIdx)->OnSceneCountdownTick();
        }
        break;

        case IoType::CHECK_GAME_CONDITION:
        {
            auto roomIdx = std::get<int64_t>(overlappedEx->extraInfo);
            gGameRoomManager->GetRoom(roomIdx)->CheckGameEnd();
        }
        break;

        case IoType::CHECK_SESSION_HEART_BEAT:
        {
            auto roomIdx = std::get<int64_t>(overlappedEx->extraInfo);
            gGameRoomManager->GetRoom(roomIdx)->CheckSessionsHeartBeat();
        }
        break;

        case IoType::UPDATE_NPC:
        {
            auto objId = static_cast<NetworkObjectIdType>(completionKey);
            auto roomId = std::get<int64_t>(overlappedEx->extraInfo);
            auto obj = gGameRoomManager->GetRoom(roomId)->GetStage().GetObjectManager()->GetObjectFromId(objId);
            if (nullptr == obj) {
                break;
            }
            
            obj->Update();
        }
        break;

        case IoType::REMOVE_PLAYER_IN_ROOM:
        {
            gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Remove Player!!!!");
            auto lobbyInfo = std::get<SessionLobbyInfo>(overlappedEx->extraInfo);
            decltype(auto) gameRoom = gGameRoomManager->GetRoom(lobbyInfo.roomIdx);
            gameRoom->RemovePlayer(static_cast<SessionIdType>(completionKey), lobbyInfo.lastRole, lobbyInfo.readyState, lobbyInfo.sessionSlot);
        }
        break;

        case IoType::REMOVE_NPC:
        {
            auto objId = static_cast<NetworkObjectIdType>(completionKey);
            auto roomId = std::get<int64_t>(overlappedEx->extraInfo);
            auto obj = gGameRoomManager->GetRoom(roomId)->GetStage().GetObjectManager()->GetObjectFromId(objId);
            if (nullptr == obj) {
                break;
            }

            obj->Reset();
        }
        break;

        case IoType::REMOVE_TRIGGER:
        {
            auto objId = static_cast<NetworkObjectIdType>(completionKey);
            auto roomId = std::get<int64_t>(overlappedEx->extraInfo);
            auto obj = gGameRoomManager->GetRoom(roomId)->GetStage().GetObjectManager()->GetObjectFromId(objId);
            if (nullptr == obj) {
                break;
            }

            obj->Reset();
        }
        break;

        default:
            break;
        }
    }

    ClearTls();
}

void ServerFrame::TimerThread() {
    InitTls();

    while (not mDone) {
        auto current_time = std::chrono::system_clock::now();
        mTimerMapLock.lock();
        if (mTimerEvents.empty()) {
            mTimerMapLock.unlock();
            std::this_thread::sleep_for(1ms);
            continue;
        }

        auto beg = mTimerEvents.begin();
        auto end = mTimerEvents.upper_bound(current_time);
        size_t executableCnt = std::distance(beg, end);
        if (0 == executableCnt) {
            mTimerMapLock.unlock();
            std::this_thread::sleep_for(1ms);
            continue;
        }

        std::vector<TimerEvent> eventList;
        eventList.reserve(executableCnt);

        std::transform(beg, end, std::back_inserter(eventList), [](const auto& pair) { return pair.second; }); // 순회를 위한 벡터에 저장
        mTimerEvents.erase(beg, end);
        mTimerMapLock.unlock(); // 더이상의 보호는 불필요

        for (auto& ev : eventList) { // 실행할 수 있는건 모두 실행
            OverlappedEx* ov = new OverlappedEx;
            ov->type = ev.eventType;
            ov->extraInfo = ev.extraInfo;
            ::PostQueuedCompletionStatus(mIocpCore.GetHandle(), 1, ev.id, ov->GetRawPtr());
        }
        std::this_thread::sleep_for(1ms);
    }

    ClearTls();
}
