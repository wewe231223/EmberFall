#include "pch.h"
#include "PlayerScript.h"
#include "Input.h"

#include "GameObject.h"
#include "ServerGameScene.h"
#include "GameEventManager.h"
#include "ObjectSpawner.h"

PlayerScript::PlayerScript(std::shared_ptr<GameObject> owner, std::shared_ptr<Input> input)
    : Script{ owner, ObjectTag::PLAYER }, mInput{ input }, mViewList{ static_cast<SessionIdType>(owner->GetId()) } {
    owner->SetEntityType(EntityType::PLAYER);
}

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
    if (mInput->IsUp('P')) {
        GetOwner()->Attack();
    }

    if (mInput->IsUp('1')) {
        GetOwner()->ChangeWeapon(Weapon::NONE);
    }

    if (mInput->IsUp('2')) {
        GetOwner()->ChangeWeapon(Weapon::SWORD);
    }

    if (mInput->IsUp('3')) {
        GetOwner()->ChangeWeapon(Weapon::SPEAR);
    }

    if (mInput->IsUp('4')) {
        GetOwner()->ChangeWeapon(Weapon::BOW);
    }

    if (mInput->IsActiveKey('A')) {
        moveDir.x += 1.0f;
    }

    if (mInput->IsActiveKey('D')) {
        moveDir.x -= 1.0f;
    }

    if (mInput->IsActiveKey('W')) {
        moveDir.z -= 1.0f;
    }

    if (mInput->IsActiveKey('S')) {
        moveDir.z += 1.0f;
    }

    if (mInput->IsDown(VK_SPACE)) {
        physics->Jump(deltaTime);
        auto animPacket = PacketSC::PacketAnimationState{ 
            sizeof(PacketSC::PacketAnimationState),
            PacketType::PACKET_ANIM_STATE,
            static_cast<SessionIdType>(GetOwner()->GetId()) 
        };

        animPacket.objId = GetOwner()->GetId();
        animPacket.animState = AnimationState::JUMP;

        gServerCore->SendAll(&animPacket);
    }

    if (mInput->IsUp(VK_F1)) {
        gObjectSpawner->SpawnProjectile(ObjectTag::ARROW, GetOwner()->GetPosition(),
            GetOwner()->GetTransform()->Forward(), GameProtocol::Unit::DEFAULT_PROJECTILE_SPEED); // temp speed
    }

    if (mInput->IsDown(VK_LSHIFT)) {
        auto maxSpeed = GameUnits::UnitCast<GameUnits::KilloMeterPerHour>(GameProtocol::Unit::PLAYER_RUN_SPEED);
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player [{}] Change Max Speed to {} km/h",
            GetOwner()->GetId(),
            maxSpeed.Count());
        physics->mFactor.maxMoveSpeed = maxSpeed;
    }
    else if (mInput->IsUp(VK_LSHIFT)) {
        auto maxSpeed = GameUnits::UnitCast<GameUnits::KilloMeterPerHour>(GameProtocol::Unit::PLAYER_WALK_SPEED);
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player [{}] Change Max Speed to {} km/h", 
            GetOwner()->GetId(),
            maxSpeed.Count());
        physics->mFactor.maxMoveSpeed = maxSpeed;
    }

    if (mInput->IsActiveKey('F')) {
        Interaction(deltaTime, GetNearestObject());
    }
    else if (mInput->IsUp('F')) {
        if (mInteractionObj != INVALID_OBJ_ID) {
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Interaction Cancel");
            auto interactionPacket = PacketSC::PacketInteractCancel{
                sizeof(PacketSC::PacketInteractCancel),
                PacketType::PACKET_INTERACTION_CANCEL,
                static_cast<uint8_t>(GetOwner()->GetId())
            };

            interactionPacket.playerId = GetOwner()->GetId();
            gServerCore->Send(static_cast<SessionIdType>(GetOwner()->GetId()), &interactionPacket);
            mInteractionObj = INVALID_OBJ_ID;
        }
    }

    if (mInput->IsUp('U')) {
        UseItem();
    }

    moveDir.Normalize();
    moveDir = SimpleMath::Vector3::Transform(moveDir, GetOwner()->GetTransform()->GetRotation());
    physics->Acceleration(moveDir, deltaTime);
}

