/*
 * CORDIC.hpp
 *
 * Created on: 13 dic. 2022
 *         Author: Ricardo
 */

#pragma once
#include <array>
#include <cstdint>
#define TG_DECIMAL_BITS 16
#define SQ_DECIMAL_BITS 16
#define MAX_INT 2147483647
#define MIN_COS_MARGIN_TG (MAX_INT >> (TG_DECIMAL_BITS-1))
#define MAX_MOD_MARGIN 2147483392
#define MAX_MARGIN 2147483392
#define Q31_MASK 0x7FFFFFFF


class Math{
public:
    static int32_t sin(int32_t angle);
    static int32_t cos(int32_t angle);
    static int32_t tg(int32_t angle);
    static int32_t phase(int32_t x, int32_t y);
    static uint32_t modulus(int32_t x, int32_t y);
    static int32_t atg(int32_t tg_in);
    static int32_t sqrt(int32_t sq_in);

    static inline int32_t sq_to_unitary(int32_t sq_in);
    static inline int32_t unitary_to_sq(int32_t in);
    static inline int32_t tg_to_unitary(int32_t tg_in);
    static inline int32_t unitary_to_tg(int32_t in);
private:
    static std::array<int32_t,4> pointers;
};
