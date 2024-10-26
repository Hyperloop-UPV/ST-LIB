#include "Sensors/EncoderSensor/EncoderSensor.hpp"

EncoderSensor::EncoderSensor(Pin pin1, Pin pin2, double *position, double* direction, double *speed, double *acceleration)
: position(position), direction(direction), speed(speed), acceleration(acceleration), idx_buff(0){
	id = Encoder::inscribe(pin1,pin2);
}

void EncoderSensor::start(){
	Encoder::reset(id);
	Encoder::turn_on(id);
	uint64_t clock_time = Time::get_global_tick();
	for(int i = 0; i < N_FRAMES; i++){
		positions[i] = 0.0;

		times[i] = i * FRAME_SIZE_IN_SECONDS - N_FRAMES*FRAME_SIZE_IN_SECONDS + (clock_time / NANO_SECOND);

		speeds[i] = 0.0;
	}
	time = clock_time / NANO_SECOND;
	last_clock_time = clock_time;
}

void EncoderSensor::read(){
	uint32_t counter = Encoder::get_counter(id);
	uint64_t clock_time = Time::get_global_tick();
	*direction = (double)Encoder::get_direction(id);

	int64_t delta_clock = clock_time - last_clock_time;
	if(clock_time < last_clock_time){ //overflow handle
		delta_clock = clock_time + CLOCK_MAX_VALUE * NANO_SECOND / HAL_RCC_GetPCLK1Freq()*2 - last_clock_time;
	}
	time = time + delta_clock / NANO_SECOND;
	last_clock_time = clock_time;

	*position= ((int32_t)(counter - START_COUNTER)) * COUNTER_DISTANCE_IN_METERS;
	uint32_t last_index = (idx_buff+1)%N_FRAMES;
	double delta_time = time - times[last_index];
	double delta_position = *position - positions[last_index];

	*speed = abs(delta_position) / (delta_time);

	double delta_speed = *speed - speeds[last_index];

	*acceleration = (delta_speed) / (delta_time);

	if(time - times[idx_buff] >= FRAME_SIZE_IN_SECONDS){
		EncoderSensor::update_arrays();
	}
}

void EncoderSensor::reset(){
	Encoder::reset(id);
}

uint8_t EncoderSensor::get_id(){
	return id;
}

void EncoderSensor::update_arrays(){
	idx_buff = (idx_buff+1)%N_FRAMES;
	positions[idx_buff] = *position;
	times[idx_buff] = time;
	speeds[idx_buff] = *speed;
}
