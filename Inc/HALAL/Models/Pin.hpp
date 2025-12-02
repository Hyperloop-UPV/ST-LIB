#include "HALAL/Models/GPIO.hpp"

using enum ST_LIB::GPIODomain::Port;

namespace ST_LIB {

// Port A
constexpr GPIODomain::Pin PA0{A, GPIO_PIN_0, 0b0111110111111001};
constexpr GPIODomain::Pin PA1{A, GPIO_PIN_1};
constexpr GPIODomain::Pin PA2{A, GPIO_PIN_2};
constexpr GPIODomain::Pin PA3{A, GPIO_PIN_3};
constexpr GPIODomain::Pin PA4{A, GPIO_PIN_4};
constexpr GPIODomain::Pin PA5{A, GPIO_PIN_5};
constexpr GPIODomain::Pin PA6{A, GPIO_PIN_6};
constexpr GPIODomain::Pin PA7{A, GPIO_PIN_7};
constexpr GPIODomain::Pin PA8{A, GPIO_PIN_8};
constexpr GPIODomain::Pin PA9{A, GPIO_PIN_9};
constexpr GPIODomain::Pin PA10{A, GPIO_PIN_10};
constexpr GPIODomain::Pin PA11{A, GPIO_PIN_11};
constexpr GPIODomain::Pin PA12{A, GPIO_PIN_12};
constexpr GPIODomain::Pin PA13{A, GPIO_PIN_13};
constexpr GPIODomain::Pin PA14{A, GPIO_PIN_14};
constexpr GPIODomain::Pin PA15{A, GPIO_PIN_15};

// Port B
constexpr GPIODomain::Pin PB0{B, GPIO_PIN_0};
constexpr GPIODomain::Pin PB1{B, GPIO_PIN_1};
constexpr GPIODomain::Pin PB2{B, GPIO_PIN_2};
constexpr GPIODomain::Pin PB3{B, GPIO_PIN_3};
constexpr GPIODomain::Pin PB4{B, GPIO_PIN_4};
constexpr GPIODomain::Pin PB5{B, GPIO_PIN_5};
constexpr GPIODomain::Pin PB6{B, GPIO_PIN_6};
constexpr GPIODomain::Pin PB7{B, GPIO_PIN_7};
constexpr GPIODomain::Pin PB8{B, GPIO_PIN_8};
constexpr GPIODomain::Pin PB9{B, GPIO_PIN_9};
constexpr GPIODomain::Pin PB10{B, GPIO_PIN_10};
constexpr GPIODomain::Pin PB11{B, GPIO_PIN_11};
constexpr GPIODomain::Pin PB12{B, GPIO_PIN_12};
constexpr GPIODomain::Pin PB13{B, GPIO_PIN_13};
constexpr GPIODomain::Pin PB14{B, GPIO_PIN_14};
constexpr GPIODomain::Pin PB15{B, GPIO_PIN_15};

// Port C
constexpr GPIODomain::Pin PC0{C, GPIO_PIN_0};
constexpr GPIODomain::Pin PC1{C, GPIO_PIN_1};
constexpr GPIODomain::Pin PC2{C, GPIO_PIN_2};
constexpr GPIODomain::Pin PC3{C, GPIO_PIN_3};
constexpr GPIODomain::Pin PC4{C, GPIO_PIN_4};
constexpr GPIODomain::Pin PC5{C, GPIO_PIN_5};
constexpr GPIODomain::Pin PC6{C, GPIO_PIN_6};
constexpr GPIODomain::Pin PC7{C, GPIO_PIN_7};
constexpr GPIODomain::Pin PC8{C, GPIO_PIN_8};
constexpr GPIODomain::Pin PC9{C, GPIO_PIN_9};
constexpr GPIODomain::Pin PC10{C, GPIO_PIN_10};
constexpr GPIODomain::Pin PC11{C, GPIO_PIN_11};
constexpr GPIODomain::Pin PC12{C, GPIO_PIN_12};
constexpr GPIODomain::Pin PC13{C, GPIO_PIN_13};
constexpr GPIODomain::Pin PC14{C, GPIO_PIN_14};
constexpr GPIODomain::Pin PC15{C, GPIO_PIN_15};

// Port D
constexpr GPIODomain::Pin PD0{D, GPIO_PIN_0};
constexpr GPIODomain::Pin PD1{D, GPIO_PIN_1};
constexpr GPIODomain::Pin PD2{D, GPIO_PIN_2};
constexpr GPIODomain::Pin PD3{D, GPIO_PIN_3};
constexpr GPIODomain::Pin PD4{D, GPIO_PIN_4};
constexpr GPIODomain::Pin PD5{D, GPIO_PIN_5};
constexpr GPIODomain::Pin PD6{D, GPIO_PIN_6};
constexpr GPIODomain::Pin PD7{D, GPIO_PIN_7};
constexpr GPIODomain::Pin PD8{D, GPIO_PIN_8};
constexpr GPIODomain::Pin PD9{D, GPIO_PIN_9};
constexpr GPIODomain::Pin PD10{D, GPIO_PIN_10};
constexpr GPIODomain::Pin PD11{D, GPIO_PIN_11};
constexpr GPIODomain::Pin PD12{D, GPIO_PIN_12};
constexpr GPIODomain::Pin PD13{D, GPIO_PIN_13};
constexpr GPIODomain::Pin PD14{D, GPIO_PIN_14};
constexpr GPIODomain::Pin PD15{D, GPIO_PIN_15};

// Port E
constexpr GPIODomain::Pin PE0{E, GPIO_PIN_0};
constexpr GPIODomain::Pin PE1{E, GPIO_PIN_1};
constexpr GPIODomain::Pin PE2{E, GPIO_PIN_2};
constexpr GPIODomain::Pin PE3{E, GPIO_PIN_3};
constexpr GPIODomain::Pin PE4{E, GPIO_PIN_4};
constexpr GPIODomain::Pin PE5{E, GPIO_PIN_5};
constexpr GPIODomain::Pin PE6{E, GPIO_PIN_6};
constexpr GPIODomain::Pin PE7{E, GPIO_PIN_7};
constexpr GPIODomain::Pin PE8{E, GPIO_PIN_8};
constexpr GPIODomain::Pin PE9{E, GPIO_PIN_9};
constexpr GPIODomain::Pin PE10{E, GPIO_PIN_10};
constexpr GPIODomain::Pin PE11{E, GPIO_PIN_11};
constexpr GPIODomain::Pin PE12{E, GPIO_PIN_12};
constexpr GPIODomain::Pin PE13{E, GPIO_PIN_13};
constexpr GPIODomain::Pin PE14{E, GPIO_PIN_14};
constexpr GPIODomain::Pin PE15{E, GPIO_PIN_15};

// Port F
constexpr GPIODomain::Pin PF0{F, GPIO_PIN_0};
constexpr GPIODomain::Pin PF1{F, GPIO_PIN_1};
constexpr GPIODomain::Pin PF2{F, GPIO_PIN_2};
constexpr GPIODomain::Pin PF3{F, GPIO_PIN_3};
constexpr GPIODomain::Pin PF4{F, GPIO_PIN_4};
constexpr GPIODomain::Pin PF5{F, GPIO_PIN_5};
constexpr GPIODomain::Pin PF6{F, GPIO_PIN_6};
constexpr GPIODomain::Pin PF7{F, GPIO_PIN_7};
constexpr GPIODomain::Pin PF8{F, GPIO_PIN_8};
constexpr GPIODomain::Pin PF9{F, GPIO_PIN_9};
constexpr GPIODomain::Pin PF10{F, GPIO_PIN_10};
constexpr GPIODomain::Pin PF11{F, GPIO_PIN_11};
constexpr GPIODomain::Pin PF12{F, GPIO_PIN_12};
constexpr GPIODomain::Pin PF13{F, GPIO_PIN_13};
constexpr GPIODomain::Pin PF14{F, GPIO_PIN_14};
constexpr GPIODomain::Pin PF15{F, GPIO_PIN_15};

// Port G
constexpr GPIODomain::Pin PG0{G, GPIO_PIN_0};
constexpr GPIODomain::Pin PG1{G, GPIO_PIN_1};
constexpr GPIODomain::Pin PG2{G, GPIO_PIN_2};
constexpr GPIODomain::Pin PG3{G, GPIO_PIN_3};
constexpr GPIODomain::Pin PG4{G, GPIO_PIN_4};
constexpr GPIODomain::Pin PG5{G, GPIO_PIN_5};
constexpr GPIODomain::Pin PG6{G, GPIO_PIN_6};
constexpr GPIODomain::Pin PG7{G, GPIO_PIN_7};
constexpr GPIODomain::Pin PG8{G, GPIO_PIN_8};
constexpr GPIODomain::Pin PG9{G, GPIO_PIN_9};
constexpr GPIODomain::Pin PG10{G, GPIO_PIN_10};
constexpr GPIODomain::Pin PG11{G, GPIO_PIN_11};
constexpr GPIODomain::Pin PG12{G, GPIO_PIN_12};
constexpr GPIODomain::Pin PG13{G, GPIO_PIN_13};
constexpr GPIODomain::Pin PG14{G, GPIO_PIN_14};
constexpr GPIODomain::Pin PG15{G, GPIO_PIN_15};

// Port H
constexpr GPIODomain::Pin PH0{H, GPIO_PIN_0};
constexpr GPIODomain::Pin PH1{H, GPIO_PIN_1};
constexpr GPIODomain::Pin PH2{H, GPIO_PIN_2};
constexpr GPIODomain::Pin PH3{H, GPIO_PIN_3};
constexpr GPIODomain::Pin PH4{H, GPIO_PIN_4};
constexpr GPIODomain::Pin PH5{H, GPIO_PIN_5};
constexpr GPIODomain::Pin PH6{H, GPIO_PIN_6};
constexpr GPIODomain::Pin PH7{H, GPIO_PIN_7};
constexpr GPIODomain::Pin PH8{H, GPIO_PIN_8};
constexpr GPIODomain::Pin PH9{H, GPIO_PIN_9};
constexpr GPIODomain::Pin PH10{H, GPIO_PIN_10};
constexpr GPIODomain::Pin PH11{H, GPIO_PIN_11};
constexpr GPIODomain::Pin PH12{H, GPIO_PIN_12};
constexpr GPIODomain::Pin PH13{H, GPIO_PIN_13};
constexpr GPIODomain::Pin PH14{H, GPIO_PIN_14};
constexpr GPIODomain::Pin PH15{H, GPIO_PIN_15};

} // namespace ST_LIB