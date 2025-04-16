#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Collider.cpp
// 2025 - 02 - 04 김성준 : 충돌 감지를 위한 충돌체를 표현.
//                      Collider 클래스로 대부분을 표현
//                      다른 Collider와의 충돌처리는 어떻게?
// 
//        02 - 05 : 어짜피 GameObject는 Transform을 무조건 가지도록 할 것임.
//                  Transform을 참조해서 Update를 할 수 있도록 하자.
//                  그러면 Update 함수도 가상함수로 구성할 수 있음.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Transform;

enum class ColliderType : BYTE {
    SPHERE,
    ORIENTED_BOX,
};

struct CollisionResult {
    bool intersects;
    SimpleMath::Vector3 penetration;
};

class BoundingObject : public std::enable_shared_from_this<BoundingObject> {
public:
    BoundingObject(ColliderType type);
    virtual ~BoundingObject();

public:
    ColliderType GetType() const;

    virtual CollisionResult IsColliding(const std::shared_ptr<BoundingObject>& other) const abstract;
    virtual std::shared_ptr<BoundingObject> Clone() const abstract;
    virtual void Update(const SimpleMath::Matrix& mat) abstract;

private:
    ColliderType mType{ };
};

class OBBCollider : public BoundingObject {
public:
    OBBCollider(const SimpleMath::Vector3& center, const SimpleMath::Vector3& ext);
    OBBCollider(const OBBCollider& orientedBox);
    virtual ~OBBCollider();

public:
    DirectX::BoundingOrientedBox GetBoundingBox() const;

    virtual CollisionResult IsColliding(const std::shared_ptr<BoundingObject>& other) const override;
    virtual std::shared_ptr<BoundingObject> Clone() const override;
    virtual void Update(const SimpleMath::Matrix& mat) override;

private:
    DirectX::BoundingOrientedBox mLocalBox{ };
    DirectX::BoundingOrientedBox mBoundingBox{ };
};

class SphereCollider : public BoundingObject {
public:
    SphereCollider(float radius);
    SphereCollider(const SimpleMath::Vector3& center, float radius);
    SphereCollider(const SphereCollider& sphere);
    virtual ~SphereCollider();

public:
    virtual CollisionResult IsColliding(const std::shared_ptr<BoundingObject>& other) const override;
    virtual std::shared_ptr<BoundingObject> Clone() const override;
    virtual void Update(const SimpleMath::Matrix& mat) override;

private:
    DirectX::BoundingSphere mLocalSphere{ };
    DirectX::BoundingSphere mBoundingSphere{ };
};