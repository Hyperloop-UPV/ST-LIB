/*
 * Pin.hpp
 *
 *  Created on: 19 oct. 2022
 *      Author: stefan
 */
#pragma once
#include "C++Utilities/CppUtils.hpp"

#define PERIPHERAL_BASE 0x40000000UL
#define DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 PERIPHERAL_BASE + 0x18020000UL

enum GPIOPin : uint16_t {
    PIN_0 = 0x0001,
    PIN_1 = 0x0002,
    PIN_2 = 0x0004,
    PIN_3 = 0x0008,
    PIN_4 = 0x0010,
    PIN_5 = 0x0020,
    PIN_6 = 0x0040,
    PIN_7 = 0x0080,
    PIN_8 = 0x0100,
    PIN_9 = 0x0200,
    PIN_10 = 0x0400,
    PIN_11 = 0x0800,
    PIN_12 = 0x1000,
    PIN_13 = 0x2000,
    PIN_14 = 0x4000,
    PIN_15 = 0x8000,
    PIN_ALL = 0xFFFF
};

enum GPIOPort : uint32_t {
    PORT_A = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x0000UL,
    PORT_B = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x0400UL,
    PORT_C = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x0800UL,
    PORT_D = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x0C00UL,
    PORT_E = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x1000UL,
    PORT_F = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x1400UL,
    PORT_G = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x1800UL,
    PORT_H = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x1C00UL,
};

enum AlternativeFunction : uint8_t {
    AF0 = 0x00,
    AF1 = 0x01,
    AF2 = 0x02,
    AF3 = 0x03,
    AF4 = 0x04,
    AF5 = 0x05,
    AF6 = 0x06,
    AF7 = 0x07,
    AF8 = 0x08,
    AF9 = 0x09,
    AF10 = 0x0A,
    AF11 = 0x0B,
    AF12 = 0x0C,
    AF13 = 0x0D,
    AF14 = 0x0E,
    AF15 = 0x0F,
};

enum OperationMode {
    NOT_USED,
    INPUT,
    OUTPUT,
    ANALOG,
    EXTERNAL_INTERRUPT_RISING,
    EXTERNAL_INTERRUPT_FALLING,
    TIMER_ALTERNATE_FUNCTION,
    ALTERNATIVE,
};

enum PinState { OFF, ON };
enum TRIGGER{
    RISING_EDGE = 1,
    FAILING_EDGE = 0,
    BOTH_EDGES = 2
};
enum class PinType {
    NOT_USED,
    DigitalOutput,
    DigitalInput,
    PWM,
    DualPWM,
    ADC,
    ENCODER,
    EXTIPin,  // Usando temporalmente este nombre por que hay colisión entre
              // nombres
    // TODO: Add more types
};
struct DigitalOutput_MockPin{
    bool state;
} ;
struct DigitalInput_MockPin {
    PinState curr_state;
} ;
struct PWM_MockPin {
    float duty_cycle;
    uint32_t frequency;
    bool is_on;
    std::chrono::nanoseconds dead_time_ns;
} ;
struct DualPWM_MockPin{
    float duty_cycle;
    uint32_t frequency;
    bool is_on;
    std::chrono::nanoseconds dead_time_ns;
} ;
struct ADC_MockPin{
    uint16_t value;
    bool is_on;
} ;
struct EXTIPin_MockPin{
    uint32_t priority;
    bool is_on;
    bool trigger_signal;
    TRIGGER trigger_mode;
} ;
struct Encoder_MockPin{
    uint32_t count_value;
    bool direction;
    bool is_on;
} ;
struct EmulatedPin {
    PinType type =  PinType::NOT_USED;  // Always check type before using the union
    union PinDataU {
        DigitalOutput_MockPin   digital_output;
        DigitalInput_MockPin    digital_input;
        PWM_MockPin             pwm;
        DualPWM_MockPin         dual_pwm;
        ADC_MockPin             adc;
        EXTIPin_MockPin         exti;
        Encoder_MockPin         encoder;
		// TODO Add more types
	}PinData;
};

