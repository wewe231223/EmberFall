#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameObject.cpp
// 
// 2025 - 02 - 02 : GameObject에 Id를 두는 점이 별로 맘에 안듦
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Transform.h"

inline constexpr BYTE MAX_INPUT_STORED = 100;

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
    size_t mKeyInputSize{ };
    std::array<Key, MAX_INPUT_STORED> mKeyInputs{ };

    Transform mTransform{ };
};