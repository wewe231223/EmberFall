#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Collider.cpp
// 2025 - 02 - 04 김성준 : 충돌 감지를 위한 충돌체를 표현.
//                      Collider 클래스로 대부분을 표현
//                      다른 Collider와의 충돌처리는 어떻게?
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum class CollisionState : BYTE {
    NONE,
    ENTER,
    STAY,
    EXIT
};

enum class ColliderType : BYTE {
    BOX,
    SPHERE,
    ORIENTED_BOX,
    OTHER
};

class Collider : public std::enable_shared_from_this<Collider> {
public:
    Collider(ColliderType type);
    virtual ~Collider();

    Collider(const Collider& other);
    Collider(Collider&& other) noexcept;
    Collider& operator=(const Collider& other);
    Collider& operator=(Collider&& other) noexcept;

public:
    void Disable();
    void Enable();
    bool IsEnable() const;

    virtual void UpdatePosition(const SimpleMath::Vector3& position) abstract;
    virtual bool CheckCollision(const std::shared_ptr<Collider>& other) const abstract;

protected:
    std::string mTag{ };                            // OTHER 타입일 경우 식별할 수 있도록 string으로 구성.
    ColliderType mType{ ColliderType::OTHER };
    CollisionState mState{ CollisionState::NONE };
    bool mEnable{ true };
};

class BoxCollider : public Collider {
public:
    BoxCollider(const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents);
    ~BoxCollider();

public:
    virtual void UpdatePosition(const SimpleMath::Vector3& position) override;

    virtual bool CheckCollision(const std::shared_ptr<Collider>& other) const override;

private:
    DirectX::BoundingBox mBoundingBox{ };
};

// 일단 더 생각해보자
//class SphereCollider : public Collider {
//public:
//    SphereCollider();
//    ~SphereCollider();
//
//public:
//    virtual void CheckCollision(const std::shared_ptr<Collider>& other) const override;
//
//    virtual void OnCollisionEnter() override;
//    virtual void OnCollisionStay() override;
//    virtual void OnCollisionExit() override;
//
//private:
//    DirectX::BoundingSphere mBoundingBox{ };
//};
//
//class OrientedBoxCollider : public Collider {
//public:
//    OrientedBoxCollider();
//    ~OrientedBoxCollider();
//
//public:
//    virtual void CheckCollision(const std::shared_ptr<Collider>& other) const override;
//
//    virtual void OnCollisionEnter() override;
//    virtual void OnCollisionStay() override;
//    virtual void OnCollisionExit() override;
//
//private:
//    DirectX::BoundingOrientedBox mBoundingBox{ };
//};