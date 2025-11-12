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
#define TOTAL_PIN_NUMBER 110
enum class GPIOPin : uint16_t {
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

enum class GPIOPort : uint32_t {
    PORT_A = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x0000UL,
    PORT_B = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x0400UL,
    PORT_C = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x0800UL,
    PORT_D = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x0C00UL,
    PORT_E = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x1000UL,
    PORT_F = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x1400UL,
    PORT_G = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x1800UL,
    PORT_H = DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 + 0x1C00UL,
};

enum class AlternativeFunction : uint8_t {
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
    NO_AF = 0xFF,
};

enum class OperationMode {
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

enum class PinState : uint8_t { OFF, ON };

enum class TRIGGER : uint8_t {
    RISING_EDGE = 1,
    FAILING_EDGE = 0,
    BOTH_EDGES = 2
};

using enum GPIOPort;
using enum GPIOPin;
using enum AlternativeFunction;
using enum OperationMode;
struct PinConfig;
class Pin {
   private:
    mutable GPIO_InitTypeDef GPIO_InitStruct{
        // Configuration for OperationMode::Not used
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_PULLDOWN,
    };

   public:
    GPIOPort port;
    GPIOPin gpio_pin;
    AlternativeFunction alternative_function;

    // const map<GPIO_TypeDef*, const string> port_to_string;
    // const map<GPIOPin, const string> gpio_pin_to_string;
    consteval Pin() {}
    consteval Pin(GPIOPort port, GPIOPin pin)
        : port(port),
          gpio_pin(pin),
          alternative_function(AlternativeFunction::NO_AF) {}
    consteval Pin(GPIOPort port, GPIOPin pin,
                  AlternativeFunction alternative_function)
        : port(port),
          gpio_pin(pin),
          alternative_function(alternative_function) {}

    const string to_string() const;
    // template <OperationMode m>
    // constexpr void inscribe() const {
    //     static_assert(!(m == OperationMode::NOT_USED),
    //                 "Pin is already registered, cannot register twice");
    //     if(m == OperationMode::ALTERNATIVE && alternative_function ==
    //     AlternativeFunction::NO_AF){
    //             throw "You can't use that pin for Alternative";
    //     }
    //     // static_assert(!(m == ALTERNATIVE &&
    //     //                 alternative_function ==
    //     AlternativeFunction::NO_AF),
    //     //             "You can't use that pin for Alternative");

    //     GPIO_InitStruct.Pin = static_cast<uint32_t>(gpio_pin);
    //     switch (mode) {
    //         case OperationMode::OUTPUT:
    //             GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    //             GPIO_InitStruct.Pull = GPIO_NOPULL;
    //             GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    //             break;

    //         case OperationMode::INPUT:
    //             GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    //             GPIO_InitStruct.Pull = GPIO_NOPULL;
    //             break;
    //         case OperationMode::ANALOG:
    //             GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    //             GPIO_InitStruct.Pull = GPIO_NOPULL;
    //             break;
    //         case OperationMode::EXTERNAL_INTERRUPT_RISING:
    //             GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    //             GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    //             break;
    //         case OperationMode::EXTERNAL_INTERRUPT_FALLING:
    //             GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    //             GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    //             break;
    //         case OperationMode::EXTERNAL_INTERRUPT_RISING_FALLING:
    //             GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    //             GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    //             break;
    //         case OperationMode::TIMER_ALTERNATE_FUNCTION:
    //             GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    //             GPIO_InitStruct.Pull = GPIO_NOPULL;
    //             GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    //             GPIO_InitStruct.Alternate =
    //             static_cast<uint32_t>(alternative_function); break;

    //         default:
    //             break;
    //     }
    // }
    void static start(const std::array<PinConfig,TOTAL_PIN_NUMBER> &pinConfigArray);

    bool constexpr operator==(const Pin& other) const {
        return (gpio_pin == other.gpio_pin && port == other.port);
    }

