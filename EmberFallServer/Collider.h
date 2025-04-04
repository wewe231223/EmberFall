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
    BOX,
    SPHERE,
    ORIENTED_BOX,
    OTHER
};

struct ContactImpulse {
    NetworkObjectIdType opponentId;
    SimpleMath::Vector3 prevImpulse;
};

class Collider : public std::enable_shared_from_this<Collider> {
public:
    Collider(ColliderType type);
    virtual ~Collider();

    Collider(Collider&& other) noexcept;
    Collider& operator=(Collider&& other) noexcept;

    Collider(const Collider& other) = delete;
    Collider& operator=(const Collider& other) = delete;

public:
    void Disable();
    void Enable();
    ColliderType GetType() const;
    bool IsEnable() const;
    bool IsColliding() const;

    void SetTransform(const std::shared_ptr<Transform>& transform);
    CollisionState GetState(NetworkObjectIdType id) const;

    virtual float GetForwardExtents() abstract;
    virtual void Update() abstract;
    virtual void LateUpdate() abstract;
    virtual bool CheckCollision(const std::shared_ptr<Collider>& other) abstract;

    void UpdateState(bool collisionResult, NetworkObjectIdType objId);

protected:
    std::string mTag{ };                            // OTHER 타입일 경우 식별할 수 있도록 string으로 구성.
    std::weak_ptr<Transform> mTransform{ };         // GameObject 에서 가지는 Transform 참조

    ColliderType mType{ ColliderType::OTHER };
    std::unordered_map<NetworkObjectIdType, CollisionState> mStates{ };   // 이전 충돌 결과를 기억하기 위한 map
    bool mEnable{ true };
};

class BoxCollider : public Collider {
public:
    BoxCollider();
    BoxCollider(const DirectX::BoundingBox& box);
    BoxCollider(const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents);
    virtual ~BoxCollider();

public:
    DirectX::BoundingBox& GetBoundingBox();
    virtual float GetForwardExtents() override;

    virtual void Update() override;
    virtual void LateUpdate() override;
    virtual bool CheckCollision(const std::shared_ptr<Collider>& other) override;

private:
    DirectX::BoundingBox mLocalBox{ };
    DirectX::BoundingBox mBoundingBox{ };
};

class SphereCollider : public Collider {
public:
    SphereCollider();
    SphereCollider(const SimpleMath::Vector3& center, const float radius);
    virtual ~SphereCollider();

public:
    DirectX::BoundingSphere& GetBoundingSphere();
    virtual float GetForwardExtents() override;

    virtual void Update() override;
    virtual void LateUpdate() override;
    virtual bool CheckCollision(const std::shared_ptr<Collider>& other) override;

private:
    DirectX::BoundingSphere mLocalSphere{ };
    DirectX::BoundingSphere mBoundingSphere{ };
};

class OrientedBoxCollider : public Collider {
public:
    OrientedBoxCollider();
    OrientedBoxCollider(const DirectX::BoundingBox& box);
    OrientedBoxCollider(const SimpleMath::Vector3& center, const SimpleMath::Vector3& extents,
        const SimpleMath::Quaternion& quat = SimpleMath::Quaternion::Identity);
    virtual ~OrientedBoxCollider();

public:
    DirectX::BoundingOrientedBox& GetBoundingBox();
    virtual float GetForwardExtents() override;

    virtual void Update() override;
    virtual void LateUpdate() override;
    virtual bool CheckCollision(const std::shared_ptr<Collider>& other) override;

private:
    DirectX::BoundingOrientedBox mLocalBox{ };
    DirectX::BoundingOrientedBox mBoundingBox{ };
};