#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameObject.h
// 
// 2025 - 02 - 02 김성준 : GameObject
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Transform.h"

inline constexpr BYTE MAX_INPUT_STORED = 100;
inline constexpr float TEMP_SPEED = 20.0f;

class GameObject {
public:
    GameObject();
    ~GameObject();
    
public:
    void SetInput(Key key);
    void Update(const float deltaTime);

    void InitId(SessionIdType id);
    SessionIdType GetId() const;
    SimpleMath::Matrix GetWorld() const;
    Transform& GetTransform();

    SimpleMath::Vector3 GetPosition() const;
    SimpleMath::Quaternion GetRotation() const;
    SimpleMath::Vector3 GetScale() const;

private:
    SessionIdType mId{ INVALID_SESSION_ID };

    std::array<bool, MAX_KEY_SIZE> mKeyState{ };

    Transform mTransform{ };
};