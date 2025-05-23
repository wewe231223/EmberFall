#include "pch.h"
#include "Collider.h"
#include "Transform.h"

BoundingObject::BoundingObject(ColliderType type)
    : mType{ type } { }

BoundingObject::~BoundingObject() { }

ColliderType BoundingObject::GetType() const {
    return mType;
}

OBBCollider::OBBCollider(const SimpleMath::Vector3& center, const SimpleMath::Vector3& ext)
    : BoundingObject{ ColliderType::ORIENTED_BOX }, mLocalBox{ center, ext, SimpleMath::Quaternion::Identity } { }

OBBCollider::OBBCollider(const DirectX::BoundingOrientedBox& box) 
    : BoundingObject{ ColliderType::ORIENTED_BOX }, mLocalBox{ box } { }

OBBCollider::OBBCollider(const OBBCollider& orientedBox) 
    : BoundingObject{ ColliderType::ORIENTED_BOX }, mLocalBox{ orientedBox.mLocalBox } { }

OBBCollider::~OBBCollider() { }

DirectX::BoundingOrientedBox OBBCollider::GetBoundingBox() const {
    return mBoundingBox;
}

float OBBCollider::GetForwardExtents() const {
    return mLocalBox.Extents.z;
}

float OBBCollider::GetRadiusCircumplex() const {
    return SimpleMath::Vector3{ mLocalBox.Extents }.Length();
}

float OBBCollider::GetRadiusCircumplexSq() const {
    return SimpleMath::Vector3{ mLocalBox.Extents }.LengthSquared();
}

CollisionResult OBBCollider::IsColliding(const std::shared_ptr<BoundingObject>& other) const {
    CollisionResult result{ };

    switch (other->GetType()) {
    case ColliderType::ORIENTED_BOX:
    {
        auto box = std::static_pointer_cast<OBBCollider>(other)->GetBoundingBox();
        auto myBox = mBoundingBox;
        result.intersects = box.Intersects(myBox);
        if (false == result.intersects) {
            return CollisionResult{ false, SimpleMath::Vector3::Zero };
        }

        result.penetration = Collision::GetMinTransVec(myBox, box);
        return result;
    }

    case ColliderType::SPHERE:
    {
        break;
    }

    default:
        break;
    }

    gLogConsole->PushLog(DebugLevel::LEVEL_WARNING, "Invalid Collider Type");
    return { };
}

std::shared_ptr<BoundingObject> OBBCollider::Clone() const {
    return nullptr;
}

void OBBCollider::Update(const SimpleMath::Matrix& mat) {
    mLocalBox.Transform(mBoundingBox, mat);
}

SphereCollider::SphereCollider(float radius)
    : BoundingObject{ ColliderType::SPHERE }, mLocalSphere{ SimpleMath::Vector3::Zero, radius } { }

SphereCollider::SphereCollider(const SimpleMath::Vector3& center, float radius)
    : BoundingObject{ ColliderType::SPHERE }, mLocalSphere{ center, radius } { }

SphereCollider::SphereCollider(const SphereCollider& sphere) 
    : BoundingObject{ ColliderType::SPHERE }, mLocalSphere{ sphere.mLocalSphere } { }

SphereCollider::~SphereCollider() { }

float SphereCollider::GetForwardExtents() const {
    return mLocalSphere.Radius;
}

float SphereCollider::GetRadiusCircumplex() const {
    return mLocalSphere.Radius;
}

float SphereCollider::GetRadiusCircumplexSq() const {
    return MathUtil::Square(mLocalSphere.Radius);
}

CollisionResult SphereCollider::IsColliding(const std::shared_ptr<BoundingObject>& other) const {
    return {};
}

std::shared_ptr<BoundingObject> SphereCollider::Clone() const {
    return nullptr;
}

void SphereCollider::Update(const SimpleMath::Matrix& mat) {
    mLocalSphere.Transform(mBoundingSphere, mat);
}
