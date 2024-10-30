
/*
 * AnalogSensor.hpp
 *
 *  Created on: Nov 9, 2022
 *      Author: ricardo
 */

#pragma once
#include "HALAL/HALAL.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#define COUNTER_DISTANCE_IN_METERS 0.0001
#define NANO_SECOND 1000000000.0
#define N_FRAMES 100
#define FRAME_SIZE_IN_SECONDS 0.005
#define START_COUNTER UINT32_MAX / 2
#define CLOCK_MAX_VALUE 4294967295 //here goes the tim23 counter period



class EncoderSensor{
public:
	EncoderSensor() = default;
	EncoderSensor(Pin pin1, Pin pin2, double* position, double* direction, double* speed, double* acceleration);
	void start();
	void read();
	void reset();
	uint8_t get_id();

protected:
	uint8_t id;
	double* position;
	double* direction;
	double* speed;
	double* acceleration;
	double time;
	// double positions[N_FRAMES];
	// double times[N_FRAMES];
	// double speeds[N_FRAMES];
	RingBuffer<double, N_FRAMES> positions;
	RingBuffer<double, N_FRAMES> times;
	RingBuffer<double, N_FRAMES> speeds;
	uint64_t last_clock_time;

private:
	void update_arrays();
};
