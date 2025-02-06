#include "pch.h"
#include "GameObject.h"
#include "Physics.h"

GameObject::GameObject() 
    : mTransform{ std::make_shared<Transform>() } { }

GameObject::~GameObject() { }

void GameObject::SetInput(Key key) {
    mKeyState[key.key] = key.state;
}

void GameObject::UpdateInput(const float deltaTime) {
    SimpleMath::Vector3 moveVec{ SimpleMath::Vector3::Zero };
    if (Key::DOWN == mKeyState['A']) {
        moveVec.x -= TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState['D']) {
        moveVec.x += TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState['W']) {
        moveVec.z -= TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState['S']) {
        moveVec.z += TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState['Q']) {
        moveVec.y -= TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState['E']) {
        moveVec.y += TEMP_SPEED * deltaTime;
    }

    if (Key::DOWN == mKeyState[VK_SPACE]) {
        mTransform->Translate(SimpleMath::Vector3{ 0.0f, 30.0f, 0.0f });
    }

    mTransform->Move(moveVec);
}

void GameObject::Update(const float deltaTime) {
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

void GameObject::MakePhysics() {
    mPhysics = std::make_shared<Physics>();
    mPhysics->SetTransform(mTransform);
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

void GameObject::OnCollisionStay(const std::string& groupTag, std::shared_ptr<GameObject>& opponent) { }

void GameObject::OnCollisionExit(const std::string& groupTag, std::shared_ptr<GameObject>& opponent) { 
    std::cout << std::format("Collision End!!! Group: {}, opponent ID: {}\n", groupTag, opponent->GetId());
}