    bool constexpr operator<(const Pin& other) const {
        if (port == other.port) return gpio_pin < other.gpio_pin;
        return port < other.port;
    }
};

// template <const Pin& pin, OperationMode m>
// consteval void pinInscribe() {
//     static_assert(mode<pin> == OperationMode::NOT_USED);
//     static_assert(!(m == ALTERNATIVE &&
//                     pin.alternative_function == AlternativeFunction::NO_AF),
//                   "You can't use that pin for Alternative");

//     mode<pin> = m;

//     pin.GPIO_InitStruct.Pin = static_cast<uint32_t>(pin.gpio_pin);
//     switch (m) {
//         case OperationMode::OUTPUT:
//             pin.GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//             pin.GPIO_InitStruct.Pull = GPIO_NOPULL;
//             pin.GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//             break;

//         case OperationMode::INPUT:
//             pin.GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//             pin.GPIO_InitStruct.Pull = GPIO_NOPULL;
//             break;
//         case OperationMode::ANALOG:
//             pin.GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//             pin.GPIO_InitStruct.Pull = GPIO_NOPULL;
//             break;
//         case OperationMode::EXTERNAL_INTERRUPT_RISING:
//             pin.GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
//             pin.GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//             break;
//         case OperationMode::EXTERNAL_INTERRUPT_FALLING:
//             pin.GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
//             pin.GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//             break;
//         case OperationMode::EXTERNAL_INTERRUPT_RISING_FALLING:
//             pin.GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
//             pin.GPIO_InitStruct.Pull = GPIO_PULLDOWN;
//             break;
//         case OperationMode::TIMER_ALTERNATE_FUNCTION:
//             pin.GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//             pin.GPIO_InitStruct.Pull = GPIO_NOPULL;
//             pin.GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//             pin.GPIO_InitStruct.Alternate =
//                 static_cast<uint32_t>(pin.alternative_function);
//             break;

//         default:
//             break;
//     }
// }

inline GPIO_TypeDef* getGPIO(GPIOPort port) {
    switch (port) {
        case GPIOPort::PORT_A:
            return GPIOA;
        case GPIOPort::PORT_B:
            return GPIOB;
        case GPIOPort::PORT_C:
            return GPIOC;
        case GPIOPort::PORT_D:
            return GPIOD;
        case GPIOPort::PORT_E:
            return GPIOE;
        case GPIOPort::PORT_F:
            return GPIOF;
        case GPIOPort::PORT_G:
            return GPIOG;
        case GPIOPort::PORT_H:
            return GPIOH;
        default:
            return nullptr;
    }
}
namespace std {
    template <>
    struct hash<Pin> {
        std::size_t operator()(const Pin& k) const {
            using std::hash;
            using std::size_t;
            using std::string;

            return ((hash<uint16_t>()(static_cast<uint16_t>(k.gpio_pin)) ^
                    (hash<uint32_t>()((uint32_t)(k.port)) << 1)) >>
                    1);
        }
    };
}  

inline constexpr Pin PA0(PORT_A, PIN_0);
inline constexpr Pin PA1(PORT_A, PIN_1);
inline constexpr Pin PA2(PORT_A, PIN_2, AF11);
inline constexpr Pin PA3(PORT_A, PIN_3);
inline constexpr Pin PA4(PORT_A, PIN_4);
inline constexpr Pin PA5(PORT_A, PIN_5);
inline constexpr Pin PA6(PORT_A, PIN_6);
inline constexpr Pin PA7(PORT_A, PIN_7);
inline constexpr Pin PA8(PORT_A, PIN_8);
inline constexpr Pin PA9(PORT_A, PIN_9, AF7);
inline constexpr Pin PA10(PORT_A, PIN_10, AF7);
inline constexpr Pin PA11(PORT_A, PIN_11, AF9);
inline constexpr Pin PA12(PORT_A, PIN_12, AF9);
inline constexpr Pin PA13(PORT_A, PIN_13);
inline constexpr Pin PA14(PORT_A, PIN_14);
inline constexpr Pin PA15(PORT_A, PIN_15);
inline constexpr Pin PB0(PORT_B, PIN_0);
inline constexpr Pin PB1(PORT_B, PIN_1);
inline constexpr Pin PB2(PORT_B, PIN_2);
inline constexpr Pin PB3(PORT_B, PIN_3);
inline constexpr Pin PB4(PORT_B, PIN_4, AF2);
inline constexpr Pin PB5(PORT_B, PIN_5, AF2);
inline constexpr Pin PB6(PORT_B, PIN_6, AF1);
inline constexpr Pin PB7(PORT_B, PIN_7, AF1);
inline constexpr Pin PB8(PORT_B, PIN_8, AF1);
inline constexpr Pin PB9(PORT_B, PIN_9, AF1);
inline constexpr Pin PB10(PORT_B, PIN_10);
inline constexpr Pin PB11(PORT_B, PIN_11);
inline constexpr Pin PB12(PORT_B, PIN_12);
inline constexpr Pin PB13(PORT_B, PIN_13, AF11);
inline constexpr Pin PB14(PORT_B, PIN_14, AF2);
inline constexpr Pin PB15(PORT_B, PIN_15, AF2);
inline constexpr Pin PC0(PORT_C, PIN_0);
inline constexpr Pin PC1(PORT_C, PIN_1, AF11);
inline constexpr Pin PC2(PORT_C, PIN_2);
inline constexpr Pin PC3(PORT_C, PIN_3);
inline constexpr Pin PC4(PORT_C, PIN_4, AF11);
inline constexpr Pin PC5(PORT_C, PIN_5, AF11);
inline constexpr Pin PC6(PORT_C, PIN_6, AF3);
inline constexpr Pin PC7(PORT_C, PIN_7, AF3);
inline constexpr Pin PC8(PORT_C, PIN_8, AF2);
inline constexpr Pin PC9(PORT_C, PIN_9, AF2);
inline constexpr Pin PC10(PORT_C, PIN_10);
inline constexpr Pin PC11(PORT_C, PIN_11);
inline constexpr Pin PC12(PORT_C, PIN_12);
inline constexpr Pin PC13(PORT_C, PIN_13);
inline constexpr Pin PC14(PORT_C, PIN_14);
inline constexpr Pin PC15(PORT_C, PIN_15);
inline constexpr Pin PD0(PORT_D, PIN_0);
inline constexpr Pin PD1(PORT_D, PIN_1);
inline constexpr Pin PD2(PORT_D, PIN_2);
inline constexpr Pin PD3(PORT_D, PIN_3);
inline constexpr Pin PD4(PORT_D, PIN_4);
inline constexpr Pin PD5(PORT_D, PIN_5, AF7);
inline constexpr Pin PD6(PORT_D, PIN_6, AF7);
inline constexpr Pin PD7(PORT_D, PIN_7);
inline constexpr Pin PD8(PORT_D, PIN_8);
inline constexpr Pin PD9(PORT_D, PIN_9);
inline constexpr Pin PD10(PORT_D, PIN_10);
inline constexpr Pin PD11(PORT_D, PIN_11);
inline constexpr Pin PD12(PORT_D, PIN_12, AF2);
inline constexpr Pin PD13(PORT_D, PIN_13, AF2);
inline constexpr Pin PD14(PORT_D, PIN_14, AF2);
inline constexpr Pin PD15(PORT_D, PIN_15, AF2);
inline constexpr Pin PE0(PORT_E, PIN_0);
inline constexpr Pin PE1(PORT_E, PIN_1);
inline constexpr Pin PE2(PORT_E, PIN_2);
inline constexpr Pin PE3(PORT_E, PIN_3);
inline constexpr Pin PE4(PORT_E, PIN_4, AF4);
inline constexpr Pin PE5(PORT_E, PIN_5, AF4);
inline constexpr Pin PE6(PORT_E, PIN_6, AF4);
inline constexpr Pin PE7(PORT_E, PIN_7);
inline constexpr Pin PE8(PORT_E, PIN_8, AF1);
inline constexpr Pin PE9(PORT_E, PIN_9, AF1);
inline constexpr Pin PE10(PORT_E, PIN_10, AF1);
inline constexpr Pin PE11(PORT_E, PIN_11, AF1);
inline constexpr Pin PE12(PORT_E, PIN_12, AF1);
inline constexpr Pin PE13(PORT_E, PIN_13, AF1);
inline constexpr Pin PE14(PORT_E, PIN_14, AF1);
inline constexpr Pin PE15(PORT_E, PIN_15);
inline constexpr Pin PF0(PORT_F, PIN_0, AF13);
inline constexpr Pin PF1(PORT_F, PIN_1, AF13);
inline constexpr Pin PF2(PORT_F, PIN_2, AF13);
inline constexpr Pin PF3(PORT_F, PIN_3, AF13);
inline constexpr Pin PF4(PORT_F, PIN_4);
inline constexpr Pin PF5(PORT_F, PIN_5);
inline constexpr Pin PF6(PORT_F, PIN_6);
inline constexpr Pin PF7(PORT_F, PIN_7);
inline constexpr Pin PF8(PORT_F, PIN_8);
inline constexpr Pin PF9(PORT_F, PIN_9);
inline constexpr Pin PF10(PORT_F, PIN_10);
inline constexpr Pin PF11(PORT_F, PIN_11);
inline constexpr Pin PF12(PORT_F, PIN_12);
inline constexpr Pin PF13(PORT_F, PIN_13);
inline constexpr Pin PF14(PORT_F, PIN_14);
inline constexpr Pin PF15(PORT_F, PIN_15);
inline constexpr Pin PG0(PORT_G, PIN_0);
inline constexpr Pin PG1(PORT_G, PIN_1);
inline constexpr Pin PG2(PORT_G, PIN_2);
inline constexpr Pin PG3(PORT_G, PIN_3);
inline constexpr Pin PG4(PORT_G, PIN_4);
inline constexpr Pin PG5(PORT_G, PIN_5);
inline constexpr Pin PG6(PORT_G, PIN_6);
inline constexpr Pin PG7(PORT_G, PIN_7);
inline constexpr Pin PG8(PORT_G, PIN_8);
inline constexpr Pin PG9(PORT_G, PIN_9, AF2);
inline constexpr Pin PG10(PORT_G, PIN_10, AF2);
inline constexpr Pin PG11(PORT_G, PIN_11, AF11);
inline constexpr Pin PG12(PORT_G, PIN_12);
inline constexpr Pin PG13(PORT_G, PIN_13, AF11);
inline constexpr Pin PG14(PORT_G, PIN_14);
inline constexpr Pin PG15(PORT_G, PIN_15);
inline constexpr Pin PH0(PORT_H, PIN_0);
inline constexpr Pin PH1(PORT_H, PIN_1);

struct PinConfig{
    const Pin* pin;
    GPIO_InitTypeDef init;
    OperationMode mode;
    bool constexpr operator==(const PinConfig& other) const {
        return pin == other.pin;
    }

