#pragma once

inline constexpr BYTE MAX_INPUT_STORED = 100;

class GameObject {
public:
    GameObject();
    ~GameObject();

public:
    void SetInput(Key key);
    void Update(const float deltaTime);

    SimpleMath::Matrix GetMatrix() const;

private:
    SimpleMath::Vector3 mPosition{ };
    SimpleMath::Quaternion mRotation{ };

    SimpleMath::Matrix mWorldMat{ };
    std::array<Key, MAX_INPUT_STORED> mKeyInputs{ };
    size_t mKeyInputSize{ };
};