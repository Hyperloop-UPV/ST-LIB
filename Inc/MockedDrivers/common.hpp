#pragma once

#ifndef glue
#define glue_(a, b) a##b
#define glue(a, b) glue_(a, b)
#endif

#undef __I
#undef __I

/* following defines should be used for structure members */

#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

#ifdef SET_BIT
#undef SET_BIT
#endif
#ifdef CLEAR_BIT
#undef CLEAR_BIT
#endif
#ifdef WRITE_REG
#undef WRITE_REG
#endif
#ifdef MODIFY_REG
#undef MODIFY_REG
#endif

// this is needed because later I will do a #define uint32_t size_t
typedef uint32_t u32;

#define SET_BIT(REG, BIT) ((REG) |= (BIT))

#define CLEAR_BIT(REG, BIT)                                                                        \
    ((REG) = static_cast<u32>(static_cast<u32>(REG) & ~static_cast<u32>(BIT)))

#define READ_BIT(REG, BIT) ((REG) & (BIT))

#define CLEAR_REG(REG) ((REG) = (0x0))

#define WRITE_REG(REG, VAL) ((REG) = static_cast<u32>(VAL))

#define READ_REG(REG) ((REG))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)                                                        \
    WRITE_REG((REG), (((READ_REG(REG)) & (~(u32)(CLEARMASK))) | (SETMASK)))

#define POSITION_VAL(VAL) (__CLZ(__RBIT(VAL)))
