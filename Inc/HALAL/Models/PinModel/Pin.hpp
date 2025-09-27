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
    EXTERNAL_INTERRUPT_RISING_FALLING,
    TIMER_ALTERNATE_FUNCTION,
    ALTERNATIVE,
};

enum PinState : uint8_t { OFF, ON };

enum TRIGGER : uint8_t { RISING_EDGE = 1, FAILING_EDGE = 0, BOTH_EDGES = 2 };

class Pin {
   public:
    GPIO_TypeDef* port;
    GPIOPin gpio_pin;
    AlternativeFunction alternative_function;
    OperationMode mode = OperationMode::NOT_USED;
    const map<GPIO_TypeDef*, const string> port_to_string;
    const map<GPIOPin, const string> gpio_pin_to_string;
    Pin();
    constexpr Pin(GPIOPort port, GPIOPin pin);
    Pin(GPIOPort port, GPIOPin pin, AlternativeFunction alternative_function);
    const string to_string() const;
    consteval void inscribe(OperationMode mode) {
        if (mode != OperationMode::NOT_USED) {
            static_assert("Pin is already registered, cannot register twice");
            return;
        }
        this->mode = mode;
    }
    void start();

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
                 (hash<uint32_t>()((uint32_t)(k.port)) << 1)) >>
                1);
    }
};
}  // namespace std

Pin PA0{};
Pin PA1{};
Pin PA2{};
Pin PA3{};
Pin PA4{};
Pin PA5{};
Pin PA6{};
Pin PA7{};
Pin PA8{};
Pin PA9{};
Pin PA10{};
Pin PA11{};
Pin PA12{};
Pin PA13{};
Pin PA14{};
Pin PA15{};
Pin PB0{};
Pin PB1{};
Pin PB2{};
Pin PB3{};
Pin PB4{};
Pin PB5{};
Pin PB6{};
Pin PB7{};
Pin PB8{};
Pin PB9{};
Pin PB10{};
Pin PB11{};
Pin PB12{};
Pin PB13{};
Pin PB14{};
Pin PB15{};
Pin PC0{};
Pin PC1{};
Pin PC2{};
Pin PC3{};
Pin PC4{};
Pin PC5{};
Pin PC6{};
Pin PC7{};
Pin PC8{};
Pin PC9{};
Pin PC10{};
Pin PC11{};
Pin PC12{};
Pin PC13{};
Pin PC14{};
Pin PC15{};
Pin PD0{};
Pin PD1{};
Pin PD2{};
Pin PD3{};
Pin PD4{};
Pin PD5{};
Pin PD6{};
Pin PD7{};
Pin PD8{};
Pin PD9{};
Pin PD10{};
Pin PD11{};
Pin PD12{};
Pin PD13{};
Pin PD14{};
Pin PD15{};
Pin PE0{};
Pin PE1{};
Pin PE2{};
Pin PE3{};
Pin PE4{};
Pin PE5{};
Pin PE6{};
Pin PE7{};
Pin PE8{};
Pin PE9{};
Pin PE10{};
Pin PE11{};
Pin PE12{};
Pin PE13{};
Pin PE14{};
Pin PE15{};
Pin PF0{};
Pin PF1{};
Pin PF2{};
Pin PF3{};
Pin PF4{};
Pin PF5{};
Pin PF6{};
Pin PF7{};
Pin PF8{};
Pin PF9{};
Pin PF10{};
Pin PF11{};
Pin PF12{};
Pin PF13{};
Pin PF14{};
Pin PF15{};
Pin PG0{};
Pin PG1{};
Pin PG2{};
Pin PG3{};
Pin PG4{};
Pin PG5{};
Pin PG6{};
Pin PG7{};
Pin PG8{};
Pin PG9{};
Pin PG10{};
Pin PG11{};
Pin PG12{};
Pin PG13{};
Pin PG14{};
Pin PG15{};
Pin PH0{};
Pin PH1{};

const auto pinVector = std::to_array<reference_wrapper<Pin>>(
    {PA0,  PA1,  PA10, PA11, PA12, PA9,  PB0,  PB1,  PB10, PB11, PB12,
     PB13, PB14, PB15, PB2,  PB4,  PB5,  PB6,  PB7,  PB8,  PB9,  PC0,
     PC1,  PC10, PC11, PC12, PC13, PC14, PC15, PC2,  PC3,  PC4,  PC5,
     PC6,  PC7,  PC8,  PC9,  PD0,  PD1,  PD10, PD11, PD12, PD13, PD14,
     PD15, PD2,  PD3,  PD4,  PD5,  PD6,  PD7,  PD8,  PD9,  PE0,  PE1,
     PE10, PE11, PE12, PE13, PE14, PE15, PE2,  PE3,  PE4,  PE5,  PE6,
     PE7,  PE8,  PE9,  PF0,  PF1,  PF10, PF11, PF12, PF13, PF14, PF15,
     PF2,  PF3,  PF4,  PF5,  PF6,  PF7,  PF8,  PF9,  PG0,  PG1,  PG10,
     PG11, PG12, PG13, PG14, PG15, PG2,  PG3,  PG4,  PG5,  PG6,  PG7,
     PG8,  PG9,  PH0,  PH1,  PA2,  PA3,  PA4,  PA5,  PA6,  PA7,  PA8});
