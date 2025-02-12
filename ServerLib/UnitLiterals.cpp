#include "pch.h"
#include "UnitLiterals.h"

using namespace GameUnit;

Unit<float, MeterPerSec> operator""mps(long double mps) {
    return Unit<float, MeterPerSec>{ mps };
}

Unit<float, MeterPerHour> operator""mph(long double mph) {
    return Unit<float, MeterPerHour>{ mph };
}

Unit<float, KilloMeterPerSec> operator""kmps(long double kmps) {
    return Unit<float, KilloMeterPerSec>{ kmps };
}

Unit<float, KilloMeterPerHour> operator""kmph(long double kmph) {
    return Unit<float, KilloMeterPerHour>{ kmph };
}

Unit<float, CentiMeter> operator""cm(long double centimeter) {
    return Unit<float, CentiMeter>{ centimeter };
}

Unit<float, Meter> operator""m(long double meter) {
    return Unit<float, Meter>{ meter };
}

Unit<float, KilloMeter> operator""km(long double killometer) {
    return Unit<float, KilloMeter>{ killometer };
}

Unit<float, Second> operator""sec(long double second) {
    return Unit<float, Second>{ second };
}

Unit<float, Hour> operator""hour(long double hour) {
    return Unit<float, Hour>{ hour };
}

Unit<float, KilloGram> operator""kg(long double killogram) {
    return Unit<float, KilloGram>{ killogram };
}

float operator""g(long double gravity) {
    return gravity * 9.8;
}