void PlayerScript::LateUpdate(const float deltaTime) {
    mInput->Update();
}

void PlayerScript::OnHandleCollisionEnter(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void PlayerScript::OnHandleCollisionStay(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void PlayerScript::OnHandleCollisionExit(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { }

void PlayerScript::DispatchGameEvent(GameEvent* event) { 
    switch (event->type) {
    case GameEventType::ATTACK_EVENT:
        if (event->sender != event->receiver) {
            auto attackEvent = reinterpret_cast<AttackEvent*>(event);
            GetOwner()->ReduceHealth(attackEvent->damage);
            gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player[] Attacked!!", GetOwner()->GetId());
        }
        break;

    case GameEventType::DESTROY_GEM_COMPLETE:
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Interaction Finish");
        {
            auto interactionPacket = PacketSC::PacketInteractFinished{
                sizeof(PacketSC::PacketInteractFinished),
                PacketType::PACKET_INTERACTION_FINISH,
                static_cast<uint8_t>(GetOwner()->GetId())
            };

            interactionPacket.interactionEntityType = EntityType::CORRUPTED_GEM;
            interactionPacket.playerId = GetOwner()->GetId();
            interactionPacket.interactObjId = event->sender;
            gServerCore->Send(GetOwner()->GetId(), &interactionPacket);
        }
        mInteractionObj = INVALID_OBJ_ID;
        break;
    }
}

std::shared_ptr<GameObject> PlayerScript::GetNearestObject() {
    decltype(auto) inRangeObjects = mViewList.GetInRangeObjects();
    if (inRangeObjects.empty()) {
        return nullptr;
    }

    auto owner = GetOwner();
    auto ownerPos = owner->GetPosition();
    decltype(auto) nearestObj = *std::min_element(inRangeObjects.begin(), inRangeObjects.end(), // 가장 가까운 오브젝트 구하기 O(N)
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

void PlayerScript::Interaction(const float deltaTime, const std::shared_ptr<GameObject>& target) {
    if (nullptr == target or not target->IsActive()) {
        mInteractionObj = INVALID_OBJ_ID;
        return;
    }

    if (target->GetId() != mInteractionObj and EntityType::CORRUPTED_GEM == target->GetEntityType()) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Interaction Start");
        auto interactionPacket = PacketSC::PacketInteractStart{
            sizeof(PacketSC::PacketInteractStart),
            PacketType::PACKET_INTERACTION_START,
            static_cast<uint8_t>(GetOwner()->GetId())
        };

        interactionPacket.interactionEntityType = target->GetEntityType();
        interactionPacket.playerId = GetOwner()->GetId();
        interactionPacket.interactObjId = target->GetId();
        gServerCore->Send(GetOwner()->GetId(), &interactionPacket);
    }

    mInteractionObj = target->GetId();
    switch (target->GetTag()) {
    case ObjectTag::CORRUPTED_GEM:
        DestroyGem(deltaTime, target);
        break;

    case ObjectTag::ITEM:
        AcquireItem(deltaTime, target);
        break;

    default:
        break;
    }
}

void PlayerScript::DestroyGem(const float deltaTime, const std::shared_ptr<GameObject>& gem) {
    static float holdStart = 0.0f; // test
    holdStart += deltaTime;

    auto gemId = gem->GetId();
    if (mInteractionObj != gemId) {
        mInteractionObj = gemId;
        holdStart = 0.0f;
    }

    std::shared_ptr<GemDestroyEvent> event = std::make_shared<GemDestroyEvent>();
    event->type = GameEventType::DESTROY_GEM_EVENT;
    event->receiver = gem->GetId();
    event->holdTime = holdStart;
    gEventManager->PushEvent(event);
}

void PlayerScript::AcquireItem(const float deltaTime, const std::shared_ptr<GameObject>& item) {
}

void PlayerScript::UseItem() {

}