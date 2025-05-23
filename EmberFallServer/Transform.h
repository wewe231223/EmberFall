#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Transform.h
// 2025 - 02 - 01 김성준 : 게임 오브젝트의 변환 행렬 표현을 위한 클래스 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Transform {
public:
    Transform();
    ~Transform();

    Transform(const Transform& other);
    Transform(Transform&& other) noexcept;
    Transform& operator=(const Transform& other);
    Transform& operator=(Transform&& other) noexcept;

public:
    SimpleMath::Vector3 Forward() const;
    SimpleMath::Vector3 Right() const;
    SimpleMath::Vector3 Up() const;

    SimpleMath::Vector3 GetPrevPosition() const;
    SimpleMath::Vector3 GetPosition() const;
    SimpleMath::Quaternion GetRotation() const;
    SimpleMath::Vector3 GetEulerRotation() const;
    SimpleMath::Vector3 GetScale() const;
    SimpleMath::Matrix GetWorld() const;

    void Reset();

    void SetPosition(const SimpleMath::Vector3& pos);
    void SetY(const float y);
    void Translate(const SimpleMath::Vector3& v);
    void CheckAndMove(const SimpleMath::Vector3& moveVec);
    //void Move(const SimpleMath::Vector3& dir, float force);

    void SetLook(const SimpleMath::Vector3& lookVec);

    void LookAt(const SimpleMath::Vector3& target);
    void LookAtSmoothly(const SimpleMath::Vector3& target, float lerpFactor);

    void Rotation(const SimpleMath::Quaternion& quat);

    void Rotate(const float yaw=0.0f, const float pitch=0.0f, const float roll=0.0f);
    void Rotate(const SimpleMath::Vector3& v);
    void Rotate(const SimpleMath::Quaternion& quat);
    void RotateSmoothly(const SimpleMath::Quaternion& quat, float lerpfactor);

    void Scale(const SimpleMath::Vector3& v);

    void Update();

private:
    SimpleMath::Vector3 mPosition{ SimpleMath::Vector3::Zero };
    SimpleMath::Quaternion mRotation{ SimpleMath::Quaternion::Identity };
    SimpleMath::Vector3 mScale{ SimpleMath::Vector3::One };

    SimpleMath::Matrix mWorld{ SimpleMath::Matrix::Identity };
    SimpleMath::Vector3 mPrevPosition{ SimpleMath::Vector3::Zero };
};
