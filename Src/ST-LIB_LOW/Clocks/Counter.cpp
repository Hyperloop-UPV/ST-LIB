/*
 * Time.hpp
 *
 *  Created on: 11 nov. 2022
 *      Author: Dani
 */

#include <Clocks/Counter.hpp>
#include <HALAL/HALAL.hpp>

void Counter::update(){
	this->freq = (this->counter + 0.0)/(this->update_period_ms / 1000.0);
	this->counter = 0;
}

Counter::Counter(uint64_t update_period_ms) : update_period_ms(update_period_ms){
	this->time_id = Time::register_low_precision_alarm(this->update_period_ms,[&](){ this->update(); });
}

Counter::~Counter(){
	Time::unregister_low_precision_alarm(this->time_id);
}

void Counter::count(){
	this->counter++;
}
