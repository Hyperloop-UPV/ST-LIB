#include "ST-LIB_LOW/Math/Math.hpp"

#include "HALAL/Services/CORDIC/CORDIC.hpp"

#ifndef SIM_ON
std::array<int32_t, 4> Math::pointers = {0};

int32_t Math::sin(int32_t angle) {
    pointers[0] = angle;
    RotationComputer::sin(&pointers[0], &pointers[1], 1);
    return pointers[1];
}

int32_t Math::cos(int32_t angle) {
    pointers[0] = angle;
    RotationComputer::cos(&pointers[0], &pointers[1], 1);
    return pointers[1];
}

int32_t Math::tg(int32_t angle) {
    pointers[0] = angle;
    RotationComputer::cos_and_sin(&pointers[0], &pointers[1], &pointers[2], 1);
    if (pointers[1] > MIN_COS_MARGIN_TG || pointers[1] < -MIN_COS_MARGIN_TG) {
        return pointers[2] / (pointers[1] >> TG_DECIMAL_BITS);
    }
    if ((pointers[1] >= 0) == (pointers[2] >= 0)) {
        return MAX_INT;
    }
    return -MAX_INT - 1;
}

int32_t Math::phase(int32_t x, int32_t y) {
    pointers[0] = x;
    pointers[1] = y;
    RotationComputer::phase(&pointers[0], &pointers[1], &pointers[2], 1);
    return pointers[2];
}

uint32_t Math::modulus(int32_t x, int32_t y) {
    if (((uint32_t)x - 4 + y) > MAX_MOD_MARGIN) {
        pointers[0] = (x >> 1);
        pointers[1] = (y >> 1);
        RotationComputer::modulus(&pointers[0], &pointers[1], &pointers[2], 1);
        uint32_t mod = pointers[2];
        mod = (mod << 1);
        return mod;
    }
    pointers[0] = x;
    pointers[1] = y;
    RotationComputer::modulus(&pointers[0], &pointers[1], &pointers[2], 1);
    return pointers[2];
}

int32_t Math::atg(int32_t in) {
    pointers[0] = 1 << TG_DECIMAL_BITS;
    pointers[1] = in;
    RotationComputer::phase(&pointers[0], &pointers[1], &pointers[2], 1);
    return pointers[2];
}

int32_t Math::sqrt(int32_t sq_in) {
    int32_t sol = 0;
    int32_t temp = 0;
    int32_t over = sq_in >> SQ_DECIMAL_BITS;
    for (int32_t i = 0x8000 >> (SQ_DECIMAL_BITS >> 1); i >= 1; i = i >> 1) {
        temp = (sol + i);
        if (((temp * temp)) <= over) {
            sol = sol + i;
        }
    }

    return sol << SQ_DECIMAL_BITS;
}

int32_t Math::sq_to_unitary(int32_t sq_in) { return sq_in << (32 - SQ_DECIMAL_BITS); }

int32_t Math::unitary_to_sq(int32_t in) { return in >> (32 - SQ_DECIMAL_BITS); }

int32_t Math::tg_to_unitary(int32_t tg_in) { return tg_in << (32 - TG_DECIMAL_BITS); }

int32_t Math::unitary_to_tg(int32_t in) { return in >> (32 - TG_DECIMAL_BITS); }

#else
#include <cmath>

int32_t Math::sin(int32_t angle) {
    double rad_angle = angle * M_PI / 180.0;
    return static_cast<int32_t>(std::sin(rad_angle) * (1 << TG_DECIMAL_BITS));
}

int32_t Math::cos(int32_t angle) {
    double rad_angle = angle * M_PI / 180.0;
    return static_cast<int32_t>(std::cos(rad_angle) * (1 << TG_DECIMAL_BITS));
}

int32_t Math::tg(int32_t angle) {
    double rad_angle = angle * M_PI / 180.0;
    return static_cast<int32_t>(std::tan(rad_angle) * (1 << TG_DECIMAL_BITS));
}

int32_t Math::phase(int32_t x, int32_t y) {
    double rad_angle = std::atan2(y, x);
    return static_cast<int32_t>(rad_angle * (1 << TG_DECIMAL_BITS));
}

uint32_t Math::modulus(int32_t x, int32_t y) {
    if (x - 4 + y > MAX_MOD_MARGIN) {
        x = x >> 1;
        y = y >> 1;
    }
    double x_double = static_cast<double>(x);
    double y_double = static_cast<double>(y);
    double x_squared = x_double * x_double;
    double y_squared = y_double * y_double;
    double mod = std::sqrt(x_squared + y_squared);
    return static_cast<uint32_t>(mod * (1 << SQ_DECIMAL_BITS));
}

int32_t Math::atg(int32_t tg_in) {
    double rad_angle = std::atan(tg_in);
    return static_cast<int32_t>(rad_angle * (1 << TG_DECIMAL_BITS));
}

int32_t Math::sqrt(int32_t sq_in) {
    double decimal_sq_in = static_cast<double>(sq_in);
    return static_cast<int32_t>(std::sqrt(decimal_sq_in) * (1 << SQ_DECIMAL_BITS));
}

int32_t Math::sq_to_unitary(int32_t sq_in) { return sq_in << (32 - SQ_DECIMAL_BITS); }

int32_t Math::unitary_to_sq(int32_t in) { return in >> (32 - SQ_DECIMAL_BITS); }

int32_t Math::tg_to_unitary(int32_t tg_in) { return tg_in << (32 - TG_DECIMAL_BITS); }

int32_t Math::unitary_to_tg(int32_t in) { return in >> (32 - TG_DECIMAL_BITS); }

#endif // SIM_ON
