#include "pch.h"
#include "PlayerScript.h"
#include "Input.h"

#include "GameObject.h"
#include "ServerGameScene.h"
#include "GameEventManager.h"

PlayerScript::PlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input, std::shared_ptr<SessionManager> sessionManager)
    : Script{ owner }, mInput{ input }, mViewList{ static_cast<SessionIdType>(owner->GetId()), sessionManager } {}

PlayerScript::~PlayerScript() { }

void PlayerScript::ResetGameScene(std::shared_ptr<IServerGameScene> gameScene) {
    mGameScene = gameScene;
    mViewList.mCurrentScene = gameScene;
}

std::shared_ptr<IServerGameScene> PlayerScript::GetCurrentScene() const {
    return mGameScene;
}

void PlayerScript::Update(const float deltaTime) {
    mViewList.mPosition = GetOwner()->GetPosition();
    mViewList.Update();
    mViewList.Send();

    auto physics = GetPhysics();

    SimpleMath::Vector3 moveDir{ SimpleMath::Vector3::Zero };
    if (mInput->IsActiveKey('A')) {
        moveDir.x -= 1.0f;
    }

    if (mInput->IsActiveKey('D')) {
        moveDir.x += 1.0f;
    }

    if (mInput->IsActiveKey('W')) {
        moveDir.z -= 1.0f;
    }

    if (mInput->IsActiveKey('S')) {
        moveDir.z += 1.0f;
    }

    if (mInput->IsDown(VK_SPACE)) {
        physics->Jump(deltaTime);
    }

    if (mInput->IsDown(VK_F1)) {
        physics->mFactor.mass += 10.0f;
    }

    if (mInput->IsDown(VK_F2)) {
        physics->mFactor.acceleration += 1.0f;
    }

    if (mInput->IsDown(VK_SHIFT)) {
        auto maxSpeed = GameUnits::UnitCast<GameUnits::KilloMeterPerHour>(50.0mps);
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player [{}] Change Max Speed to {} km/h",
            GetOwner()->GetId(),
            maxSpeed.Count());
        physics->mFactor.maxMoveSpeed = maxSpeed.Count();
    }
    else if (mInput->IsUp(VK_SHIFT)) {
        auto maxSpeed = GameUnits::UnitCast<GameUnits::KilloMeterPerHour>(5.0mps);
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player [{}] Change Max Speed to {} km/h", 
            GetOwner()->GetId(),
            maxSpeed.Count());
        physics->mFactor.maxMoveSpeed = maxSpeed.Count();
    }

    if (mInput->IsUp(VK_F1)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Push GameEvent: Attack All Monster");

        auto event = std::make_shared<AttackEvent>();
        event->type = GameEventType::ATTACK_EVENT;
        event->sender = GetOwner()->GetId();
        event->damage = 10.0f;
        gEventManager->PushEvent(event);
    }

    moveDir.Normalize();
    physics->Acceleration(moveDir, deltaTime);
}

void PlayerScript::LateUpdate(const float deltaTime) {
    mInput->Update();
}

void PlayerScript::OnHandleCollisionEnter(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void PlayerScript::OnHandleCollisionStay(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void PlayerScript::OnHandleCollisionExit(const std::string& groupTag, const std::shared_ptr<GameObject>& opponent) { }

void PlayerScript::DispatchGameEvent(GameEvent* event) { }
