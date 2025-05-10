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

ServerFrame::ServerFrame() {}

ServerFrame::~ServerFrame() { 
    gServerCore->End();
}

std::shared_ptr<class InputManager> ServerFrame::GetInputManager() const {
    return mInputManager;
}

void ServerFrame::Run() {
    ResourceManager::LoadEnvFromFile("../Resources/Binarys/Collider/EnvBB.bin");
    ResourceManager::LoadEntityFromFile("../Resources/Binarys/Collider/Entitybb.bin");
    ResourceManager::LoadAnimationFromFile("../Resources/Binarys/Collider/AnimationInfo.bin");

    gServerCore->Init();

    mInputManager = std::make_shared<InputManager>();

    auto sessionManager = gServerCore->GetSessionManager();
    static auto createSessionFn = []() { 
        return std::make_shared<GameSession>();
    };

    sessionManager->RegisterCreateSessionFn(createSessionFn);

    gServerCore->Start("", SERVER_PORT);

    mTimerThread = std::thread{ [this]() { TimerThread(); } };
 
    gGameRoomManager->InitGameRooms();

    while (not mDone);
}

void ServerFrame::Done() {
    mDone = true;
    if (mTimerThread.joinable()) {
        mTimerThread.join();
    }
}

void ServerFrame::PQCS(int32_t transfferedBytes, ULONG_PTR completionKey, OverlappedEx* overlapped) {
    gServerCore->PQCS(transfferedBytes, completionKey, overlapped);
}

void ServerFrame::AddTimerEvent(uint16_t roomIdx, NetworkObjectIdType id, SysClock::time_point executeTime, TimerEventType eventType, ExtraInfo info) {
    mTimerEvents.push(TimerEvent{ roomIdx, id, executeTime, eventType, info });
}

bool ServerFrame::IsGameRoomEvent(TimerEventType type) const {
    return static_cast<uint8_t>(type) & GAME_ROOM_EVENT;
}

void ServerFrame::TimerThread() {
    while (not mDone) {
        auto currentTime = SysClock::now();

        TimerEvent event;
        if (false == mTimerEvents.try_pop(event)) {
            std::this_thread::sleep_for(1ms);
            continue;
        }

        if (event.executeTime > currentTime) {
            mTimerEvents.push(event);
            std::this_thread::sleep_for(1ms);
            continue;
        }

        decltype(auto) gameRoom = gGameRoomManager->GetRoom(event.roomIdx);
       
        std::shared_ptr<GameObject> obj{ };
        if (not IsGameRoomEvent(event.eventType)) {
            obj = gameRoom->GetStage().GetObjectFromId(event.id);
            if (nullptr == obj) {
                gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Object Is Dead");
                continue;
            }
        }

        switch (event.eventType) {
        case TimerEventType::SCENE_TRANSITION_COUNTDOWN:
        {
            gameRoom->OnSceneCountdownTick();
            break;
        }

        case TimerEventType::CHECK_GAME_CONDITION:
        {
            gameRoom->CheckGameEnd();
            break;
        }

        case TimerEventType::CHECK_SESSION_HEART_BEAT:
        {
            gameRoom->CheckSessionsHeartBeat();
            break;
        }

        case TimerEventType::UPDATE_NPC:
        {
            if (false == obj->mSpec.active) {
                break;
            }

            obj->RegisterUpdate();
            break;
        }

        case TimerEventType::REMOVE_PLAYER_IN_ROOM:
        {
            gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Remove Player!!!!");
            auto lobbyInfo = std::get<SessionLobbyInfo>(event.extraInfo);
            gameRoom->RemovePlayer(static_cast<SessionIdType>(event.id), lobbyInfo.lastRole, lobbyInfo.readyState, lobbyInfo.sessionSlot);
            break;
        }

        case TimerEventType::REMOVE_NPC:
        {
            obj->Reset();
            break;
        }

        case TimerEventType::REMOVE_TRIGGER:
        {
            obj->Reset();
            break;
        }

        default:
            break;
        }
    }
}
