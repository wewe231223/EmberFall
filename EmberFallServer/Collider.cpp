#include "pch.h"
#include "Collider.h"

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

BoxCollider::BoxCollider(const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents) 
    : Collider{ ColliderType::BOX }, mBoundingBox{ center, extents } { }

BoxCollider::~BoxCollider() { }

void BoxCollider::UpdatePosition(const SimpleMath::Vector3& position) {
    mBoundingBox.Center = position;
}

bool BoxCollider::CheckCollision(const std::shared_ptr<Collider>& other) const {
    if (false == mEnable or false == other->IsEnable()) {
        return false;
    }

    auto collider = std::static_pointer_cast<BoxCollider>(other);
    if (true == mBoundingBox.Intersects(collider->mBoundingBox)) {
        return true;
    }
}
