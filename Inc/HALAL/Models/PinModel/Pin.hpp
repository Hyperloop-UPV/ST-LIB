/*
 * Pin.hpp
 *
 *  Created on: 19 oct. 2022
 *      Author: stefan
 */
#pragma once
#include "C++Utilities/CppUtils.hpp"

#define PERIPHERAL_BASE 0x40000000UL
#define DOMAIN3_ADVANCED_HIGH_PERFORMANCE_BUS1 PERIPHERAL_BASE+0x18020000UL



enum GPIOPin : uint16_t{
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

enum AlternativeFunction: uint8_t {
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

enum OperationMode{
	NOT_USED,
	INPUT,
	OUTPUT,
	ANALOG,
	EXTERNAL_INTERRUPT_RISING,
	EXTERNAL_INTERRUPT_FALLING,
	TIMER_ALTERNATE_FUNCTION,
	ALTERNATIVE,
};

enum PinState{
	OFF,
	ON
};

enum TRIGGER{
    RISING_EDGE = 1,
    FAILING_EDGE = 0,
    BOTH_EDGES = 2
};


//Struct for a physic pin
class PinBase {
public:
	GPIOPort port;
	GPIOPin gpio_pin;
	std::optional<AlternativeFunction> alternative_function;
	consteval PinBase(GPIOPort port, GPIOPin pin)
        : port(port), gpio_pin(pin) {}
    consteval PinBase(GPIOPort port, GPIOPin pin, AlternativeFunction alternative_function)
        : port(port), gpio_pin(pin), alternative_function(alternative_function) {}
	// consteval string_view port_string() const;
	// consteval string_view pin_string() const;
	const string to_string() const;
	bool operator== (const PinBase &other) const {
		return (gpio_pin == other.gpio_pin && port == other.port);
	}
	bool operator< (const PinBase &other) const {
		if (port == other.port)
			return gpio_pin < other.gpio_pin;
		return port < other.port;
	}
};
namespace std {
	template <>
	struct hash<PinBase> {
		std::size_t operator()(const PinBase& k) const {
		    using std::size_t;
		    using std::hash;
		    using std::string;

		    return ((hash<uint16_t>()(k.gpio_pin) ^ (hash<uint32_t>()((uint32_t)(k.port)) << 1)) >> 1);
		}
	  };
}
// New type that contains PinBase and Mode
template<const PinBase& P, OperationMode Mode>
struct PinConfig{
	static constexpr GPIOPort port = P.port;
	static constexpr GPIOPin pin = P.gpio_pin;
	static constexpr OperationMode mode = Mode;
	static constexpr AlternativeFunction alternative_function = P.alternative_function;
};
template <typename... Ts> struct TypeList{}; // type container
//To append elements in the TypeList
template<typename,typename> struct Append;
template<typename... Ts, typename U>
struct Append<TypeList<Ts...>, U> {using type = TypeList<Ts...,U>; };

//	Contenedor que busca duplicados en tiempo de compilacion:
template<typename, typename> struct Contains;
template<typename T>
struct Contains<T, TypeList<>> : std::false_type {};
template<typename T, typename U, typename... Rest>
struct Contains<T, TypeList<U, Rest...>>
    : std::conditional_t<(T::port == U::port && T::pin == U::pin), 
                         std::true_type, 
                         Contains<T, TypeList<Rest...>>> {};


template<typename... Ts>
struct Registry {
    using Pins = TypeList<Ts...>;
    static void init() { (init_one(Ts{}), ...); }
private:
    template<typename P>
    static void init_one(const P&) { 
		GPIO_InitTypeDef GPIO_InitStruct = {0};
        auto* port = reinterpret_cast<GPIO_TypeDef*>(P::port);
        GPIO_InitStruct.Pin = P::pin;
		switch(P::mode){
			case OperationMode::NOT_USED:
				GPIO_InitStruct.Mode =  GPIO_MODE_INPUT;
				GPIO_InitStruct.Pull = GPIO_PULLDOWN;
				HAL_GPIO_Init(port, &GPIO_InitStruct);
				break;

			case OperationMode::OUTPUT:
				GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
				HAL_GPIO_Init(port, &GPIO_InitStruct);
				break;

			case OperationMode::INPUT:
				GPIO_InitStruct.Mode =  GPIO_MODE_INPUT;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				HAL_GPIO_Init(port, &GPIO_InitStruct);
				break;

			case OperationMode::ANALOG:
				GPIO_InitStruct.Mode =  GPIO_MODE_ANALOG;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				HAL_GPIO_Init(port, &GPIO_InitStruct);
				break;
			case OperationMode::EXTERNAL_INTERRUPT_RISING:
				GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
				GPIO_InitStruct.Pull = GPIO_PULLDOWN;
				HAL_GPIO_Init(port, &GPIO_InitStruct);
				break;
			case OperationMode::EXTERNAL_INTERRUPT_FALLING:
				GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
				GPIO_InitStruct.Pull = GPIO_PULLDOWN;
				HAL_GPIO_Init(port, &GPIO_InitStruct);
				break;
			case OperationMode::TIMER_ALTERNATE_FUNCTION:
				GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
				GPIO_InitStruct.Pull = GPIO_NOPULL;
				GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
				GPIO_InitStruct.Alternate = P::alternative_function;
				HAL_GPIO_Init(port, &GPIO_InitStruct);
				break;

			default:
				break;
		}
    }
};

template<typename List>
struct State { using type = List; };
inline State<TypeList<>> GlobalState{};
class Pin {
public:
    template<const PinBase& P, OperationMode Mode>
    static consteval void inscribe() {
        using NewPin  = PinConfig<P, Mode>;
        using OldList = typename decltype(GlobalState)::type;
        static_assert(!Contains<NewPin, OldList>::value, "Duplicate pin inscribed!");
        using NewList = typename Append<OldList, NewPin>::type;
        GlobalState = State<NewList>{};
    }

