#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GameUnits.h (header only)
// 
// 2025 - 02 - 12 : 게임에서 사용할 단위들을 정의.
//                  단위가 붙어야하는 정수들에는 무조건 literal을 붙이도록 할 예정
// 
//                 : 단위간 변환은 조금 더 생각해 봐야한다.
//                   지금 UnitCast 함수는 단위간 변환을 고려하지 않는다.
//                   단위간 변환을 제약할 수 있도록 하는 시스템을 만들어야할 필요성이 있음.
// 
//        02 - 13 : 게임 단위를 chrono 처럼 ratio로만 표현하니 세부적인 표현이 안된다.
//                  예를 들어 meter = std::ratio<1, 1> 이고 second = std::ratio<1, 1>이면
//                  결국 둘이 같은 타입이기 때문에 템플릿에서 타입 검사로 걸러내지를 못한다.
// 
//                  그냥 여러 struct들로 만들고 UnitTag를 달아서 검사하기로 하자.
// 
//                  operator정의
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <ratio>
#include <iostream>
#include <format>

// 단위들의 정의 부분
namespace GameUnits {
    struct UnitTagLength { };
    struct UnitTagMass { };
    struct UnitTagTime { };
    struct UnitTagSpeed { };
    struct UnitTagAccel { };
    struct UnitTagForce { };

    struct CentiMeter {
        using UnitTag = UnitTagLength;
        using UnitRatio = std::ratio<1, 100>;
    };

    struct Meter {
        using UnitTag = UnitTagLength;
        using UnitRatio = std::ratio<1, 1>;
    };

    struct KilloMeter {
        using UnitTag = UnitTagLength;
        using UnitRatio = std::ratio<1000, 1>;
    };

    struct Second {
        using UnitTag = UnitTagTime;
        using UnitRatio = std::ratio<1, 1>;
    };

    struct Hour {
        using UnitTag = UnitTagTime;
        using UnitRatio = std::ratio<3600, 1>;
    };

    struct MeterPerSec {
        using UnitTag = UnitTagSpeed;
        using UnitRatio = std::ratio_divide<Meter::UnitRatio, Second::UnitRatio>;
    };

    struct MeterPerHour {
        using UnitTag = UnitTagSpeed;
        using UnitRatio = std::ratio_divide<Meter::UnitRatio, Hour::UnitRatio>;
    };

    struct KilloMeterPerSec {
        using UnitTag = UnitTagSpeed;
        using UnitRatio = std::ratio_divide<KilloMeter::UnitRatio, Second::UnitRatio>;
    };

    struct KilloMeterPerHour {
        using UnitTag = UnitTagSpeed;
        using UnitRatio = std::ratio_divide<KilloMeter::UnitRatio, Hour::UnitRatio>;
    };

    struct Gram {
        using UnitTag = UnitTagMass;
        using UnitRatio = std::ratio<1, 1000>;
    };

    struct KilloGram {
        using UnitTag = UnitTagMass;
        using UnitRatio = std::ratio<1, 1>;
    };

    struct MeterPerSec2 {
        using UnitTag = UnitTagAccel;
        using UnitRatio = std::ratio_divide<Meter::UnitRatio, std::ratio_multiply<Second::UnitRatio, Second::UnitRatio>>;
    };

    struct Newton {
        using UnitTag = UnitTagForce;
        using UnitRatio = std::ratio_multiply<KilloGram::UnitRatio, MeterPerSec2::UnitRatio>;
    };

    using StandardLength = GameUnits::Meter;
    using StandardTime = GameUnits::Second;
    using StandardMass = GameUnits::KilloGram;
    using StandardSpeed = GameUnits::MeterPerSec;
    using StandardAccel = GameUnits::MeterPerSec2;
    using StandardForce = GameUnits::Newton;
}

namespace GameUnits { // concept 들 정의
    template <typename T, typename... Types>
    inline constexpr bool IsAnyOf = (std::is_same_v<T, Types> or ...);

    template <typename T, typename... Types>
    inline constexpr bool IsAllOf = (std::is_same_v<T, Types> and ...);

    template <typename UnitType>
    concept IsLengthUnit = std::is_same_v<typename UnitType::UnitTag, UnitTagLength>;

