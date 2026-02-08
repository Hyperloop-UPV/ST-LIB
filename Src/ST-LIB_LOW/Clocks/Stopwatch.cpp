/*
 * Time.hpp
 *
 *  Created on: 11 nov. 2022
 *      Author: Dani
 */

#include "Clocks/Stopwatch.hpp"

#include "HALAL/Services/Time/Time.hpp"


void Stopwatch::start(const string id){
	start_times[id] = Time::get_global_tick();
}

uint64_t Stopwatch::stop(const string id){
	if(not start_times.contains(id)){
		ErrorHandler("No encoder registered with id %u", id);
		return 0;
	}

	uint64_t result = Time::get_global_tick() - start_times[id];
	start(id);
	return result;
}
