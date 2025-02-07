#include "pch.h"
#include "GameObject.h"
#include "Physics.h"
#include "Input.h"

GameObject::GameObject() 
    : mTransform{ std::make_shared<Transform>() }, mPhysics{ std::make_shared<Physics>() } {
    mPhysics->SetTransform(mTransform);
    mInput = std::make_shared<Input>(mPhysics, mTransform);
}

GameObject::~GameObject() { }

void GameObject::SetInput(Key key) {
    mInput->UpdateInput(key);
}

void GameObject::Update(const float deltaTime) {
    mInput->Update(deltaTime);
    mPhysics->Update(deltaTime);
    mTransform->Update();

    if (nullptr != mCollider) {
        mCollider->Update();
    }
}

void GameObject::InitId(NetworkObjectIdType id) {
    mId = id;
}

NetworkObjectIdType GameObject::GetId() const {
    return mId;
}

SimpleMath::Matrix GameObject::GetWorld() const {
    return mTransform->GetWorld();
}

std::shared_ptr<Transform> GameObject::GetTransform() {
    return mTransform;
}

std::shared_ptr<Collider> GameObject::GetCollider() const {
    return mCollider;
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

void GameObject::SetColor(const SimpleMath::Vector3& color) {
    mColor = color;
}

SimpleMath::Vector3 GameObject::GetColor() const {
    if (mCollider->IsColliding()) {
        return SimpleMath::Vector3{ 1.0f, 0.0f, 0.0f }; // RED
    }
    return mColor;
}

void GameObject::OnCollision(const std::string& groupTag, std::shared_ptr<GameObject>& opponent) {
    auto state = mCollider->GetState(opponent->GetId());
    switch (state) {
    case CollisionState::ENTER:
        OnCollisionEnter(groupTag, opponent);
        break;

    case CollisionState::STAY:
        OnCollisionStay(groupTag, opponent);
        break;

    case CollisionState::EXIT:
        OnCollisionExit(groupTag, opponent);
        break;

    default:
        break;
    }
}

void GameObject::OnCollisionTerrain(const float height) {
    mTransform->SetY(height);
}

void GameObject::OnCollisionEnter(const std::string& groupTag, std::shared_ptr<GameObject>& opponent) {
    std::cout << std::format("Collision!!! Group: {}, opponent ID: {}\n", groupTag, opponent->GetId());
}

void GameObject::OnCollisionStay(const std::string& groupTag, std::shared_ptr<GameObject>& opponent) { 
    static constexpr float repulsiveForce = 2.0f;
    SimpleMath::Vector3 repulsiveDir = GetPosition() - opponent->GetPosition();
    repulsiveDir.Normalize();

    mTransform->Translate(repulsiveDir * repulsiveForce);
}

void GameObject::OnCollisionExit(const std::string& groupTag, std::shared_ptr<GameObject>& opponent) { 
    std::cout << std::format("Collision End!!! Group: {}, opponent ID: {}\n", groupTag, opponent->GetId());
}
