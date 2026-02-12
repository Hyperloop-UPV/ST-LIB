/*
 * EXTI.hpp
 *
 *  Created on: Nov 5, 2022
 *      Author: alejandro 
 */

#ifndef EXTI_HPP
#define EXTI_HPP
#include "HALAL/Models/GPIO.hpp"
#include "HALAL/Models/Pin.hpp"
#include <cmath>

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

namespace ST_LIB {

struct EXTIDomain {

	static constexpr uint8_t get_pin_index(uint16_t pin_mask) {
		for(uint8_t i = 0; i < 16; i++) {
			if (pin_mask & (1 << i)) return i;
		}
		return 0xFF;
	}

	enum class Trigger : uint8_t {
		RISING_EDGE = 6, // ST_LIB::GPIODomain::OperationMode::EXTI_RISING
		FALLING_EDGE = 7, // ST_LIB::GPIODomain::OperationMode::EXTI_FALLING
		BOTH_EDGES = 8 // ST_LIB::GPIODomain::OperationMode::EXTI_RISING_FALLING 
	};

  struct Entry {
    std::size_t pin_idx;
	uint16_t pin_mask;
	void (*action)();
  };

  struct Device {
    using domain = EXTIDomain;
	ST_LIB::GPIODomain::GPIO pin;
	void (*action)();

    consteval Device(ST_LIB::GPIODomain::Pin pin, Trigger trigger, void (*action)()) 
		: pin(pin, static_cast<ST_LIB::GPIODomain::OperationMode>(trigger), ST_LIB::GPIODomain::Pull::None, ST_LIB::GPIODomain::Speed::Low), action(action) {
			#ifndef HAL_EXTI_MODULE_ENABLED
			ST_LIB::compile_error("EXTI module not enabled in HAL"); 
			#endif
		}

    template <class Ctx>
    consteval std::size_t inscribe(Ctx &ctx) const {
			Entry e;
			e.pin_idx = pin.inscribe(ctx);
			e.pin_mask = pin.e.pin; 
			e.action = action;
			return ctx.template add<EXTIDomain>(e, this);
    }
  };

  static constexpr std::size_t max_instances = 16;

  struct Config {
    std::size_t pin_idx;
		uint8_t interrupt_num;
		void (*action)();
  };

  template <std::size_t N>
  static consteval std::array<Config, N> build(std::span<const Entry> entries) {
    std::array<Config, N> cfgs{};

		uint16_t used_lines_mask = 0;

    for (std::size_t i = 0; i < N; i++) {
      const Entry &e = entries[i];

      cfgs[i].pin_idx = e.pin_idx;
	  uint8_t pin_num = get_pin_index(e.pin_mask);
	  
	  if (pin_num >= 16) {
		  ST_LIB::compile_error("Invalid Pin for EXTI");
	  }

	  uint16_t line_mask = (1 << pin_num);
	  if (used_lines_mask & line_mask) {
			ST_LIB::compile_error("EXTI line already used (Hardware Conflict: Pins with same number share EXTI Line)");
	  }
	  
	  used_lines_mask |= line_mask;
	  cfgs[i].interrupt_num = pin_num;
	  cfgs[i].action = e.action;
    }

    return cfgs;
  }

  struct Instance {
		friend void ::HAL_GPIO_EXTI_Callback(uint16_t);
		template <std::size_t> friend struct Init;
    void turn_off() { is_on = false; }
		void turn_on() { is_on = true; }
		GPIO_PinState read() { 
			return gpio->read();
		}

	  private:
		bool is_on = false;
		void (*action)() = nullptr;
    GPIODomain::Instance* gpio = nullptr;
  };

	static Instance* g_instances[EXTIDomain::max_instances];

  template <std::size_t N>
  struct Init {
    static inline std::array<Instance, N> instances{};

    static void init(std::span<const Config, N> cfgs,
										std::span<GPIODomain::Instance> gpio_instances) {
      for (std::size_t i = 0; i < N; i++) {
        const auto &cfg = cfgs[i];
        auto &inst = instances[i];

				uint8_t id = cfg.interrupt_num;
				g_instances[id] = &inst;
				inst.action = cfg.action;
        inst.gpio = &gpio_instances[cfg.pin_idx];
						
				IRQn_Type irq_n;
				if (id <= 4) irq_n = (IRQn_Type)(EXTI0_IRQn + id);
				else if (id <= 9) irq_n = EXTI9_5_IRQn;
				else irq_n = EXTI15_10_IRQn;

				HAL_NVIC_SetPriority(irq_n, 0, 0);
				HAL_NVIC_EnableIRQ(irq_n);

				inst.turn_on();
      }
    }
  };
};

}

#endif // EXTI_HPP