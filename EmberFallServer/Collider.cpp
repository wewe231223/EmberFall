#include "pch.h"
#include "Collider.h"
#include "Transform.h"

Collider::Collider(ColliderType type) 
    : mType{ type } { }

Collider::~Collider() { }

Collider::Collider(const Collider& other) 
    : mState{ other.mState } { }

Collider::Collider(Collider&& other) noexcept 
    : mState{ other.mState } { }

Collider& Collider::operator=(const Collider& other) {
    mState = other.mState;
    return *this;
}

Collider& Collider::operator=(Collider&& other) noexcept {
    mState = other.mState;
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

CollisionState Collider::UpdateState(CollisionState state) {
    mState = state;
    return mState;
}

CollisionState Collider::UpdateState(bool collisionResult) {
    if (true == collisionResult) {
        switch (mState) {
        case CollisionState::NONE:
        case CollisionState::EXIT:
            mState = CollisionState::ENTER;
            break;

        case CollisionState::ENTER:
            mState = CollisionState::STAY;
            break;

        default:        // STAY인 경우 처리 X
            break;
        }
    }
    else {
        switch (mState) {
        case CollisionState::ENTER:
        case CollisionState::STAY:
            mState = CollisionState::EXIT;
            break;

        case CollisionState::EXIT:
            mState = CollisionState::NONE;
            break;

        default:        // NONE인 경우 처리 X
            break;
        }
    }

    return mState;
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

CollisionState BoxCollider::CheckCollision(const std::shared_ptr<Collider>& other) {
    if (false == mEnable or false == other->IsEnable()) {
        return UpdateState(CollisionState::NONE);
    }

    auto collider = std::static_pointer_cast<BoxCollider>(other);
    bool collisionResult = mBoundingBox.Intersects(collider->mBoundingBox);

    return UpdateState(collisionResult);
}
