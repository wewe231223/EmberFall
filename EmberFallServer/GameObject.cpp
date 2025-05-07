#include "pch.h"
#include "GameObject.h"
#include "Physics.h"
#include "Input.h"

#include "CollisionManager.h"
#include "ObjectManager.h"
#include "Sector.h"
#include "ServerFrame.h"
#include "GameRoom.h"

GameObject::GameObject()
    : mTransform{ std::make_shared<Transform>() }, mPhysics{ std::make_shared<Physics>() }, mTimer{ std::make_unique<SimpleTimer>() } {
    mWeaponSystem.SetWeapon(Packets::Weapon_SWORD);
    mPhysics->SetTransform(mTransform);
    mOverlapped = std::make_unique<OverlappedUpdate>();
}

GameObject::GameObject(uint16_t roomIdx) 
    : INetworkObject{ roomIdx }, mTransform { std::make_shared<Transform>() }, mPhysics{ std::make_shared<Physics>() }, mTimer{ std::make_unique<SimpleTimer>() } {
    mWeaponSystem.SetWeapon(Packets::Weapon_SWORD);
    mPhysics->SetTransform(mTransform);
    mOverlapped = std::make_unique<OverlappedUpdate>();
}

GameObject::~GameObject() { }

SimpleMath::Matrix GameObject::GetWorld() const {
    return mTransform->GetWorld();
}

float GameObject::GetDeltaTime() const {
    return mDeltaTime;
}

float GameObject::GetSpeed() const {
    return mPhysics->GetSpeed();
}

SimpleMath::Vector3 GameObject::GetMoveDir() const {
    return mPhysics->GetMoveDir();
}

bool GameObject::IsDead() const {
    return Packets::AnimationState_DEAD == mAnimationStateMachine.GetCurrState();
}

std::shared_ptr<Transform> GameObject::GetTransform() const {
    return mTransform;
}

std::shared_ptr<Physics> GameObject::GetPhysics() const {
    return mPhysics;
}

std::shared_ptr<BoundingObject> GameObject::GetBoundingObject() const {
    return mBoundingObject;
}

std::shared_ptr<Script> GameObject::GetScript() const {
    return mEntityScript;
}

std::shared_ptr<BuffSystem> GameObject::GetBuffSystem() const {
    return mBuffSystem;
}

SimpleMath::Vector3 GameObject::GetPrevPosition() const {
    return mTransform->GetPrevPosition();
}

SimpleMath::Vector3 GameObject::GetPosition() const {
    return mTransform->GetPosition();
}

SimpleMath::Quaternion GameObject::GetRotation() const {
    return mTransform->GetRotation();
}

SimpleMath::Vector3 GameObject::GetEulerRotation() const {
    return mTransform->GetEulerRotation();
}

SimpleMath::Vector3 GameObject::GetScale() const {
    return mTransform->GetScale();
}

ObjectTag GameObject::GetTag() const {
    return mTag;
}

void GameObject::SetTag(ObjectTag tag) {
    mTag = tag;
}

void GameObject::ChangeWeapon(Packets::Weapon weapon) {
    mWeaponSystem.SetWeapon(weapon);
}

void GameObject::DisablePhysics() {
    mPhysics->Disable();
}

void GameObject::Reset() {
    // Reset My Spec
    mAnimationStateMachine.ChangeState(Packets::AnimationState_IDLE, true);
    mSpec.entity = Packets::EntityType_ENV;
    mSpec.active = false;
    mSpec.interactable = false;
    mSpec.hp = 0.0f;

    decltype(auto) sendBuf = GetSendBuf();
    OverlappedSend* sendPacket{ };
    while (true == sendBuf.try_pop(sendPacket)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Object [{}] - Release Packet", GetId());
        FbsPacketFactory::ReleasePacketBuf(sendPacket);
    }

    std::shared_ptr<GameEvent> event{ };
    while (true == mGameEvents.try_pop(event)) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Object [{}] - Release Event", GetId());
    }

    auto myRoom = GetMyRoomIdx();
    gGameRoomManager->GetRoom(myRoom)->GetStage().GetSectorSystem()->RemoveInSector(GetId(), GetPosition());
    gGameRoomManager->GetRoom(myRoom)->GetStage().GetObjectManager()->ReleaseObject(GetId());
    gGameRoomManager->GetRoom(myRoom)->NotifyDestructedObject(GetTag());

    mTag = ObjectTag::ENV;

    // Reset Base Components
    mTransform->Reset();
    mPhysics->Reset();

    mEntityScript.reset();
}

