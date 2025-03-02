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

void PlayerScript::Init() { }

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

    if (mInput->IsActiveKey(VK_SPACE)) {
        physics->Jump(deltaTime);
    }

    if (mInput->IsDown(VK_F1)) {
        physics->mFactor.mass += 10.0kg;
    }

    if (mInput->IsDown(VK_F2)) {
        physics->mFactor.acceleration += 1.0mps2;
    }

    if (mInput->IsDown(VK_SHIFT)) {
        auto maxSpeed = GameUnits::UnitCast<GameUnits::KilloMeterPerHour>(50.0mps);
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player [{}] Change Max Speed to {} km/h",
            GetOwner()->GetId(),
            maxSpeed.Count());
        physics->mFactor.maxMoveSpeed = maxSpeed;
    }
    else if (mInput->IsUp(VK_SHIFT)) {
        auto maxSpeed = GameUnits::UnitCast<GameUnits::KilloMeterPerHour>(5.0mps);
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player [{}] Change Max Speed to {} km/h", 
            GetOwner()->GetId(),
            maxSpeed.Count());
        physics->mFactor.maxMoveSpeed = maxSpeed;
    }

    if (mInput->IsUp(VK_F1)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Push GameEvent: Attack All Monster");

        auto event = std::make_shared<AttackEvent>();
        event->type = GameEventType::ATTACK_EVENT;
        event->sender = GetOwner()->GetId();
        event->damage = 10.0f;
        gEventManager->PushEvent(event);
    }

    if (mInput->IsActiveKey('F')) {
        DestroyGem(deltaTime);
    }
    else if (mInput->IsInactiveKey('F')) {
        mInterationObj = NULL;
    }

    moveDir.Normalize();
    physics->Acceleration(moveDir, deltaTime);
}

void PlayerScript::LateUpdate(const float deltaTime) {
    mInput->Update();
}

void PlayerScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent) { }

void PlayerScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent) { }

void PlayerScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent) { }

void PlayerScript::DispatchGameEvent(GameEvent* event) { }

std::shared_ptr<GameObject> PlayerScript::GetNearestObject() {
    decltype(auto) inRangeObjects = mViewList.GetInRangeObjects();
    if (inRangeObjects.empty()) {
        return nullptr;
    }

    auto owner = GetOwner();
    auto ownerPos = owner->GetPosition();
    auto nearestObj = *std::min_element(inRangeObjects.begin(), inRangeObjects.end(), // 가장 가까운 오브젝트 구하기 O(N)
        [=](const std::shared_ptr<GameObject>& obj1, const std::shared_ptr<GameObject>& obj2) {
            if (owner == obj1 or owner == obj2) {
                return false;
            }

            return SimpleMath::Vector3::DistanceSquared(ownerPos, obj1->GetPosition()) < SimpleMath::Vector3::DistanceSquared(ownerPos, obj2->GetPosition());
        }
    );

    auto distance = SimpleMath::Vector3::DistanceSquared(nearestObj->GetPosition(), ownerPos);
    if (100.0f > distance) {
        return nearestObj;
    }

    return nullptr;
}

void PlayerScript::DestroyGem(const float deltaTime) {
    static float holdStart = 0.0f; // test

    auto nearestObj = GetNearestObject();
    if (nullptr == nearestObj) {
        holdStart = 0.0f;
        return;
    }
    
    auto nearestObjId = nearestObj->GetId();
    if (mInterationObj != nearestObjId) {
        mInterationObj = nearestObjId;
        holdStart = 0.0f;
    }

    std::shared_ptr<GemDestroyEvent> event = std::make_shared<GemDestroyEvent>();
    event->type = GameEventType::DESTROY_GEM_EVENT;
    event->receiver = nearestObj->GetId();
    event->holdTime = holdStart += deltaTime;
    gEventManager->PushEvent(event);
}