    static void start() {
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		__HAL_RCC_GPIOE_CLK_ENABLE();
		__HAL_RCC_GPIOF_CLK_ENABLE();
		__HAL_RCC_GPIOG_CLK_ENABLE();
        using Pins = typename decltype(GlobalState)::type;
        Registry<Pins>{}.init();
    }
};

//declare all the pins inline constexpr, so is accesible in any other file and you is inizialite in compilation time
inline constexpr PinBase PA0(PORT_A, PIN_0);
inline constexpr PinBase PA1(PORT_A, PIN_1);
inline constexpr PinBase PA2(PORT_A, PIN_2, AF11);
inline constexpr PinBase PA3(PORT_A, PIN_3);
inline constexpr PinBase PA4(PORT_A, PIN_4);
inline constexpr PinBase PA5(PORT_A, PIN_5);
inline constexpr PinBase PA6(PORT_A, PIN_6);
inline constexpr PinBase PA7(PORT_A, PIN_7);
inline constexpr PinBase PA8(PORT_A, PIN_8);
inline constexpr PinBase PA9(PORT_A, PIN_9, AF7);
inline constexpr PinBase PA10(PORT_A, PIN_10, AF7);
inline constexpr PinBase PA11(PORT_A, PIN_11, AF9);
inline constexpr PinBase PA12(PORT_A, PIN_12, AF9);
inline constexpr PinBase PA13(PORT_A, PIN_13);
inline constexpr PinBase PA14(PORT_A, PIN_14);
inline constexpr PinBase PA15(PORT_A, PIN_15);
inline constexpr PinBase PB0(PORT_B, PIN_0);
inline constexpr PinBase PB1(PORT_B, PIN_1);
inline constexpr PinBase PB2(PORT_B, PIN_2);
inline constexpr PinBase PB3(PORT_B, PIN_3);
inline constexpr PinBase PB4(PORT_B, PIN_4, AF2);
inline constexpr PinBase PB5(PORT_B, PIN_5, AF2);
inline constexpr PinBase PB6(PORT_B, PIN_6, AF1);
inline constexpr PinBase PB7(PORT_B, PIN_7, AF1);
inline constexpr PinBase PB8(PORT_B, PIN_8, AF1);
inline constexpr PinBase PB9(PORT_B, PIN_9, AF1);
inline constexpr PinBase PB10(PORT_B, PIN_10);
inline constexpr PinBase PB11(PORT_B, PIN_11);
inline constexpr PinBase PB12(PORT_B, PIN_12);
inline constexpr PinBase PB13(PORT_B, PIN_13, AF11);
inline constexpr PinBase PB14(PORT_B, PIN_14, AF2);
inline constexpr PinBase PB15(PORT_B, PIN_15, AF2);
inline constexpr PinBase PC0(PORT_C, PIN_0);
inline constexpr PinBase PC1(PORT_C, PIN_1, AF11);
inline constexpr PinBase PC2(PORT_C, PIN_2);
inline constexpr PinBase PC3(PORT_C, PIN_3);
inline constexpr PinBase PC4(PORT_C, PIN_4, AF11);
inline constexpr PinBase PC5(PORT_C, PIN_5, AF11);
inline constexpr PinBase PC6(PORT_C, PIN_6, AF3);
inline constexpr PinBase PC7(PORT_C, PIN_7, AF3);
inline constexpr PinBase PC8(PORT_C, PIN_8, AF2);
inline constexpr PinBase PC9(PORT_C, PIN_9, AF2);
inline constexpr PinBase PC10(PORT_C, PIN_10);
inline constexpr PinBase PC11(PORT_C, PIN_11);
inline constexpr PinBase PC12(PORT_C, PIN_12);
inline constexpr PinBase PC13(PORT_C, PIN_13);
inline constexpr PinBase PC14(PORT_C, PIN_14);
inline constexpr PinBase PC15(PORT_C, PIN_15);
inline constexpr PinBase PD0(PORT_D, PIN_0);
inline constexpr PinBase PD1(PORT_D, PIN_1);
inline constexpr PinBase PD2(PORT_D, PIN_2);
inline constexpr PinBase PD3(PORT_D, PIN_3);
inline constexpr PinBase PD4(PORT_D, PIN_4);
inline constexpr PinBase PD5(PORT_D, PIN_5, AF7);
inline constexpr PinBase PD6(PORT_D, PIN_6, AF7);
inline constexpr PinBase PD7(PORT_D, PIN_7);
inline constexpr PinBase PD8(PORT_D, PIN_8);
inline constexpr PinBase PD9(PORT_D, PIN_9);
inline constexpr PinBase PD10(PORT_D, PIN_10);
inline constexpr PinBase PD11(PORT_D, PIN_11);
inline constexpr PinBase PD12(PORT_D, PIN_12, AF2);
inline constexpr PinBase PD13(PORT_D, PIN_13, AF2);
inline constexpr PinBase PD14(PORT_D, PIN_14, AF2);
inline constexpr PinBase PD15(PORT_D, PIN_15, AF2);
inline constexpr PinBase PE0(PORT_E, PIN_0);
inline constexpr PinBase PE1(PORT_E, PIN_1);
inline constexpr PinBase PE2(PORT_E, PIN_2);
inline constexpr PinBase PE3(PORT_E, PIN_3);
inline constexpr PinBase PE4(PORT_E, PIN_4, AF4);
inline constexpr PinBase PE5(PORT_E, PIN_5, AF4);
inline constexpr PinBase PE6(PORT_E, PIN_6, AF4);
inline constexpr PinBase PE7(PORT_E, PIN_7);
inline constexpr PinBase PE8(PORT_E, PIN_8, AF1);
inline constexpr PinBase PE9(PORT_E, PIN_9, AF1);
inline constexpr PinBase PE10(PORT_E, PIN_10, AF1);
inline constexpr PinBase PE11(PORT_E, PIN_11, AF1);
inline constexpr PinBase PE12(PORT_E, PIN_12, AF1);
inline constexpr PinBase PE13(PORT_E, PIN_13, AF1);
inline constexpr PinBase PE14(PORT_E, PIN_14, AF1);
inline constexpr PinBase PE15(PORT_E, PIN_15);
inline constexpr PinBase PF0(PORT_F, PIN_0, AF13);
inline constexpr PinBase PF1(PORT_F, PIN_1, AF13);
inline constexpr PinBase PF2(PORT_F, PIN_2, AF13);
inline constexpr PinBase PF3(PORT_F, PIN_3, AF13);
inline constexpr PinBase PF4(PORT_F, PIN_4);
inline constexpr PinBase PF5(PORT_F, PIN_5);
inline constexpr PinBase PF6(PORT_F, PIN_6);
inline constexpr PinBase PF7(PORT_F, PIN_7);
inline constexpr PinBase PF8(PORT_F, PIN_8);
inline constexpr PinBase PF9(PORT_F, PIN_9);
inline constexpr PinBase PF10(PORT_F, PIN_10);
inline constexpr PinBase PF11(PORT_F, PIN_11);
inline constexpr PinBase PF12(PORT_F, PIN_12);
inline constexpr PinBase PF13(PORT_F, PIN_13);
inline constexpr PinBase PF14(PORT_F, PIN_14);
inline constexpr PinBase PF15(PORT_F, PIN_15);
inline constexpr PinBase PG0(PORT_G, PIN_0);
inline constexpr PinBase PG1(PORT_G, PIN_1);
inline constexpr PinBase PG2(PORT_G, PIN_2);
inline constexpr PinBase PG3(PORT_G, PIN_3);
inline constexpr PinBase PG4(PORT_G, PIN_4);
inline constexpr PinBase PG5(PORT_G, PIN_5);
inline constexpr PinBase PG6(PORT_G, PIN_6);
inline constexpr PinBase PG7(PORT_G, PIN_7);
inline constexpr PinBase PG8(PORT_G, PIN_8);
inline constexpr PinBase PG9(PORT_G, PIN_9, AF2);
inline constexpr PinBase PG10(PORT_G, PIN_10, AF2);
inline constexpr PinBase PG11(PORT_G, PIN_11, AF11);
inline constexpr PinBase PG12(PORT_G, PIN_12);
inline constexpr PinBase PG13(PORT_G, PIN_13, AF11);
inline constexpr PinBase PG14(PORT_G, PIN_14);
inline constexpr PinBase PG15(PORT_G, PIN_15);
inline constexpr PinBase PH0(PORT_H, PIN_0);
inline constexpr PinBase PH1(PORT_H, PIN_1);


// inline constexpr std::array<Pin, 118> pinArray = {
//         PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA9, PA10, PA11, PA12, PA13, PA14, PA15,
//         PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15,
//         PC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7, PC8, PC9, PC10, PC11, PC12, PC13, PC14, PC15,
//         PD0, PD1, PD2, PD3, PD4, PD5, PD6, PD7, PD8, PD9, PD10, PD11, PD12, PD13, PD14, PD15,
//         PE0, PE1, PE2, PE3, PE4, PE5, PE6, PE7, PE8, PE9, PE10, PE11, PE12, PE13, PE14, PE15,
//         PF0, PF1, PF2, PF3, PF4, PF5, PF6, PF7, PF8, PF9, PF10, PF11, PF12, PF13, PF14, PF15,
//         PG0, PG1, PG2, PG3, PG4, PG5, PG6, PG7, PG8, PG9, PG10, PG11, PG12, PG13, PG14, PG15,
//         PH0, PH1
//     };