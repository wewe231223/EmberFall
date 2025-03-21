#include "pch.h"
#include "GameObject.h"
#include "Physics.h"
#include "Input.h"

#include "ServerGameScene.h"

GameObject::GameObject(std::shared_ptr<IServerGameScene> gameScene)
    : mTransform{ std::make_shared<Transform>() }, mPhysics{ std::make_shared<Physics>() }, mGameScene{ gameScene } {
    mWeaponSystem.SetWeapon(Weapon::NONE);
    mPhysics->SetTransform(mTransform);
}

GameObject::~GameObject() { }

bool GameObject::IsActive() const {
    return true == mActive;
}

NetworkObjectIdType GameObject::GetId() const {
    return mId;
}

float GameObject::HP() const {
    return mHP;
}

SimpleMath::Matrix GameObject::GetWorld() const {
    return mTransform->GetWorld();
}

std::shared_ptr<Transform> GameObject::GetTransform() const {
    return mTransform;
}

std::shared_ptr<Physics> GameObject::GetPhysics() const {
    return mPhysics;
}

std::shared_ptr<Collider> GameObject::GetCollider() const {
    return mCollider;
}

std::shared_ptr<IServerGameScene> GameObject::GetOwnGameScene() const {
    return mGameScene;
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

EntityType GameObject::GetEntityType() const {
    return mEntityType;
}

bool GameObject::IsCollidingObject() const {
    return nullptr != mCollider;
}

void GameObject::InitId(NetworkObjectIdType id) {
    mId = id;
}

void GameObject::SetActive(bool active) {
    mActive = active;
}

void GameObject::SetTag(ObjectTag tag) {
    mTag = tag;
}

void GameObject::SetEntityType(EntityType type) {
    mEntityType = type;
}

void GameObject::SetCollider(std::shared_ptr<Collider> collider) {
    mCollider = collider;
}

void GameObject::ChangeWeapon(Weapon weapon) {
    mWeaponSystem.SetWeapon(weapon);
    gLogConsole->PushLog(DebugLevel::LEVEL_DEBUG, "Player Change Weapon : {}", static_cast<int>(weapon));
}

void GameObject::Reset() {
    ClearComponents();
    mHP = 0.0f;
    mCollider.reset();
    mTransform->Reset();
    mPhysics->Reset();
}

void GameObject::ReduceHealth(float hp) {
    mHP -= hp;
}

void GameObject::RestoreHealth(float hp) {
    mHP += hp;
}

void GameObject::Init() {
    for (auto& component : mComponents) {
        component->Init();
    }
}

void GameObject::Update(const float deltaTime) {
    if (not IsActive()) {
        return;
    }

    for (auto& component : mComponents) {
        component->Update(deltaTime);
    } 

    mPhysics->Update(deltaTime);
    mTransform->Update();

    if (nullptr != mCollider) {
        mCollider->Update();
    }
}

void GameObject::LateUpdate(const float deltaTime) {
    if (not IsActive()) {
        return;
    }

    for (auto& component : mComponents) {
        component->LateUpdate(deltaTime);
    }

    mPhysics->LateUpdate(deltaTime);
    mTransform->LateUpdate(deltaTime);

    if (nullptr != mCollider) {
        mCollider->LateUpdate();
    }
}

void GameObject::OnCollision(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    auto state = mCollider->GetState(opponent->GetId());
    switch (state) {
    case CollisionState::ENTER:
        OnCollisionEnter(opponent, impulse);
        break;

    case CollisionState::STAY:
        OnCollisionStay(opponent, impulse);
        break;

    case CollisionState::EXIT:
        OnCollisionExit(opponent, impulse);
        break;

    default:
        break;
    }
}

void GameObject::OnCollisionTerrain(const float height) {
    mTransform->SetY(height);
}

void GameObject::DispatchGameEvent(GameEvent* event) {
    for (auto& component : mComponents) {
        component->DispatchGameEvent(event);
    }
}

void GameObject::ClearComponents() {
    mComponents.clear();
}

void GameObject::Attack() {
    mWeaponSystem.Attack(mTransform->GetPosition(), mTransform->Forward());
}

void GameObject::OnCollisionEnter(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { 
    for (auto& component : mComponents) {
        component->OnHandleCollisionEnter(opponent, impulse);
    }
}

void GameObject::OnCollisionStay(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) { 
    for (auto& component : mComponents) {
        component->OnHandleCollisionStay(opponent, impulse);
    }
}

void GameObject::OnCollisionExit(std::shared_ptr<GameObject>& opponent, const SimpleMath::Vector3& impulse) {
    for (auto& component : mComponents) {
        component->OnHandleCollisionExit(opponent, impulse);
    }
}