    bool constexpr operator<(const PinConfig& other) const {
        return pin < other.pin;
    }
    consteval PinConfig(const Pin* pin,GPIO_InitTypeDef init,OperationMode mode):
        pin(pin),
        init(init),
        mode(mode)
    {}
};
template <const Pin& p>
struct pin_token {};

namespace reg {
template <const Pin& p>
constexpr bool has_mode() {
    return requires { mode_of(pin_token<p>{}); };
}

template <const Pin& p>
constexpr OperationMode get_mode() {
    if constexpr (has_mode<p>())
        return mode_of(pin_token<p>{});
    else
        return OperationMode::NOT_USED;
}

}  // namespace reg



inline constexpr std::array<const Pin*, TOTAL_PIN_NUMBER> pinArray = {
    &PA0,  &PA1,  &PA10, &PA11, &PA12, &PA9,  &PB0,  &PB1,  &PB10, &PB11, &PB12,
    &PB13, &PB14, &PB15, &PB2,  &PB4,  &PB5,  &PB6,  &PB7,  &PB8,  &PB9,  &PC0,
    &PC1,  &PC10, &PC11, &PC12, &PC13, &PC14, &PC15, &PC2,  &PC3,  &PC4,  &PC5,
    &PC6,  &PC7,  &PC8,  &PC9,  &PD0,  &PD1,  &PD10, &PD11, &PD12, &PD13, &PD14,
    &PD15, &PD2,  &PD3,  &PD4,  &PD5,  &PD6,  &PD7,  &PD8,  &PD9,  &PE0,  &PE1,
    &PE10, &PE11, &PE12, &PE13, &PE14, &PE15, &PE2,  &PE3,  &PE4,  &PE5,  &PE6,
    &PE7,  &PE8,  &PE9,  &PF0,  &PF1,  &PF10, &PF11, &PF12, &PF13, &PF14, &PF15,
    &PF2,  &PF3,  &PF4,  &PF5,  &PF6,  &PF7,  &PF8,  &PF9,  &PG0,  &PG1,  &PG10,
    &PG11, &PG12, &PG13, &PG14, &PG15, &PG2,  &PG3,  &PG4,  &PG5,  &PG6,  &PG7,
    &PG8,  &PG9,  &PH0,  &PH1,  &PA2,  &PA3,  &PA4,  &PA5,  &PA6,  &PA7,  &PA8};

template<const Pin& pin>
static consteval auto make_pin_config(){
            GPIO_InitTypeDef init{};
            constexpr OperationMode mode = reg::get_mode<pin>();
            if (mode == OperationMode::ALTERNATIVE && pin.alternative_function == AlternativeFunction::NO_AF){
                throw "You can't use mode Alternative with a pin without Alternative Function";
            }
            init.Pin = static_cast<uint32_t>(pin.gpio_pin);
            switch (mode) {
                case OperationMode::NOT_USED:
                    init.Mode = GPIO_MODE_INPUT;
                    init.Pull = GPIO_PULLDOWN;
                    break;
                case OperationMode::OUTPUT:
                    init.Mode = GPIO_MODE_OUTPUT_PP;
                    init.Pull = GPIO_NOPULL;
                    init.Speed = GPIO_SPEED_FREQ_LOW;
                    break;

                case OperationMode::INPUT:
                    init.Mode = GPIO_MODE_INPUT;
                    init.Pull = GPIO_NOPULL;
                    break;
                case OperationMode::ANALOG:
                    init.Mode = GPIO_MODE_ANALOG;
                    init.Pull = GPIO_NOPULL;
                    break;
                case OperationMode::EXTERNAL_INTERRUPT_RISING:
                    init.Mode = GPIO_MODE_IT_RISING;
                    init.Pull = GPIO_PULLDOWN;
                    break;
                case OperationMode::EXTERNAL_INTERRUPT_FALLING:
                    init.Mode = GPIO_MODE_IT_FALLING;
                    init.Pull = GPIO_PULLDOWN;
                    break;
                case OperationMode::EXTERNAL_INTERRUPT_RISING_FALLING:
                    init.Mode = GPIO_MODE_IT_RISING_FALLING;
                    init.Pull = GPIO_PULLDOWN;
                    break;
                case OperationMode::TIMER_ALTERNATE_FUNCTION:
                    init.Mode = GPIO_MODE_AF_PP;
                    init.Pull = GPIO_NOPULL;
                    init.Speed = GPIO_SPEED_FREQ_LOW;
                    init.Alternate = static_cast<uint32_t>(pin.alternative_function);
                    break;
                default:
                    break;
            }
            return PinConfig{&pin,init,mode};
}

template<std::size_t... I>
static consteval auto make_pinConfigArray_impl(std::index_sequence<I...>) {
    return std::array{
        make_pin_config<*pinArray[I]>()...
    };
}

consteval std::array<PinConfig,TOTAL_PIN_NUMBER> make_pinConfigArray() {
    return make_pinConfigArray_impl(std::make_index_sequence<TOTAL_PIN_NUMBER>{});
}
