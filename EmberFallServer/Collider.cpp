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

ColliderType Collider::GetType() const {
    return mType;
}

bool Collider::IsEnable() const {
    return mEnable;
}

bool Collider::IsColliding() const {
    return not mStates.empty();
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

        decltype(auto) state = mStates[objId];
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

        decltype(auto) state = mStates[objId];
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

BoxCollider::BoxCollider() 
    : Collider{ ColliderType::BOX } { }

BoxCollider::BoxCollider(const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents)
    : Collider{ ColliderType::BOX }, mLocalBox{ center, extents }, mBoundingBox{ } { }

BoxCollider::~BoxCollider() { }

DirectX::BoundingBox& BoxCollider::GetBoundingBox() {
    return mBoundingBox;
}

void BoxCollider::Update() {
    if (mTransform.expired()) {
        return;
    }

    mLocalBox.Transform(mBoundingBox, mTransform.lock()->GetWorld());
}

void BoxCollider::LateUpdate() { }

bool BoxCollider::CheckCollision(const std::shared_ptr<Collider>& other) {
    if (false == mEnable or false == other->IsEnable()) {
        return false;
    }
    
    switch (other->GetType()) {
    case ColliderType::BOX:
        {
            auto collider = std::static_pointer_cast<BoxCollider>(other);
            return mBoundingBox.Intersects(collider->GetBoundingBox());
        }

    case ColliderType::SPHERE:
        {
            auto collider = std::static_pointer_cast<SphereCollider>(other);
            return mBoundingBox.Intersects(collider->GetBoundingSphere());
        }

    case ColliderType::ORIENTED_BOX:
        {
            auto collider = std::static_pointer_cast<OrientedBoxCollider>(other);
            return mBoundingBox.Intersects(collider->GetBoundingBox());
        }

    default:
        return false;
    }
}

SphereCollider::SphereCollider()
    : Collider{ ColliderType::SPHERE } { }

SphereCollider::SphereCollider(const SimpleMath::Vector3& center, const float radius)
    : Collider{ ColliderType::SPHERE }, mLocalSphere{ center, radius }, mBoundingSphere{ } {}

SphereCollider::~SphereCollider() { }

DirectX::BoundingSphere& SphereCollider::GetBoundingSphere() {
    return mBoundingSphere;
}

void SphereCollider::Update() { 
    if (mTransform.expired()) {
        return;
    }

    mLocalSphere.Transform(mBoundingSphere, mTransform.lock()->GetWorld());
}

void SphereCollider::LateUpdate() { }

bool SphereCollider::CheckCollision(const std::shared_ptr<Collider>& other) {
    if (false == mEnable or false == other->IsEnable()) {
        return false;
    }

    switch (other->GetType()) {
    case ColliderType::BOX:
        {
            auto collider = std::static_pointer_cast<BoxCollider>(other);
            return mBoundingSphere.Intersects(collider->GetBoundingBox());
        }

    case ColliderType::SPHERE:
        {
            auto collider = std::static_pointer_cast<SphereCollider>(other);
            return mBoundingSphere.Intersects(collider->GetBoundingSphere());
        }

    case ColliderType::ORIENTED_BOX:
        {
            auto collider = std::static_pointer_cast<OrientedBoxCollider>(other);
            return mBoundingSphere.Intersects(collider->GetBoundingBox());
        }

    default:
        return false;
    }
}

OrientedBoxCollider::OrientedBoxCollider()
    : Collider{ ColliderType::ORIENTED_BOX } { }

OrientedBoxCollider::OrientedBoxCollider(const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents)
    : Collider{ ColliderType::ORIENTED_BOX }, mLocalBox{ center, extents, SimpleMath::Quaternion::Identity }, mBoundingBox{ } { }

OrientedBoxCollider::~OrientedBoxCollider() { }

DirectX::BoundingOrientedBox& OrientedBoxCollider::GetBoundingBox() {
    return mBoundingBox;
}

void OrientedBoxCollider::Update() {
    if (mTransform.expired()) {
        return;
    }

    mLocalBox.Transform(mBoundingBox, mTransform.lock()->GetWorld());
}

void OrientedBoxCollider::LateUpdate() { }

bool OrientedBoxCollider::CheckCollision(const std::shared_ptr<Collider>& other) {
    if (false == mEnable or false == other->IsEnable()) {
        return false;
    }

    switch (other->GetType()) {
    case ColliderType::BOX:
        {
            auto collider = std::static_pointer_cast<BoxCollider>(other);
            return mBoundingBox.Intersects(collider->GetBoundingBox());
        }

    case ColliderType::SPHERE:
        {
            auto collider = std::static_pointer_cast<SphereCollider>(other);
            return mBoundingBox.Intersects(collider->GetBoundingSphere());
        }

    case ColliderType::ORIENTED_BOX:
        {
            auto collider = std::static_pointer_cast<OrientedBoxCollider>(other);
            return mBoundingBox.Intersects(collider->GetBoundingBox());
        }

    default:
        return false;
    }
}
