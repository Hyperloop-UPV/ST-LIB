#pragma once

#include "hal_wrapper.h"
#include <array>
#include <span>
#include <tuple>

using std::array;
using std::size_t;
using std::span;
using std::tuple;

namespace ST_LIB {
extern void compile_error(const char* msg);

struct GPIODomain {
    enum class OperationMode : uint8_t {
        INPUT,               // GPIO_MODE_INPUT
        OUTPUT_PUSHPULL,     // GPIO_MODE_OUTPUT_PP
        OUTPUT_OPENDRAIN,    // GPIO_MODE_OUTPUT_OD
        ANALOG,              // GPIO_MODE_ANALOG
        ALT_PP,              // GPIO_MODE_AF_PP
        ALT_OD,              // GPIO_MODE_AF_OD
        EXTI_RISING,         // GPIO_MODE_IT_RISING
        EXTI_FALLING,        // GPIO_MODE_IT_FALLING
        EXTI_RISING_FALLING, // GPIO_MODE_IT_RISING_FALLING
    };
    static constexpr uint32_t to_hal_mode(OperationMode m) {
        switch (m) {
        case OperationMode::INPUT:
            return GPIO_MODE_INPUT;
        case OperationMode::OUTPUT_PUSHPULL:
            return GPIO_MODE_OUTPUT_PP;
        case OperationMode::OUTPUT_OPENDRAIN:
            return GPIO_MODE_OUTPUT_OD;
        case OperationMode::ANALOG:
            return GPIO_MODE_ANALOG;
        case OperationMode::ALT_PP:
            return GPIO_MODE_AF_PP;
        case OperationMode::ALT_OD:
            return GPIO_MODE_AF_OD;
        case OperationMode::EXTI_RISING:
            return GPIO_MODE_IT_RISING;
        case OperationMode::EXTI_FALLING:
            return GPIO_MODE_IT_FALLING;
        case OperationMode::EXTI_RISING_FALLING:
            return GPIO_MODE_IT_RISING_FALLING;
        }
    }
    enum class Pull : uint8_t { None, Up, Down };
    static constexpr uint32_t to_hal_pull(Pull p) {
        switch (p) {
        case Pull::None:
            return GPIO_NOPULL;
        case Pull::Up:
            return GPIO_PULLUP;
        case Pull::Down:
            return GPIO_PULLDOWN;
        }
    }
    enum class Speed : uint8_t { Low, Medium, High, VeryHigh };
    static constexpr uint32_t to_hal_speed(Speed s) {
        switch (s) {
        case Speed::Low:
            return GPIO_SPEED_FREQ_LOW;
        case Speed::Medium:
            return GPIO_SPEED_FREQ_MEDIUM;
        case Speed::High:
            return GPIO_SPEED_FREQ_HIGH;
        case Speed::VeryHigh:
            return GPIO_SPEED_FREQ_VERY_HIGH;
        }
    }
    // Note: AF mapping is inverted: AF0 -> 15, AF1 -> 14, ..., AF15 -> 0, it is staticly casted and
    // inverted later
    enum class AlternateFunction : uint8_t {
        NO_AF = 20,
        AF0 = 15,
        AF1 = 14,
        AF2 = 13,
        AF3 = 12,
        AF4 = 11,
        AF5 = 10,
        AF6 = 9,
        AF7 = 8,
        AF8 = 7,
        AF9 = 6,
        AF10 = 5,
        AF11 = 4,
        AF12 = 3,
        AF13 = 2,
        AF14 = 1,
        AF15 = 0
    };
    enum class Port : uint8_t { A, B, C, D, E, F, G, H };
    static inline GPIO_TypeDef* port_to_reg(Port p) {
        switch (p) {
        case Port::A:
            return GPIOA;
        case Port::B:
            return GPIOB;
        case Port::C:
            return GPIOC;
        case Port::D:
            return GPIOD;
        case Port::E:
            return GPIOE;
        case Port::F:
            return GPIOF;
        case Port::G:
            return GPIOG;
        case Port::H:
            return GPIOH;
        default:
            return nullptr;
        }
    }
    static inline void enable_gpio_clock(Port port) {
        switch (port) {
        case Port::A:
            __HAL_RCC_GPIOA_CLK_ENABLE();
            break;

        case Port::B:
            __HAL_RCC_GPIOB_CLK_ENABLE();
            break;

        case Port::C:
            __HAL_RCC_GPIOC_CLK_ENABLE();
            break;

        case Port::D:
            __HAL_RCC_GPIOD_CLK_ENABLE();
            break;

        case Port::E:
            __HAL_RCC_GPIOE_CLK_ENABLE();
            break;

        case Port::F:
            __HAL_RCC_GPIOF_CLK_ENABLE();
            break;

        case Port::G:
            __HAL_RCC_GPIOG_CLK_ENABLE();
            break;

        case Port::H:
            __HAL_RCC_GPIOH_CLK_ENABLE();
            break;
        }
    }