class Pin {
   public:
    GPIOPort* port;
    GPIOPin gpio_pin;
    AlternativeFunction alternative_function;
    OperationMode mode = OperationMode::NOT_USED;
    static const vector<reference_wrapper<Pin>> pinVector;
    static const map<GPIOPort*, const string> port_to_string;
    static const map<GPIOPin, const string> gpio_pin_to_string;
    Pin();
    Pin(GPIOPort port, GPIOPin pin);
    Pin(GPIOPort port, GPIOPin pin, AlternativeFunction alternative_function);
    const string to_string() const;
    static void inscribe(Pin& pin, OperationMode mode);
    static void start();

    bool operator==(const Pin& other) const {
        return (gpio_pin == other.gpio_pin && port == other.port);
    }

    bool operator<(const Pin& other) const {
        if (port == other.port) return gpio_pin < other.gpio_pin;
        return port < other.port;
    }
};

namespace std {
template <>
struct hash<Pin> {
    std::size_t operator()(const Pin& k) const {
        using std::hash;
        using std::size_t;
        using std::string;

        return ((hash<uint16_t>()(k.gpio_pin) ^
                 (hash<uint32_t>()((uint32_t)((uintptr_t)k.port)) << 1)) >>
                1);
    }
};
}  // namespace std

extern Pin PA0;
extern Pin PA1;
extern Pin PA10;
extern Pin PA11;
extern Pin PA12;
extern Pin PA13;
extern Pin PA14;
extern Pin PA15;
extern Pin PA9;
extern Pin PB0;
extern Pin PB1;
extern Pin PB10;
extern Pin PB11;
extern Pin PB12;
extern Pin PB13;
extern Pin PB14;
extern Pin PB15;
extern Pin PB2;
extern Pin PB3;
extern Pin PB4;
extern Pin PB5;
extern Pin PB6;
extern Pin PB7;
extern Pin PB8;
extern Pin PB9;
extern Pin PC0;
extern Pin PC1;
extern Pin PC10;
extern Pin PC11;
extern Pin PC12;
extern Pin PC13;
extern Pin PC14;
extern Pin PC15;
extern Pin PC2;
extern Pin PC3;
extern Pin PC4;
extern Pin PC5;
extern Pin PC6;
extern Pin PC7;
extern Pin PC8;
extern Pin PC9;
extern Pin PD0;
extern Pin PD1;
extern Pin PD10;
extern Pin PD11;
extern Pin PD12;
extern Pin PD13;
extern Pin PD14;
extern Pin PD15;
extern Pin PD2;
extern Pin PD3;
extern Pin PD4;
extern Pin PD5;
extern Pin PD6;
extern Pin PD7;
extern Pin PD8;
extern Pin PD9;
extern Pin PE0;
extern Pin PE1;
extern Pin PE10;
extern Pin PE11;
extern Pin PE12;
extern Pin PE13;
extern Pin PE14;
extern Pin PE15;
extern Pin PE2;
extern Pin PE3;
extern Pin PE4;
extern Pin PE5;
extern Pin PE6;
extern Pin PE7;
extern Pin PE8;
extern Pin PE9;
extern Pin PF0;
extern Pin PF1;
extern Pin PF10;
extern Pin PF11;
extern Pin PF12;
extern Pin PF13;
extern Pin PF14;
extern Pin PF15;
extern Pin PF2;
extern Pin PF3;
extern Pin PF4;
extern Pin PF5;
extern Pin PF6;
extern Pin PF7;
extern Pin PF8;
extern Pin PF9;
extern Pin PG0;
extern Pin PG1;
extern Pin PG10;
extern Pin PG11;
extern Pin PG12;
extern Pin PG13;
extern Pin PG14;
extern Pin PG15;
extern Pin PG2;
extern Pin PG3;
extern Pin PG4;
extern Pin PG5;
extern Pin PG6;
extern Pin PG7;
extern Pin PG8;
extern Pin PG9;
extern Pin PH0;
extern Pin PH1;
extern Pin PA2;
extern Pin PA3;
extern Pin PA4;
extern Pin PA5;
extern Pin PA6;
extern Pin PA7;
extern Pin PA8;