    template <typename UnitType>
    concept IsSpeedUnit = std::is_same_v<typename UnitType::UnitTag, UnitTagSpeed>;

    template <typename UnitType>
    concept IsMassUnit = std::is_same_v<typename UnitType::UnitTag, UnitTagMass>;

    template <typename UnitType>
    concept IsTimeUnit = std::is_same_v<typename UnitType::UnitTag, UnitTagTime>;

    template <typename UnitType>
    concept IsAccelUnit = std::is_same_v<typename UnitType::UnitTag, UnitTagAccel>;

    template <typename UnitType>
    concept IsForceUnit = std::is_same_v<typename UnitType::UnitTag, UnitTagForce>;

    template <typename UnitType>
    concept IsUnitType = IsAnyOf<typename UnitType::UnitTag,
        UnitTagForce, UnitTagAccel, UnitTagTime, UnitTagMass, UnitTagSpeed, UnitTagLength>;

    template <typename Unit1, typename Unit2>
    concept IsConvertibleUnit =
        (IsMassUnit<typename Unit1::Unit> and IsMassUnit<typename Unit2::Unit>)
        or (IsLengthUnit<typename Unit1::Unit> and IsLengthUnit<typename Unit2::Unit>)
        or (IsTimeUnit<typename Unit1::Unit> and IsTimeUnit<typename Unit2::Unit>)
        or (IsSpeedUnit<typename Unit1::Unit> and IsSpeedUnit<typename Unit2::Unit>)
        or (IsAccelUnit<typename Unit1::Unit> and IsAccelUnit<typename Unit2::Unit>);
}

// GameUnit 클래스와 함수들 정의
namespace GameUnits {
    template <typename UnitType> requires IsUnitType<UnitType>
    class GameUnit {
    public:
        using Rep = float;
        using Unit = UnitType;
        using Ratio = typename Unit::UnitRatio;

        // Default Constructor
        constexpr GameUnit() = default;

        template <typename Rep2> requires std::is_arithmetic_v<Rep2>
        constexpr GameUnit(Rep2 val) : mRep{ static_cast<Rep>(val) } { }

        // copy Constructor
        template <typename UnitType2> requires IsConvertibleUnit<GameUnit<UnitType2>, GameUnit>
        constexpr GameUnit(const GameUnit<UnitType2>& unit) : mRep{ UnitCast<GameUnit::Unit>(unit).Count() } { }

        template <typename UnitType2> requires IsConvertibleUnit<GameUnit<UnitType2>, GameUnit>
        constexpr GameUnit& operator=(const GameUnit<UnitType2>& unit) {
            mRep = UnitCast<GameUnit::Unit>(unit).Count();
            return *this;
        }

        // move Constructor

        // return Rep
        constexpr Rep Count() const {
            return mRep;
        }

        template <typename UnitType2> requires IsUnitType<UnitType2>
        constexpr GameUnit operator+(GameUnit<UnitType2> right) {
            return GameUnit(Count() + UnitCast<Unit>(right).Count());
        }

        template <typename UnitType2> requires IsUnitType<UnitType2>
        void operator+=(GameUnit<UnitType2> right) {
            mRep += UnitCast<Unit>(right).Count();
        }

        // operator
        template <typename T> requires std::is_arithmetic_v<T>
        constexpr auto operator*(T scalar) {
            mRep = Count() * scalar;
            return *this;
        }

        template <typename T> requires std::is_arithmetic_v<T>
        constexpr auto operator/(T scalar) {
            mRep = Count() / scalar;
            return *this;
        }

        constexpr auto operator*(GameUnit<Unit> right) {
            mRep = Count() * right.Count();
            return *this;
        }
    
