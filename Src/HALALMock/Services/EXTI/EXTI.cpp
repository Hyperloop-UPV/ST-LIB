/*
 * EXTI.cpp
 *
 *  Created on: Nov 5, 2022
 *      Author: alejandro 
 */

#include "HALALMock/Services/EXTI/EXTI.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

uint8_t ExternalInterrupt::id_counter = 0;
map<uint8_t, Pin> ExternalInterrupt::service_ids = {};

std::mutex ExternalInterrupt::mutex;
std::thread ExternalInterrupt::interrupt_thread;
std::vector<ExternalInterrupt::Instance*> ExternalInterrupt::interrupts_arr = {};
bool ExternalInterrupt::is_running = false;

ExternalInterrupt::Instance::Instance(uint32_t interrupt_request_number) :
		interrupt_request_number(interrupt_request_number) {}


uint8_t ExternalInterrupt::inscribe(Pin& pin, function<void()>&& action, TRIGGER trigger) {
	if (not instances.contains(pin.gpio_pin)) {
		ErrorHandler(" The pin %s is already used or isn t available for EXTI usage", pin.to_string().c_str());
		return 0;
	}
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	if(pin_data.type != PinType::NOT_USED) {
		ErrorHandler("ID %d is not registered as a EXTIPin",id_counter);
		return 0;
	}
	pin_data.type = PinType::EXTIPin;
	(pin_data.PinData.exti.trigger_mode) = trigger;
	(pin_data.PinData.exti.is_on) = false;

	id_counter++;
	service_ids[id_counter] = pin;
	instances[pin.gpio_pin].action = action;
	instances[pin.gpio_pin].trigger_signal = &pin_data.PinData.exti.trigger_signal;
	*instances[pin.gpio_pin].trigger_signal = PinState::OFF;

	return id_counter;
}
//TODO: assigne priority on the emulated pin
void ExternalInterrupt::start() {
	is_running = true;
	interrupt_thread = std::thread(handle_interrupts);
}

void ExternalInterrupt::turn_on(uint8_t id) {
	if (not service_ids.contains(id)) {
		ErrorHandler("ID %d is not registered as an active instance", id);
		return;
	}

	Instance& instance = instances[service_ids[id].gpio_pin];
	instance.is_on=true;
	Pin& pin = service_ids[id];
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	if(pin_data.type != PinType::EXTIPin) {
		ErrorHandler("ID %d is not registered as a EXTIPin",id);
		return;
	}
	(pin_data.PinData.exti.is_on) = true;
}

bool ExternalInterrupt::get_pin_value(uint8_t id) {
	if (not service_ids.contains(id)) {
		ErrorHandler("ID %d is not registered as an active instance", id);
		return false;
	}

	Pin& pin = service_ids[id];
	EmulatedPin &pin_data = SharedMemory::get_pin(pin);
	if(pin_data.type != PinType::EXTIPin) {
		ErrorHandler("ID %d is not registered as a EXTIPin",id);
		std::terminate();
	}
	return (pin_data.PinData.exti.trigger_signal);
}

//IMPORTANT EXECUTE STOP ON FINALLY SECUENCE
void ExternalInterrupt::stop()
{
	std::unique_lock<std::mutex> lock(mutex);
	is_running = false;
	lock.unlock();
	if (interrupt_thread.joinable()) {
        interrupt_thread.join();
    }
}

void ExternalInterrupt::handle_interrupts()
{
	while(1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::unique_lock<std::mutex> lock(mutex);

		if(!is_running) break;

		for(auto &it : instances)
		{
			if(it.second.is_on)
			{
				if(*(it.second.trigger_signal))
				{
					interrupts_arr.push_back(&(it.second));
				}
			}
		}

		lock.unlock();
		
		if(!interrupts_arr.size()) continue;

		for(auto it : interrupts_arr)
		{
			it->action();
			lock.lock();
			*(it->trigger_signal) = false;
			lock.unlock();
		}
		interrupts_arr.clear();
	}
}
