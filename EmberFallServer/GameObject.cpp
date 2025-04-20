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

std::shared_ptr<Transform> GameObject::GetTransform() const {
    return mTransform;
}

std::shared_ptr<Physics> GameObject::GetPhysics() const {
    return mPhysics;
}

std::shared_ptr<BoundingObject> GameObject::GetBoundingObject() const {
    return mBoundingObject;
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
    // Reset Base Component
    mTransform->Reset();
    mPhysics->Reset();

    // Reset My Spec
    mSpec.entity = Packets::EntityType_ENV;
    mSpec.interactable = false;
    mSpec.hp = 0.0f;

    gObjectManager->ReleaseObject(GetId());
}

void GameObject::Init() {
    decltype(auto) sharedThis = std::static_pointer_cast<GameObject>(shared_from_this());
    mWeaponSystem.SetOwnerId(GetId());
    mAnimationStateMachine.SetOwner(sharedThis);

    for (auto& component : mComponents) {
        component->Init();
    }

    if (nullptr != mScript) {
        mScript->Init();
    }
}

void GameObject::RegisterUpdate() {
    mOverlapped->owner = shared_from_this();
    gServerCore->PQCS(0, GetId(), mOverlapped.get());
}

void GameObject::ProcessOverlapped(OverlappedEx* overlapped, INT32 numOfBytes) {
    if (IOType::UPDATE != overlapped->type) {
        gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "GameObject gotted Update Overlapped");
        return;
    }

    Update();
    LateUpdate();

    gServerFrame->AddTimerEvent(GetId(), SysClock::now() + 500ms, TimerEventType::NPC_UPDATE);
}

void GameObject::Update() {
    if (not mSpec.active) {
        return;
    }

    mTimer->UpdatePoint();
    mDeltaTime = mTimer->GetDeltaTime();

    mAnimationStateMachine.Update(mDeltaTime);

    for (auto& component : mComponents) {
        component->Update(mDeltaTime);
    } 

    if (nullptr != mScript) {
        mScript->Update(mDeltaTime);
    }

    mPhysics->Update(mDeltaTime);
    mTransform->Update();
    mTransform->SetY(0.0f); // test
    if (nullptr == mBoundingObject) {
        return;
    }

    mBoundingObject->Update(mTransform->GetWorld());

    decltype(auto) sharedThis = std::static_pointer_cast<GameObject>(shared_from_this());
    gCollisionManager->UpdateCollision(sharedThis);
    gSectorSystem->UpdateEntityMove(sharedThis);
}

void GameObject::LateUpdate() {
    if (not mSpec.active) {
        return;
    }

    // Game Event 처리
    if (nullptr == mScript) {
        return;
    }

    std::shared_ptr<GameEvent> event;
    while (true) {
        if (false == mGameEvents.try_pop(event)) {
            break;
        }

        mScript->DispatchGameEvent(event.get());
    }

    for (auto& component : mComponents) {
        component->LateUpdate(mDeltaTime);
    }

    mPhysics->LateUpdate(mDeltaTime);
    mTransform->LateUpdate(mDeltaTime);
    mScript->LateUpdate(mDeltaTime);
}

void GameObject::OnCollision(const std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    if (nullptr == mScript) {
        return;
    }

    mScript->OnCollision(opponent, impulse);
    if (ObjectTag::TRIGGER == opponent->GetTag()) {
        opponent->OnCollision(std::static_pointer_cast<GameObject>(shared_from_this()), impulse);
    }
}

void GameObject::OnCollisionTerrain(const float height) {
    mTransform->SetY(height);
    for (auto& component : mComponents) {
        component->OnCollisionTerrain(height);
    }

    mScript->OnCollisionTerrain(height);
}

void GameObject::DispatchGameEvent(std::shared_ptr<GameEvent> event) {
    mGameEvents.push(event);
}

void GameObject::ClearComponents() {
    mComponents.clear();
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