        template <typename RightUnitType> requires IsUnitType<RightUnitType>
        constexpr auto operator*(GameUnit<RightUnitType> right) {
            if constexpr (IsConvertibleUnit<GameUnit, RightUnitType>) {
                // 같은 단위에 대한 곱셈
                mRep = Count() * UnitCast<Unit>(right).Count();
                return *this;
            }

            else if constexpr (IsSpeedUnit<Unit> and IsTimeUnit<RightUnitType>) {
                // 속력 X 시간 인 경우 -> 반환은 거리
                auto speed = UnitCast<StandardSpeed>(*this).Count();
                auto time = UnitCast<StandardTime>(right).Count();
                return GameUnit<StandardLength>(speed * time);
            }
            else if constexpr (IsTimeUnit<Unit> and IsSpeedUnit<RightUnitType>) {
                // 속력 X 시간 인 경우 -> 반환은 거리
                auto time = UnitCast<StandardTime>(*this).Count();
                auto speed = UnitCast<StandardSpeed>(right).Count();
                return GameUnit<StandardLength>(speed * time);
            }
            else if constexpr (IsAccelUnit<Unit> and IsTimeUnit<RightUnitType>) {
                // 가속력 X 시간 인 경우 -> 반환은 속력
                auto accel = UnitCast<StandardAccel>(*this).Count();
                auto time = UnitCast<StandardTime>(right).Count();
                return GameUnit<StandardSpeed>(accel * time);
            }
            else if constexpr (IsTimeUnit<Unit> and IsAccelUnit<RightUnitType>) {
                // 가속력 X 시간 인 경우 -> 반환은 속력
                auto time = UnitCast<StandardTime>(*this).Count();
                auto accel = UnitCast<StandardAccel>(right).Count();
                return GameUnit<StandardSpeed>(accel * time);
            }
            else if constexpr (IsMassUnit<Unit> and IsAccelUnit<RightUnitType>) {
                // 질량 X 가속도인 경우 -> 반환은 힘
                auto mass = UnitCast<StandardMass>(*this).Count();
                auto accel = UnitCast<StandardAccel>(right).Count();
                return GameUnit<StandardForce>(mass * accel);
            }
            else if constexpr (IsAccelUnit<Unit> and IsMassUnit<RightUnitType>) {
                // 질량 X 가속도인 경우 -> 반환은 힘
                auto mass = UnitCast<StandardMass>(right).Count();
                auto accel = UnitCast<StandardAccel>(*this).Count();
                return GameUnit<StandardForce>(mass * accel);
            }
            else {
                static_assert(false);
            }
        }

        constexpr auto operator/(GameUnit<Unit> right) {
            mRep = Count() / right.Count();
            return *this;
        }

        template <typename RightUnitType> requires IsUnitType<RightUnitType>
        constexpr auto operator/(GameUnit<RightUnitType> right) {
            if constexpr (IsConvertibleUnit<Unit, RightUnitType>) {
                // 같은 단위에 대한 나눗셈
                mRep = Count() / UnitCast<GameUnit::Unit>(right).Count();
                return *this;
            }

            else if constexpr (IsSpeedUnit<Unit> and IsTimeUnit<RightUnitType>) {
                // 속력 / 시간 인 경우 -> 반환은 가속력
                auto speed = UnitCast<StandardSpeed>(*this).Count();
                auto time = UnitCast<StandardTime>(right).Count();
                return GameUnit<StandardAccel>(speed / time);
            }
            else if constexpr (IsTimeUnit<Unit> and IsSpeedUnit<RightUnitType>) {
                // 속력 / 시간 인 경우 -> 반환은 가속력
                auto speed = UnitCast<StandardSpeed>(right).Count();
                auto time = UnitCast<StandardTime>(*this).Count();
                return GameUnit<StandardAccel>(speed / time);
            }
            else if constexpr (IsLengthUnit<Unit> and IsTimeUnit<RightUnitType>) {
                // 거리 / 시간 인 경우 -> 반환은 속력
                auto len = UnitCast<StandardLength>(*this).Count();
                auto time = UnitCast<StandardTime>(right).Count();
                return GameUnit<StandardSpeed>(len / time);
            }
            else if constexpr (IsTimeUnit<Unit> and IsLengthUnit<RightUnitType>) {
                // 거리 / 시간 인 경우 -> 반환은 속력
                auto len = UnitCast<StandardLength>(right).Count();
                auto time = UnitCast<StandardTime>(*this).Count();
                return GameUnit<StandardSpeed>(len / time);
            }
            else if constexpr (IsForceUnit<Unit> and IsMassUnit<RightUnitType>) {
                // 힘 / 질량 인 경우 -> 반환은 가속력
                //auto force = UnitCast<StandardForce>(*this).Count();
                auto force = Count();
                auto mass = UnitCast<StandardMass>(right).Count();
                return GameUnit<StandardAccel>(force / mass);
            }
            else if constexpr (IsMassUnit<Unit> and IsForceUnit<RightUnitType>) {
                // 힘 / 질량 인 경우 -> 반환은 가속력
                auto force = UnitCast<StandardForce>(right).Count();
                auto mass = UnitCast<StandardMass>(*this).Count();
                return GameUnit<StandardAccel>(force / mass);
            }
        }

