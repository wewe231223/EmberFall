#include "pch.h"
#include "GameObject.h"
#include "Physics.h"
#include "Input.h"

#include "ServerGameScene.h"

GameObject::GameObject(std::shared_ptr<IServerGameScene> gameScene)
    : mTransform{ std::make_shared<Transform>() }, mPhysics{ std::make_shared<Physics>() }, mGameScene{ gameScene } {
    mPhysics->SetTransform(mTransform);
}

GameObject::~GameObject() { }

bool GameObject::IsActive() const {
    return true == mActive;
}

NetworkObjectIdType GameObject::GetId() const {
    return mId;
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

SimpleMath::Vector3 GameObject::GetScale() const {
    return mTransform->GetScale();
}

SimpleMath::Vector3 GameObject::GetColor() const {
    if (mCollider->IsColliding()) { // 디버깅을 위한 색상 변경
        return SimpleMath::Vector3{ 1.0f, 0.0f, 0.0f }; // RED
    }
    return mColor;
}

ObjectTag GameObject::GetTag() const {
    return mTag;
}

void GameObject::InitId(NetworkObjectIdType id) {
    mId = id;
}

void GameObject::SetActive(bool active) {
    mActive = active;
}

void GameObject::SetColor(const SimpleMath::Vector3& color) {
    mColor = color;
}

void GameObject::SetTag(ObjectTag tag) {
    mTag = tag;
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
