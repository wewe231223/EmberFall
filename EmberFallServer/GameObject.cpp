#include "pch.h"
#include "GameObject.h"
#include "Physics.h"
#include "Input.h"

GameObject::GameObject() 
    : mTransform{ std::make_shared<Transform>() }, mPhysics{ std::make_shared<Physics>() } {
    mPhysics->SetTransform(mTransform);
}

GameObject::~GameObject() { }

bool GameObject::IsActive() const {
    return true == mActive;
}

void GameObject::SetActive(bool active) {
    mActive = active;
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

std::shared_ptr<Transform> GameObject::GetTransform() const {
    return mTransform;
}

std::shared_ptr<Physics> GameObject::GetPhysics() const {
    return mPhysics;
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
    //if (mCollider->IsColliding()) { // 디버깅을 위한 색상 변경
    //    return SimpleMath::Vector3{ 1.0f, 0.0f, 0.0f }; // RED
    //}
    return mColor;
}

void GameObject::Update(const float deltaTime) {
    for (auto& component : mComponents) {
        component->Update(deltaTime);
    }

    mPhysics->Update(deltaTime);
    mTransform->Update();

    if (nullptr != mCollider) {
        mCollider->Update();
    }
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
    mPhysics->SetOnGround(true);
}

void GameObject::OnCollisionEnter(const std::string& groupTag, std::shared_ptr<GameObject>& opponent) { }

void GameObject::OnCollisionStay(const std::string& groupTag, std::shared_ptr<GameObject>& opponent) {
    auto obb1 = std::static_pointer_cast<OrientedBoxCollider>(mCollider)->GetBoundingBox();
    auto obb2 = std::static_pointer_cast<OrientedBoxCollider>(opponent->mCollider)->GetBoundingBox();

    float myMass = mPhysics->mFactor.mass;
    float opponentMass = opponent->mPhysics->mFactor.mass;
    // 내가 무거울 수록 덜 밀려나는 구조.
    float coefficient = (opponentMass / (myMass + opponentMass)) + MathUtil::EPSILON; // 0.0f ~ 1.0f 사이 값. (EPSILON 추가)
    auto repulsiveVec = Collision::GetMinTransVec(obb1, obb2) * coefficient;

    bool amIOnGround = mPhysics->IsOnGround();
    bool isOpponentOnGround = opponent->mPhysics->IsOnGround();
    if ((amIOnGround and isOpponentOnGround) or (not amIOnGround and not isOpponentOnGround)) {
        mTransform->Translate(repulsiveVec);
    }

    if ((mTransform->GetPrevPosition().y) > (obb2.Center.y + obb2.Extents.y)) {
        mPhysics->SetOnGround(true);
        mPhysics->SetOnOtherObject(true);
        mTransform->SetY(obb1.Extents.y + obb2.Center.y + obb2.Extents.y);
    }
}

void GameObject::OnCollisionExit(const std::string& groupTag, std::shared_ptr<GameObject>& opponent) { }