        friend std::ostream& operator<<(std::ostream& os, GameUnit unit) {
            std::cout << std::format("{} {}", unit.Count(), GetLiteralUnitSuffix<char, GameUnit>());
            return os;
        }

    private:
        Rep mRep;
    };

    // 다른 단위로의 변환 ex) m/s -> km/h (속도) 같은 단위로의 변환만 가능
    template <typename To, typename UnitType>
        requires IsUnitType<UnitType> and IsUnitType<To> and IsConvertibleUnit<GameUnit<UnitType>, GameUnit<To>>
    constexpr GameUnit<To> UnitCast(GameUnit<UnitType> unit) noexcept {
        using ConversionFactor = std::ratio_divide<typename UnitType::UnitRatio, typename To::UnitRatio>;
        using CommonRep = std::common_type_t<float, intmax_t>;

        constexpr bool ratioDenIsOne = 1 == ConversionFactor::den;
        constexpr bool ratioNumIsOne = 1 == ConversionFactor::num;

        if constexpr (ratioDenIsOne) {
            if constexpr (ratioNumIsOne) {
                return GameUnit<To>(static_cast<float>(unit.Count()));
            }
            else {
                return GameUnit<To>(
                    static_cast<CommonRep>(unit.Count()) * static_cast<CommonRep>(ConversionFactor::num)
                );
            }
        }
        else {
            if constexpr (ratioNumIsOne) {
                return GameUnit<To>(
                    static_cast<CommonRep>(unit.Count()) / static_cast<CommonRep>(ConversionFactor::den)
                );
            }
            else {
                return GameUnit<To>(
                    static_cast<CommonRep>(unit.Count()) *
                    static_cast<CommonRep>(ConversionFactor::num) / static_cast<CommonRep>(ConversionFactor::den)
                );
            }
        }
    }

    template <typename UnitType>
    constexpr auto ToStandard(GameUnit<UnitType> unit) {
        if constexpr (IsSpeedUnit<UnitType>) {
            return UnitCast<StandardSpeed>(unit);
        }
        else if constexpr (IsLengthUnit<UnitType>) {
            return UnitCast<StandardLength>(unit);
        }
        else if constexpr (IsMassUnit<UnitType>) {
            return UnitCast<StandardMass>(unit);
        }
        else if constexpr (IsTimeUnit<UnitType>) {
            return UnitCast<StandardTime>(unit);
        }
        else if constexpr (IsForceUnit<UnitType>) {
            return UnitCast<StandardForce>(unit);
        }
    }

    template <typename UnitType>
    auto ToUnit(float unit) {
        if constexpr (IsSpeedUnit<UnitType>) {
            return GameUnit<StandardSpeed>(unit);
        }
        else if constexpr (IsLengthUnit<UnitType>) {
            return GameUnit<StandardLength>(unit);
        }
        else if constexpr (IsMassUnit<UnitType>) {
            return GameUnit<StandardMass>(unit);
        }
        else if constexpr (IsTimeUnit<UnitType>) {
            return GameUnit<StandardTime>(unit);
        }
        else if constexpr (IsForceUnit<UnitType>) {
            return GameUnit<StandardForce>(unit);
        }
    }

    // Speed To Velocity
    template <typename UnitType> requires IsSpeedUnit<UnitType>
    SimpleMath::Vector3 ToVelocity(const SimpleMath::Vector3& dir, GameUnit<UnitType> speed) {
        return dir * speed.Count();
    }

