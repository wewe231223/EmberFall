#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameRandom.h
//  2024 - 03 - 03 : Random 함수들 
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Random {
public:
    template <typename IntType>
    using UniformInt = std::uniform_int_distribution<IntType>;

    template <typename RealType>
    using UniformReal = std::uniform_real_distribution<RealType>;

    using NormalDist = std::normal_distribution<double>;

public:
    template <typename T> requires std::is_arithmetic_v<T>
    static T GetRandom(T min=std::numeric_limits<T>::min(), T max=std::numeric_limits<T>::max()) {
        if (min > max) {
            std::swap(min, max);
        }

        if constexpr (std::is_integral_v<T>) {
            UniformInt<T> uid{ min, max };
            return uid(dre);
        }
        else {
            UniformReal<T> urd{ min, max };
            return urd(dre);
        }
    }
    
    static SimpleMath::Vector3 GetRandomColor();
    static SimpleMath::Vector3 GetRandomDirVec3();
    static SimpleMath::Vector2 GetRandomDirVec2();

    static SimpleMath::Vector3 GetRandomVec3(float min, float max);
    static SimpleMath::Vector3 GetRandomVec3(const SimpleMath::Vector3& min, const SimpleMath::Vector3& max);
    static SimpleMath::Vector2 GetRandomVec2(const SimpleMath::Vector2& min, const SimpleMath::Vector2& max);

private:
    inline static std::random_device rd{ };
    inline static std::default_random_engine dre{ rd() };
};

