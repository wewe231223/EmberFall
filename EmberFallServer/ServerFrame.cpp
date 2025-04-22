#include "pch.h"
#include "ServerFrame.h"
#include "GameTimer.h"
#include "GameObject.h"
#include "GameEventManager.h"
#include "BoundingBoxImporter.h"
#include "Input.h"

#include "PlayerScript.h"
#include "GameSession.h"
#include "ObjectManager.h"
#include "Resources.h"

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
    ResourceManager::LoadEnvFromFile("../Resources/Binarys/Collider/AnimationInfo.bin");
    BoundingBoxImporter::LoadFromFile();
    gObjectManager->Init();

    gServerCore->Init();

    mInputManager = std::make_shared<InputManager>();

    auto sessionManager = gServerCore->GetSessionManager();
    static auto createSessionFn = []() { 
        return std::make_shared<GameSession>();
    };

    sessionManager->RegisterCreateSessionFn(createSessionFn);

    gServerCore->Start("", SERVER_PORT);

    mTimerThread = std::thread{ [this]() { TimerThread(); } };
    
    gObjectManager->LoadEnvFromFile("../Resources/Binarys/Collider/env1.bin");

    for (int32_t test = 0; test < 100; ++test) {
        decltype(auto) monster = gObjectManager->SpawnObject(Packets::EntityType_MONSTER);
        std::this_thread::sleep_for(1ms);
        monster->GetTransform()->SetY(0.0f);
        monster->GetTransform()->Translate(Random::GetRandomVec3(SimpleMath::Vector3{ -100.0f, 0.0f, -100.0f }, SimpleMath::Vector3{ 100.0f, 0.0f, 100.0f }));
    }

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

void ServerFrame::AddTimerEvent(NetworkObjectIdType id, SysClock::time_point executeTime, TimerEventType eventType) {
    mTimerEvents.push(TimerEvent{ id, executeTime, eventType });
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

        auto obj = gObjectManager->GetObjectFromId(event.id);

        if (nullptr == obj) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Object Is Dead");
            continue;
        }

        switch (event.eventType) {
        case TimerEventType::UPDATE_NPC:
        {
            if (false == obj->mSpec.active) {
                break;
            }

            obj->RegisterUpdate();
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