    struct Pin {
        GPIODomain::Port port;
        uint32_t pin;
        uint16_t afs;

        inline constexpr bool valid_af(const AlternateFunction af) const {
            if (af == AlternateFunction::NO_AF)
                return true;
            return ((1 << static_cast<uint8_t>(af)) & afs) != 0;
        }

        constexpr bool operator==(const Pin& other) const {
            return (port == other.port) && (pin == other.pin);
        }

        constexpr bool operator!=(const Pin& other) const { return !(*this == other); }
    };

    struct Entry {
        Port port;
        uint32_t pin;
        OperationMode mode;
        Pull pull;
        Speed speed;
        AlternateFunction af;
    };

    struct GPIO {
        using domain = GPIODomain;

        Entry e;

        consteval GPIO(
            const Pin& pin,
            OperationMode mode,
            Pull pull,
            Speed speed,
            AlternateFunction af = AlternateFunction::NO_AF
        )
            : e{pin.port, pin.pin, mode, pull, speed, af} {
            if (!pin.valid_af(af)) {
                compile_error("Alternate function not valid for this pin");
            }

            if ((mode == OperationMode::ALT_PP || mode == OperationMode::ALT_OD) &&
                af == AlternateFunction::NO_AF) {
                compile_error("Alternate function must be specified for alternate modes");
            }
        }

        template <class Ctx> consteval std::size_t inscribe(Ctx& ctx) const {
            return ctx.template add<GPIODomain>(e, this);
        }
    };

    static constexpr std::size_t max_instances{110};

    struct Config {
        std::tuple<Port, GPIO_InitTypeDef> init_data{};
    };

    template <size_t N> static consteval array<Config, N> build(span<const Entry> pins) {
        array<Config, N> cfgs{};
        for (std::size_t i = 0; i < N; ++i) {
            const auto& e = pins[i];

            for (std::size_t j = 0; j < i; ++j) {
                const auto& prev = pins[j];
                if (prev.pin == e.pin && prev.port == e.port) {
                    compile_error("GPIO already inscribed");
                }
            }

            GPIO_InitTypeDef GPIO_InitStruct{};
            GPIO_InitStruct.Pin = e.pin;
            GPIO_InitStruct.Mode = to_hal_mode(e.mode);
            GPIO_InitStruct.Pull = to_hal_pull(e.pull);
            GPIO_InitStruct.Speed = to_hal_speed(e.speed);
            if (e.mode == OperationMode::ALT_PP || e.mode == OperationMode::ALT_OD) {
                GPIO_InitStruct.Alternate =
                    15 - static_cast<uint32_t>(e.af); // AF mapping inversion
            }

            cfgs[i].init_data = std::make_tuple(e.port, GPIO_InitStruct);
        }

        return cfgs;
    }

    // Runtime object
    struct Instance {
    private:
        GPIO_TypeDef* port;
        uint32_t pin;

    public:
        constexpr Instance() : port{nullptr}, pin{0} {}
        Instance(GPIO_TypeDef* p, uint32_t pin) : port{p}, pin{static_cast<uint16_t>(pin)} {}

        void turn_on() { HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET); }

        void turn_off() { HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET); }

        void toggle() { HAL_GPIO_TogglePin(port, pin); }

        GPIO_PinState read() { return HAL_GPIO_ReadPin(port, pin); }
    };

    template <std::size_t N> struct Init {
        static inline std::array<Instance, N> instances{};

        static void init(std::span<const Config, N> cfgs) {
            for (std::size_t i = 0; i < N; ++i) {
                const auto& e = cfgs[i];
                auto [port, gpio_init] = e.init_data;

                enable_gpio_clock(port);
                HAL_GPIO_Init(port_to_reg(port), &gpio_init);

                instances[i] = Instance{port_to_reg(port), gpio_init.Pin};
            }
        }
    };
};
} // namespace ST_LIB
