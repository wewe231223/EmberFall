#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// UnitLiterals.h
// 
// 2025 - 02 - 12 : 게임에서 사용할 단위들을 정의.
//                  단위가 붙어야하는 정수들에는 무조건 literal을 붙이도록 할 예정
// 
//                 : 단위간 변환은 조금 더 생각해 봐야한다.
//                   지금 UnitCast 함수는 단위간 변환을 고려하지 않는다.
//                   단위간 변환을 제약할 수 있도록 하는 시스템을 만들어야할 필요성이 있음.
//                  
// 
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <ratio>
#include "Types.h"

namespace GameUnit {
    using CentiMeter = std::ratio<1, 100>;
    using Meter = std::ratio<1, 1>;
    using KilloMeter = std::ratio<1000, 1>;

    using Second = std::ratio<1, 1>;
    using Hour = std::ratio<3600, 1>;

    using MeterPerSec = std::ratio<1, 1>;
    using MeterPerHour = std::ratio<1, 3600>;
    using KilloMeterPerSec = std::ratio<1000, 1>;
    using KilloMeterPerHour = std::ratio<1000, 3600>;

    using Gram = std::ratio<1, 1000>;
    using KilloGram = std::ratio<1, 1>;
    //using Ton = std::ratio<1000, 1>;

    using Centi = std::centi;
    using Milli = std::milli;

    template <typename UnitRatio>
    concept IsUnitRatio = IsAnyOf<UnitRatio, Meter, CentiMeter, Second, Hour, MeterPerSec, MeterPerHour, KilloMeterPerSec, KilloMeterPerHour>;

    template <typename UnitType>
    concept IsUnit = true;

    template <typename UnitType>
    concept IsLengthUnit = IsAnyOf<UnitType, Meter, CentiMeter, KilloMeter>;

    template <typename UnitType>
    concept IsSpeedUnit = IsAnyOf<UnitType, MeterPerSec, MeterPerHour, KilloMeterPerSec, KilloMeterPerHour>;

    template <typename UnitType>
    concept IsMassUnit = IsAnyOf<UnitType, Gram, KilloGram>;

    template <typename UnitType>
    concept IsTimeUnit = IsAnyOf<UnitType, Second, Hour>;

    template <typename Unit1, typename Unit2>
    concept IsConvertible = (IsMassUnit<Unit1> and IsMassUnit<Unit2>) or (IsLengthUnit<Unit1> and IsLengthUnit<Unit2>) or
        (IsTimeUnit<Unit1> and IsTimeUnit<Unit2>) or (IsSpeedUnit<Unit1> or IsSpeedUnit<Unit2>);

    template <typename Rep, typename UnitRatio> requires std::is_arithmetic_v<Rep> and IsUnitRatio<UnitRatio>
    class Unit {
    public:
        using Rep = Rep;
        using Ratio = UnitRatio;

        constexpr Unit() = default;

        template <typename Rep2> requires std::is_arithmetic_v<Rep2> and std::is_convertible_v<Rep2, Rep>
        constexpr Unit(Rep2& rep) : mRep{ static_Cast<Rep>(rep) } { }

        template <typename Rep2, typename UnitRatio2>
        constexpr Unit(const Unit<Rep2, UnitRatio2>& unit) : mRep{ UnitCast<Unit>(unit).Count(); }

        constexpr Rep Count() const {
            return mRep;
        }

    private:
        Rep mRep;
    };

    template <typename To, typename Rep, typename UnitRatio> requires IsUnitRatio<UnitRatio>
    constexpr To UnitCast(const Unit<Rep, UnitRatio>& unit) noexcept {
        using ToRep = typename To::Rep;

        using ConversionFactor = std::ratio_divide<To::UnitRatio, UnitRatio>;
        using CommonRep = std::common_type_t<ToRep, Rep, intmax_t>;

        constexpr bool ratioDenIsOne = ConversionFactor::Den == 1;
        constexpr bool ratioNumIsOne = ConversionFactor::Num == 1;

        if constexpr (ratioDenIsOne) {
            if constexpr (ratioNumIsOne) {
                return static_cast<To>(static_cast<ToRep>(unit.Count()));
            }
            else {
                return static_cast<To>(
                    static_cast<ToRep>(static_cast<CommonRep>(unit.Count()) * static_cast<CommonRep>(ConversionFactor::num))
                );
            }
        }
        else {
            if constexpr (ratioNumIsOne) {
                return static_cast<To>(
                    static_cast<ToRep>(static_cast<CommonRep>(unit.Count()) / static_cast<CommonRep>(ConversionFactor::den))
                );
            }
            else {
                return static_cast<To>(static_cast<ToRep>(
                    static_cast<CommonRep>(unit.Count()) * static_cast<CommonRep>(ConversionFactor::num) / static_cast<CommonRep>(ConversionFactor::den))
                );
            }
        }
    }

    // speed
    Unit<float, MeterPerSec> operator""mps(long double mps);
    Unit<float, MeterPerHour> operator""mph(long double mph);
    Unit<float, KilloMeterPerSec> operator""kmps(long double kmps);
    Unit<float, KilloMeterPerHour> operator""kmph(long double kmph);

    // length
    Unit<float, CentiMeter> operator""cm(long double centimeter);
    Unit<float, Meter> operator""m(long double meter);
    Unit<float, KilloMeter> operator""km(long double killometer);

    // time
    Unit<float, Second> operator""sec(long double second);
    Unit<float, Hour> operator""hour(long double hour);

    // Mass
    Unit<float, Gram> operator""kg(long double killogram);

    float operator""g(long double gravity);
}

using StandardLength = GameUnit::Meter;
using StandardTime = GameUnit::Second;
using StandardMass = GameUnit::KilloGram;