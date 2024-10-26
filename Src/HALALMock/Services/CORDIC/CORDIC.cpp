/*
 * CORDIC.cpp
 *
 * Created on: 29 nov. 2022
 * 		Author: Ricardo
 */

#include "HALALMock/Services/CORDIC/CORDIC.hpp"


Operation_Computation RotationComputer::mode = NONE;

void RotationComputer::start(){
	__HAL_RCC_CORDIC_CLK_ENABLE();
}

void RotationComputer::cos(int32_t *angle, int32_t *out, int32_t size){
	if(RotationComputer::mode != COSINE){
		RotationComputer::mode = COSINE;
		MODIFY_REG(CORDIC -> CSR,
					RESET_MASK,
					COSINE_CONFIG
		);
	}
	for(int n = 0; n < size; n++){
		CORDIC -> WDATA = angle[n];
		out[n] = CORDIC -> RDATA;
	}
}

void RotationComputer::sin(int32_t *angle, int32_t *out, int32_t size){
	if(RotationComputer::mode != SINE){
		RotationComputer::mode = SINE;
		MODIFY_REG(CORDIC -> CSR,
					RESET_MASK,
					SINE_CONFIG
		);
	}
	for(int n = 0; n < size; n++){
		CORDIC -> WDATA = angle[n];
		out[n] = CORDIC -> RDATA;
	}
}

void RotationComputer::cos_and_sin(int32_t *angle, int32_t *cos_out, int32_t *sin_out, int32_t size){
	if(RotationComputer::mode != SINE_COSINE){
		RotationComputer::mode = SINE_COSINE;
		MODIFY_REG(CORDIC -> CSR,
					RESET_MASK,
					SINE_COSINE_CONFIG
		);
	}
	for(int n = 0; n < size; n++){
		CORDIC -> WDATA = angle[n];
		cos_out[n] = CORDIC -> RDATA;
		sin_out[n] = CORDIC -> RDATA;
	}
}

void RotationComputer::phase(int32_t *x, int32_t *y, int32_t *angle_out, int32_t size){
	if(RotationComputer::mode != PHASE){
		RotationComputer::mode = PHASE;
		MODIFY_REG(CORDIC -> CSR,
					RESET_MASK,
					PHASE_CONFIG
		);
	}
	for(int n = 0; n < size; n++){
		CORDIC -> WDATA = x[n];
		CORDIC -> WDATA = y[n];
		angle_out[n] = CORDIC -> RDATA;
	}
}

void RotationComputer::modulus(int32_t *x, int32_t *y, int32_t *out, int32_t size){
	if(RotationComputer::mode != MODULUS){
		RotationComputer::mode = MODULUS;
		MODIFY_REG(CORDIC -> CSR,
					RESET_MASK,
					MODULUS_CONFIG
		);
	}
	for(int n = 0; n < size; n++){
		CORDIC -> WDATA = x[n];
		CORDIC -> WDATA = y[n];
		out[n] = CORDIC -> RDATA;
	}
}

void RotationComputer::phase_and_modulus(int32_t *x, int32_t *y, int32_t *angle_out, int32_t *mod_out, int32_t size){
	if(RotationComputer::mode != PHASE_MODULUS){
		RotationComputer::mode = PHASE_MODULUS;
		MODIFY_REG(CORDIC -> CSR,
					RESET_MASK,
					PHASE_MODULUS_CONFIG
		);
	}
	for(int n = 0; n < size; n++){
		CORDIC -> WDATA = x[n];
		CORDIC -> WDATA = y[n];
		angle_out[n] = CORDIC -> RDATA;
		mod_out[n] = CORDIC -> RDATA;
	}
}


float RotationComputer::q31_to_radian_f32(uint32_t in){
	return M_PI*ldexp((int32_t) in, -31);
}

int32_t RotationComputer::radian_f32_to_q31(double in){
	return (int)roundf(scalbnf(fmaxf(fminf(in/M_PI,0.9995),-0.9995),31));
}