void GameObject::Init() {
    mTimer->UpdatePoint();

    decltype(auto) sharedThis = std::static_pointer_cast<GameObject>(shared_from_this());
    mWeaponSystem.SetOwnerId(GetMyRoomIdx(), GetId());
    mAnimationStateMachine.SetOwner(sharedThis);

    if (nullptr == mBuffSystem) {
        mBuffSystem = std::make_shared<BuffSystem>(sharedThis);
    }

    if (nullptr != mEntityScript) {
        mEntityScript->Init();
    }
}

void GameObject::RegisterUpdate() {
    auto myRoom = GetMyRoomIdx();
    auto roomStageState = gGameRoomManager->GetRoom(myRoom)->GetStage().GetActiveState();
    if (false == roomStageState) {
        return;
    }

    mOverlapped->owner = shared_from_this();
    gServerCore->PQCS(0, GetId(), mOverlapped.get());
}

void GameObject::ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) {
    if (IOType::UPDATE != overlapped->type) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "GameObject ProcessOverlapped - Is not Overlapped Update");
        return;
    }

    if (not mSpec.active) {
        return;
    }

    Update();
    LateUpdate();
    
    gServerFrame->AddTimerEvent(GetMyRoomIdx(), GetId(), SysClock::now() + 100ms, TimerEventType::UPDATE_NPC);
}

void GameObject::Update() {
    if (not mSpec.active) {
        return;
    }

    mTimer->UpdatePoint();
    mDeltaTime = mTimer->GetDeltaTime();

    mAnimationStateMachine.Update(mDeltaTime);

    if (nullptr != mEntityScript) {
        mEntityScript->Update(mDeltaTime);
    }

    if (nullptr != mBuffSystem) {
        mBuffSystem->Update(mDeltaTime);
    }

    mPhysics->Update(mDeltaTime);
    mTransform->SetY(0.0f);
    mTransform->Update();

    auto currPos = mTransform->GetPosition();
    auto movePacket = FbsPacketFactory::ObjectMoveSC(
        GetId(),
        GetTransform()->GetEulerRotation().y,
        currPos,
        mTransform->Forward(),
        mPhysics->GetSpeed()
    );
    StorePacket(movePacket);

    if (nullptr == mBoundingObject) {
        return;
    }
    mBoundingObject->Update(mTransform->GetWorld());

    auto myRoom = GetMyRoomIdx();
    decltype(auto) sharedThis = std::static_pointer_cast<GameObject>(shared_from_this());
    gGameRoomManager->GetRoom(myRoom)->GetStage().UpdateCollision(sharedThis);
}

void GameObject::LateUpdate() {
    if (not mSpec.active) {
        gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Entity: {}, Update Failure", Packets::EnumNameEntityType(mSpec.entity));
        return;
    }

    // Game Event 처리
    if (nullptr != mEntityScript) {
        std::shared_ptr<GameEvent> event;
        while (true) {
            if (false == mGameEvents.try_pop(event)) {
                break;
            }

            mEntityScript->DispatchGameEvent(event.get());
        }

        mEntityScript->LateUpdate(mDeltaTime);
    }

    if (nullptr != mBuffSystem) {
        mBuffSystem->LateUpdate(mDeltaTime);
    }

    auto myRoom = GetMyRoomIdx();
    decltype(auto) sharedThis = std::static_pointer_cast<GameObject>(shared_from_this());
    gGameRoomManager->GetRoom(myRoom)->GetStage().GetSectorSystem()->UpdateEntityMove(sharedThis);
}

void GameObject::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    if (nullptr == mEntityScript) {
        return;
    }

    mEntityScript->OnCollision(opponent, impulse);
}

void GameObject::OnCollisionTerrain(const float height) {
    mTransform->SetY(height);
    if (nullptr != mEntityScript) {
        mEntityScript->OnCollisionTerrain(height);
    }
}

void GameObject::DoInteraction(std::shared_ptr<GameObject>& obj) {
    if (nullptr != mEntityScript) {
        mEntityScript->DoInteraction(obj);
    }
}

void GameObject::DispatchGameEvent(std::shared_ptr<GameEvent> event) {
    mGameEvents.push(event);
}

void GameObject::Attack() {
    auto changable = mAnimationStateMachine.IsChangable();
    if (not changable or nullptr == mBoundingObject) {
        return;
    }

    mAnimationStateMachine.ChangeState(Packets::AnimationState_ATTACK);
    auto extentsZ = SimpleMath::Vector3::Forward * mBoundingObject->GetForwardExtents();
    mWeaponSystem.Attack(mTransform->GetPosition() + extentsZ, mTransform->Forward());
}

void GameObject::Attack(const SimpleMath::Vector3& dir) {
    mTransform->SetLook(dir);
    Attack();
}