    // Force with Direction
    template <typename UnitType> requires IsForceUnit<UnitType>
    SimpleMath::Vector3 ToDirForce(const SimpleMath::Vector3& dir, GameUnit<UnitType> magnitute) {
        return dir * magnitute.Count();
    }

#define IF_UNIT_RETURN_SUFFIX_ELSE(_TYPE, _SUFFIX)         \
    if constexpr (std::is_same_v<_Unit, _TYPE>) {        \
        if constexpr (std::is_same_v<_CharT, char>) {   \
            return _SUFFIX;                             \
        } else {                                        \
            return L##_SUFFIX;                          \
        }                                               \
    } else

    template <class _CharT, class _Unit>
    _NODISCARD constexpr const _CharT* GetLiteralUnitSuffix() {
        IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<CentiMeter>, "cm")
            IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<Meter>, "m")
            IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<KilloMeter>, "km")
            IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<MeterPerSec>, "m/s")
            IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<MeterPerHour>, "m/h")
            IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<KilloMeterPerSec>, "km/s")
            IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<KilloMeterPerHour>, "km/h")
            IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<MeterPerSec2>, "m/s^2")
            IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<Newton>, "N")
            IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<Second>, "sec")
            IF_UNIT_RETURN_SUFFIX_ELSE(GameUnit<Hour>, "hour")
        {
            return nullptr;
        }
    }

#undef _IF_PERIOD_RETURN_SUFFIX_ELSE

}

namespace GameUnitLiterals {
#pragma warning(disable: 4455)
    // speed
    constexpr GameUnits::GameUnit<GameUnits::MeterPerSec> operator""mps(long double mps) {
        return GameUnits::GameUnit<GameUnits::MeterPerSec>(mps);
    }

    constexpr GameUnits::GameUnit<GameUnits::MeterPerHour> operator""mph(long double mph) {
        return GameUnits::GameUnit<GameUnits::MeterPerHour>(mph);
    }

    constexpr GameUnits::GameUnit<GameUnits::KilloMeterPerSec> operator""kmps(long double kmps) {
        return GameUnits::GameUnit<GameUnits::KilloMeterPerSec>(kmps);
    }

    constexpr GameUnits::GameUnit<GameUnits::KilloMeterPerHour> operator""kmph(long double kmph) {
        return GameUnits::GameUnit<GameUnits::KilloMeterPerHour>(kmph);
    }

    constexpr GameUnits::GameUnit<GameUnits::MeterPerSec2> operator""mps2(long double mps2) {
        return GameUnits::GameUnit<GameUnits::MeterPerSec2>(mps2);
    }

    // length
    constexpr GameUnits::GameUnit<GameUnits::CentiMeter> operator""cm(long double centimeter) {
        return GameUnits::GameUnit<GameUnits::CentiMeter>(centimeter);
    }

    constexpr GameUnits::GameUnit<GameUnits::Meter> operator""m(long double meter) {
        return GameUnits::GameUnit<GameUnits::Meter>(meter);
    }

    constexpr GameUnits::GameUnit<GameUnits::KilloMeter> operator""km(long double killometer) {
        return GameUnits::GameUnit<GameUnits::KilloMeter>(killometer);
    }

    // time
    constexpr GameUnits::GameUnit<GameUnits::Second> operator""sec(long double second) {
        return GameUnits::GameUnit<GameUnits::Second>(second);
    }

    constexpr GameUnits::GameUnit<GameUnits::Hour> operator""hour(long double hour) {
        return GameUnits::GameUnit<GameUnits::Hour>(hour);
    }

    // mass
    constexpr GameUnits::GameUnit<GameUnits::KilloGram> operator""kg(long double killogram) {
        return GameUnits::GameUnit<GameUnits::KilloGram>(killogram);
    }

    constexpr GameUnits::GameUnit<GameUnits::MeterPerSec2> operator""G(long double gravity) {
        return GameUnits::GameUnit<GameUnits::MeterPerSec2>(gravity * 9.8);
    }

    // force
    constexpr GameUnits::GameUnit<GameUnits::Newton> operator""N(long double newton) {
        return GameUnits::GameUnit<GameUnits::Newton>(newton);
    }
}