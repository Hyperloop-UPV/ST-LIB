/*
 * CORDIC.hpp
 *
 * Created on: 29 nov. 2022
 * 		Author: Ricardo
 */

#pragma once

#include "HALAL/Models/PinModel/Pin.hpp"
#ifdef HAL_CORDIC_MODULE_ENABLED
#define RESET_MASK 0x007F07FF
#define COSINE_CONFIG 0x00000050
#define SINE_CONFIG 0x00000051
#define SINE_COSINE_CONFIG 0x00080050
#define PHASE_CONFIG 0x00100052
#define MODULUS_CONFIG 0x00100053
#define PHASE_MODULUS_CONFIG 0x00180052

/**
 * @brief Cordic class. Has some trigonometric static functions with high speed computation time
 *
 * The cordic class calculates with integers cosine, sine, phase and modulus twice as fast as
 * arm_q31 functions do The class uses its own units (as arm_q31 methods do) to traduce angles,
 * coordinates, and results to have the maximun precision Angles go in the range of [-pi, pi] ->
 * [-max int32, max int32] radians for ints and [0,2*pi] -> [0, max uint32] for uints. They are
 * equivalent Coordinates go in the range of [-1,1] -> [-max int32, max int32] for ints (they work
 * on uints but not in a logical way). If we wanted to represent any euclidean space we would need
 * to delimit and scale it. Precision of these functions are all around 5*10^-10 The methods just
 * check if the cordic is correctly configurated, configurate it if thats not the case, then writes
 * inside the CORDIC and reads the results They work with vectors so you can ask for multiple values
 * so it skips the small overhead after the first.
 *
 *
 * The CORDIC configuration work as follows
 * In the CSR CORDIC register you can directly configurate the CORDIC by writting on it.
 * The masks for each variable that you can configurate are:
 * Functions : 0x0000000F
 * (0.COSINE 1.SINE 2.PHASE 3.MODULUS 4.ARCTANGENT 5.HCOSINE 6.HSINE 7.HARCTANGENT 8.NATLOG 9.SQRT)
 * Precision : 0x000000F0 (0.1_cycle 1.2_cycles 2.3_cycles ...) (we use on all of them 6, the
 * maximum for the CORDIC inside the stm (can configurate more but does nothing)) Scale : 0x00000700
 * (0 = 0, 1 = 1 ... means that the in has been shifted to the right and the output needs to be
 * shifted to the left) (we do not use this, 0) Interrupt enabled : 0x00010000
 * (0.disabled 1.enabled) (only useful for DMA, which we do not use) DMA read and write : 0x00020000
 * 0x00040000 (respectively) (same as abode) In and out arguments: 0x00100000 0x00080000
 * (respectively) (0.1argument 1.2arguments) (depends on the function and if we want residual
 * results) Width input and output: 0x00200000 0x00400000 (respectively) (0.32bytes 1.16bytes) (we
 * use max size for more precision, 0) Ready flag : 0x80000000 (this is to check if the CORDIC is
 * ready, but the bus already handles all the transactions on the CORDIC inside the stm so we don t
 * need to waste processing time on this)
 */

enum Operation_Computation { COSINE, SINE, SINE_COSINE, PHASE, MODULUS, PHASE_MODULUS, NONE };

class RotationComputer {
public:
    static void start();
    /**
     * @brief Cosine function. Receives size angles and output size results
     * @param Angle the pointer to the angle array with size "size". input
     * @param Out the pointer to the array where the results are saved, with size "size". output
     * @param Size the size of both pointers in indexes.
     *
     * The cosine queries on single shot each of the values on the angle array until size index
     * If the angle or the out parameter has less size than the size parameter, it will throw an
     * exception If one or both angle and out parameters have more size than size, it will just
     * calculate until size and leave the rest as it is
     */
    static void cos(int32_t* angle, int32_t* out, int32_t size);

    /**
     * @brief sine function. virtually the same as the cosine function
     */
    static void sin(int32_t* angle, int32_t* out, int32_t size);

    /**
     * @brief Cosine and sine function. Does both while only needing the time that one takes
     * @param Angle the pointer to the angle array with size "size". input
     * @param Cos_out the pointer to the array where the cosine results are saved, with size "size".
     * output
     * @param Sin:out the pointer to the array where the sine results are saved, with size "size".
     * output
     * @param Size the size of both pointers in indexes.
     *
     * Uses a cosine query but configurating the CORDIC to save the residual results
     * The residual results are values that appear when calculating any function, which are not the
     * return of the function itself The cosine function of the CORDIC gives as residual result the
     * sine function, so if you read twice you will get first cosine then sine If you don t
     * configurate the CORDIC to save its residual result it will not work. The method does it by
     * itself, thats why we don t just use the cosine method. The exact time it takes is around 4
     * cycles more than the cos function, which is around 30-40. So 10% more as it needs to read one
     * more register. The reads are protected to give the correct result, thats why it needs 4
     * cycles for the read (on average) and not just one
     */
    static void cos_and_sin(int32_t* angle, int32_t* cos_out, int32_t* sin_out, int32_t size);

    /**
     * @brief Phase function. Calculates the angle between a vector from (0,0) to (x,y) and the
     * vector (1,0)
     * @param x. The array of values x of each vector with size "size". input.
     * @param y. The array of values y of each vector with size "size". input.
     * @param angle_out. The array where the functions puts its output, which is an angle. output.
     */
    static void phase(int32_t* x, int32_t* y, int32_t* angle_out, int32_t size);

    /**
     * @brief modulus function. Calculates the modulus of the vector from (0,0) to (x,y). If the
     * modulus result where to be higher than 2147483392 (~0.995) it will give aberrant result
     * instead
     * @param x. The array of values x of each vector with size "size". input.
     * @param y. The array of values y of each vector with size "size". input.
     * @param out. The array where the functions puts its output, cannot be higher than 2147483392.
     * output.
     *
     * The modulus function is a bit more fragile than the other functions.
     * If it were to give a value higher than 2147483392, the value that it would return would be
     * absurd. Avoiding to get near the limits of the euclidean space described (no more than 70% on
     * both of the axis) is advised.
     */
    static void modulus(int32_t* x, int32_t* y, int32_t* out, int32_t size);

    /**
     * @brief modulus and phase function. Does both while only needing the time that one takes
     * @param x. The array of values x of each vector with size "size". input.
     * @param y. The array of values y of each vector with size "size". input.
     * @param angle_out. The array of results of the phase, given as an angle. output.
     * @param mod_out. The array of results of the modulus, with same sepcifications. output.
     *
     * This function uses the same trick as the cosine and sine function, as the residual result of
     * the phase is the modulus. The modulus returned by this function shares weaknesses with the
     * modulus function. Avoiding getting near the 70% of the maximun (~ 1518500000) on both x and y
     * at the same time is advised, unless modulus is handled.
     */
    static void
    phase_and_modulus(int32_t* x, int32_t* y, int32_t* angle_out, int32_t* mod_out, int32_t size);
    /**
     * @brief Testing function that translates the angle to a float in radians, to make reading
     * easier. Its slow, not intended to use on continous calculations
     */
    static float q31_to_radian_f32(uint32_t in);
    /**
     * @brief Testing function that translates radians to an angle on int format, to make writting
     * easier. Its slow, not intended to use on continous calculations
     */
    static int32_t radian_f32_to_q31(double in);

private:
    /**
     * @brief The mode that the cordic is configurated at this instant. Used to skip configuration
     * if possible
     */
    static Operation_Computation mode;
};

#endif
