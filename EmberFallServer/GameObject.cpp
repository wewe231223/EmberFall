#include "pch.h"
#include "GameObject.h"
#include "Physics.h"
#include "Input.h"

#include "CollisionManager.h"
#include "ObjectManager.h"
#include "Sector.h"
#include "ServerFrame.h"

GameObject::GameObject()
    : mTransform{ std::make_shared<Transform>() }, mPhysics{ std::make_shared<Physics>() }, mTimer{ std::make_unique<SimpleTimer>() } {
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
    mSpec.entity = Packets::EntityType_ENV;
    mSpec.interactable = false;
    mSpec.hp = 0.0f;

    gSectorSystem->RemoveInSector(GetId(), GetPosition());
    gObjectManager->ReleaseObject(GetId());

    mTag = ObjectTag::ENV;

    // Reset Base Components
    mTransform->Reset();
    mPhysics->Reset();

    mEntityScript.reset();
}

void GameObject::Init() {
    decltype(auto) sharedThis = std::static_pointer_cast<GameObject>(shared_from_this());
    mWeaponSystem.SetOwnerId(GetId());
    mAnimationStateMachine.SetOwner(sharedThis);

    if (nullptr != mEntityScript) {
        mEntityScript->Init();
    }
}

void GameObject::RegisterUpdate() {
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
    
    gServerFrame->AddTimerEvent(GetId(), SysClock::now() + 150ms, TimerEventType::UPDATE_NPC);
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

    mPhysics->Update(mDeltaTime);
    mTransform->Update();
    mTransform->SetY(0.0f); // test

    auto movePacket = FbsPacketFactory::ObjectMoveSC(
        GetId(),
        GetTransform()->GetEulerRotation().y,
        GetPosition(), mTransform->Forward(),
        mPhysics->GetSpeed()
    );
    StorePacket(movePacket);

    if (nullptr == mBoundingObject) {
        return;
    }
    mBoundingObject->Update(mTransform->GetWorld());

    decltype(auto) sharedThis = std::static_pointer_cast<GameObject>(shared_from_this());
    gCollisionManager->UpdateCollision(sharedThis);
}

void GameObject::LateUpdate() {
    if (not mSpec.active) {
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

    decltype(auto) sharedThis = std::static_pointer_cast<GameObject>(shared_from_this());
    gSectorSystem->UpdateEntityMove(sharedThis);
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