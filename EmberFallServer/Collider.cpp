#include "pch.h"
#include "Collider.h"
#include "Transform.h"

Collider::Collider(ColliderType type) 
    : mType{ type } { }

Collider::~Collider() { }

Collider::Collider(Collider&& other) noexcept 
    : mStates{ std::move(other.mStates) } { }

Collider& Collider::operator=(Collider&& other) noexcept {
    mStates = std::move(other.mStates);
    return *this;
}

void Collider::Disable() {
    mEnable = false;
}

void Collider::Enable() {
    mEnable = true;
}

bool Collider::IsEnable() const {
    return mEnable;
}

void Collider::SetTransform(const std::shared_ptr<Transform>& transform) {
    mTransform = transform;
}

CollisionState Collider::GetState(NetworkObjectIdType id) const {
    if (mStates.contains(id)) {
        return mStates.at(id);
    }

    return CollisionState::NONE;
}

void Collider::UpdateState(bool collisionResult, NetworkObjectIdType objId) {
    if (true == collisionResult) {
        if (not mStates.contains(objId)) {
            mStates[objId] = CollisionState::NONE;
        }

        auto& state = mStates[objId];
        switch (state) {
        case CollisionState::NONE:
        case CollisionState::EXIT:
            state = CollisionState::ENTER;
            break;

        case CollisionState::ENTER:
            state = CollisionState::STAY;
            break;

        default:        // STAY인 경우 처리 X
            break;
        }
    }
    else {
        if (not mStates.contains(objId)) {
            return;
        }

        auto& state = mStates[objId];
        switch (state) {
        case CollisionState::ENTER:
        case CollisionState::STAY:
            state = CollisionState::EXIT;
            break;

        case CollisionState::EXIT:
            state = CollisionState::NONE;
            break;

        default:        // NONE인 경우 Map에서 삭제
            mStates.erase(objId);
            break;
        }
    }
}

BoxCollider::BoxCollider(const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents) 
    : Collider{ ColliderType::BOX }, mBoundingBox{ center, extents } { }

BoxCollider::~BoxCollider() { }

void BoxCollider::Update() {
    if (mTransform.expired()) {
        return;
    }

    mBoundingBox.Center = mTransform.lock()->GetPosition();
}

bool BoxCollider::CheckCollision(const std::shared_ptr<Collider>& other) {
    if (false == mEnable or false == other->IsEnable()) {
        return false;
    }

    auto collider = std::static_pointer_cast<BoxCollider>(other);
    bool collisionResult = mBoundingBox.Intersects(collider->mBoundingBox);

    return collisionResult;
}
