/*
 * ADC.hpp
 *
 *  Created on: 20 oct. 2022
 *      Author: alejandro
 */

#pragma once
#include <string>

#include "HALAL/Models/DMA/DMA.hpp"
#include "HALAL/Models/LowPowerTimer/LowPowerTimer.hpp"
#include "HALAL/Models/PinModel/Pin.hpp"
#include "stm32h7xx_hal_adc.h"
#if defined(HAL_ADC_MODULE_ENABLED) && defined(HAL_LPTIM_MODULE_ENABLED)

using std::string;

#define ADC_BUF_LEN 16
#define LPTIM1_PERIOD 6875
#define LPTIM2_PERIOD 6875
#define LPTIM3_PERIOD 6875

#define ADC_MAX_VOLTAGE 3.3
#define MAX_12BIT 4095.0
#define MAX_16BIT 65535.0

#define MAX_ADC_INSTANCES 30

namespace ST_ADC {

class Peripheral {
    struct InitData {
       public:
        ADC_TypeDef* adc;
        uint32_t resolution;
        uint32_t external_trigger;

        // Guardamos canales + número de canales (array fijo, sin vector)
        std::array<uint32_t, MAX_ADC_INSTANCES> channels{};
        uint16_t num_channels{0};

        DMA::Stream dma_stream;

        constexpr InitData() = default;
        // (El ctor con std::vector no puede ser constexpr ⇒ lo eliminamos)
    };

    ADC_HandleTypeDef handle;
    InitData init_data;
    LowPowerTimer& timer;

    explicit Peripheral(ADC_HandleTypeDef& handle, LowPowerTimer& timer,
                        InitData& init_data)
        : handle(handle), timer(timer), init_data(init_data) {}

    std::array<const Pin*, MAX_ADC_INSTANCES> pins_{};
    std::array<uint16_t, MAX_ADC_INSTANCES> chans_{};
    uint16_t count_{0};
    alignas(4) volatile uint16_t dma_buf_[MAX_ADC_INSTANCES]{};

   public:
    // Devuelve el índice/rank asignado en la secuencia
    uint16_t attach(const Pin& pin, uint8_t ch) {
        const uint16_t idx = count_;
        pins_[idx] = &pin;
        chans_[idx] = ch;
        init_data.channels[idx] = ch;
        init_data.num_channels = idx + 1;
        ++count_;
        return idx;
    }

    uint16_t read(uint16_t idx) const {
        return (idx < count_) ? dma_buf_[idx] : 0;
    }

    void start() {
        ADC_MultiModeTypeDef multimode = {0};
        ADC_ChannelConfTypeDef sConfig = {0};

        for (uint16_t i = 0; i < count_; ++i) {
            pins_[i]->inscribe<OperationMode::ANALOG>();
        }

        handle.Instance = init_data.adc;
        handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
        handle.Init.Resolution = init_data.resolution;
        handle.Init.ScanConvMode = ADC_SCAN_ENABLE;
        handle.Init.EOCSelection = ADC_EOC_SEQ_CONV;
        handle.Init.LowPowerAutoWait = DISABLE;
        handle.Init.ContinuousConvMode = DISABLE;
        handle.Init.NbrOfConversion = init_data.num_channels;  // <-- FIX
        handle.Init.DiscontinuousConvMode = DISABLE;
        handle.Init.ExternalTrigConv = init_data.external_trigger;
        handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
        if (handle.Instance == ADC3) {
            handle.Init.DMAContinuousRequests = ENABLE;
        } else {
            handle.Init.ConversionDataManagement =
                ADC_CONVERSIONDATA_DMA_CIRCULAR;
        }
        handle.Init.Overrun = ADC_OVR_DATA_PRESERVED;
        handle.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
        handle.Init.OversamplingMode = DISABLE;
        if (HAL_ADC_Init(&handle) != HAL_OK) {
            ErrorHandler("ADC init failed");
            return;
        }

        // 3) Multimode solo para ADC1
        multimode.Mode = ADC_MODE_INDEPENDENT;
        if (handle.Instance == ADC1) {
            if (HAL_ADCEx_MultiModeConfigChannel(&handle, &multimode) !=
                HAL_OK) {
                ErrorHandler("ADC MultiMode config failed");
                return;
            }
        }

        // 4) Canales por orden de inscripción (rank = i+1)
        for (uint16_t i = 0; i < count_; ++i) {
            sConfig.Channel = chans_[i];
            sConfig.Rank = i + 1;  // <-- sin 'ranks[]', directo
            sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
            sConfig.SingleDiff = ADC_SINGLE_ENDED;
            sConfig.OffsetNumber = ADC_OFFSET_NONE;
            sConfig.Offset = 0;
            sConfig.OffsetSignedSaturation = DISABLE;
            if (HAL_ADC_ConfigChannel(&handle, &sConfig) != HAL_OK) {
                ErrorHandler("ADC ConfigChannel failed");
                return;
            }
        }

        // 5) Configura DMA para que escriba en dma_buf_ con longitud = count_
        //    (usa tu HALAL::DMA aquí con init_data.dma_stream)
        //    DMA::configure(init_data.dma_stream, dma_buf_, count_);

        // 6) Timer/trigger si procede
        timer.init();
    }
};

Peripheral ADC_P1{};
Peripheral ADC_P2{};
Peripheral ADC_P3{};

struct ADCEntry {
    const Pin& pin;
    Peripheral& adc;
    uint8_t ch;
};

inline constexpr std::array<ADCEntry, 18 + 16 + 13> Catalog = {{
    // --- ADC3 ---
    ADCEntry{PF3, ADC_P3, 5},
    ADCEntry{PF4, ADC_P3, 9},
    ADCEntry{PF5, ADC_P3, 4},
    ADCEntry{PF6, ADC_P3, 8},
    ADCEntry{PF7, ADC_P3, 3},
    ADCEntry{PF8, ADC_P3, 7},
    ADCEntry{PF9, ADC_P3, 2},
    ADCEntry{PF10, ADC_P3, 6},
    ADCEntry{PC0, ADC_P3, 10},
    ADCEntry{PC1, ADC_P3, 11},
    ADCEntry{PC2, ADC_P3, 12},
    ADCEntry{PC2, ADC_P3, 0},
    ADCEntry{PC3, ADC_P3, 1},

    // --- ADC2 ---
    ADCEntry{PC0, ADC_P2, 10},
    ADCEntry{PC1, ADC_P2, 11},
    ADCEntry{PC2, ADC_P2, 12},
    ADCEntry{PC3, ADC_P2, 13},
    ADCEntry{PA2, ADC_P2, 14},
    ADCEntry{PA3, ADC_P2, 15},
    ADCEntry{PA4, ADC_P2, 18},
    ADCEntry{PA5, ADC_P2, 19},
    ADCEntry{PA6, ADC_P2, 3},
    ADCEntry{PC4, ADC_P2, 4},
    ADCEntry{PC5, ADC_P2, 8},
    ADCEntry{PA7, ADC_P2, 7},
    ADCEntry{PB1, ADC_P2, 5},
    ADCEntry{PF13, ADC_P2, 2},
    ADCEntry{PF14, ADC_P2, 6},
    ADCEntry{PB0, ADC_P2, 9},

    // --- ADC1 ---
    ADCEntry{PA0, ADC_P1, 16},
    ADCEntry{PA1, ADC_P1, 17},
    ADCEntry{PC0, ADC_P1, 10},
    ADCEntry{PC1, ADC_P1, 11},
    ADCEntry{PC2, ADC_P1, 12},
    ADCEntry{PC3, ADC_P1, 13},
    ADCEntry{PA2, ADC_P1, 14},
    ADCEntry{PA3, ADC_P1, 15},
    ADCEntry{PA4, ADC_P1, 18},
    ADCEntry{PA5, ADC_P1, 19},
    ADCEntry{PA6, ADC_P1, 3},
    ADCEntry{PC4, ADC_P1, 4},
    ADCEntry{PC5, ADC_P1, 8},
    ADCEntry{PA7, ADC_P1, 7},
    ADCEntry{PB1, ADC_P1, 5},
    ADCEntry{PF11, ADC_P1, 2},
    ADCEntry{PF12, ADC_P1, 6},
    ADCEntry{PB0, ADC_P1, 9},
}};

enum class Strategy { Auto, Prefer, Only };

template <auto& PinObj, Strategy = Strategy::Auto>
consteval ADCEntry select_option() {
    std::array<MapEntry, 8> opts{};
    std::size_t n = 0;
    for (auto e : Catalog)
        if (e.pin == &PinObj) opts[n++] = e;
    static_assert(n > 0,
                  "This pin is not mapped with any ADC Peripheral in this MCU");

    if constexpr (std::is_same_v<Strategy, Auto>) {
        return opts[0];  // si quieres balanceo, aquí puedes aplicar una
                         // política CT simple
    } else {
        constexpr Id want = Strategy::value;  // Prefer/Only
        for (std::size_t i = 0; i < n; i++)
            if (opts[i].adc == want) return opts[i];
        if constexpr (std::is_same_v<Strategy, Prefer<want>>)
            return opts[0];
        else
            static_assert(
                [] { return false; }(),
                "This pin can't be linked to the peripheral requested");
    }
    return opts[0];
}

template <Pin& pin, Strategy s>
struct Instance {
    consteval Instance() {}

    uint16_t read() {}

   private:
    ADCEntry entry{select_option<pin>()};
    void start() {}
};
};  // namespace ST_ADC

/**
 * @brief A utility class that controls ADC inputs.
 *
 * This class handles the configuration, read, and correct casting of the ADC
 * values in the ADC inputs using DMA to make cyclic reads, and the code pulls
 * from the space given to the DMA memory. For the class to work, the pins used
 * as parameters in each inscribe has to be declared on Runes and on Pins as
 * ADCs (change adc_instances and AF of pins, respectively). Moreover, if the
 * ADC used is not on the code generated by the .ioc you may need to change
 * HAL_ADC_MspInit (on stm32h7xx_hal_msp.c) to configure the ADC pins and
 * channels and their DMAs. Template-project has all the adcs declared on runes
 * and the .ioc correctly configured, so unless more are needed these
 * configurations can be assumed.
 */
// class ADC {
//    public:
//     struct InitData {
//        public:
//         ADC_TypeDef* adc;
//         uint32_t resolution;
//         uint32_t external_trigger;

//         DMA::Stream dma_stream;

//         constexpr InitData() = default;
//         constexpr InitData(ADC_TypeDef* adc, uint32_t resolution,
//                            uint32_t external_trigger, DMA::Stream dma_stream)
//             : adc(adc),
//               resolution(resolution),
//               external_trigger(external_trigger),
//               dma_stream(dma_stream) {}
//     };

//     template <size_t T>
//     class Peripheral {
//        public:
//         array<bool, 20> channels_active{false};
//         array<pair<Pin, uint8_t>, T>& pins_channel_available;
//         handleTypeDef* handle;
//         uint16_t* dma_data_buffer;
//         LowPowerTimer timer;
//         InitData init_data;
//         uint8_t channels_in_use{0};

//         constexpr Peripheral() = default;
//         constexpr Peripheral(handleTypeDef* handle, LowPowerTimer& timer,
//                              InitData& init_data,
//                              array<pair<Pin, int>, T>&
//                              pins_channel_available);

//         bool is_registered();
//     };

//     class Instance {
//        public:
//         Peripheral<T>* peripheral;
//         uint32_t channel;
//         uint32_t rank;

//         Instance() = default;
//         Instance(Peripheral<T>* peripheral, uint32_t channel);
//     };

//     /**
//      * @brief A method to add a pin as an ADC input on the ST-LIB.

//      * This method has to be invoked before the ST-LIB::start(), as it marks
//      the pin to be configured as an ADC, and that configuration is made in
//      the ST-LIB::start().
//      * As said on the class description, only correctly configured ADC pins
//      will work correctly when declared as an ADC.
//      * It is forbidden to declare a pin as an ADC and as anything else on
//      services at the same time, and it will result in undefined behavior.
//      *
//      * @param pin	the Pin to be added as an ADC
//      *
//      * @return the id that represents the ADC inside this utility class, used
//      in all its functions.
//      */
//     consteval static uint8_t inscribe(Pin pin) { pin.inscribe<ANALOG>(); }

//     /**
//      * @brief Method used in ST-LIBstart() to configure pins inscribed as
//      ADCs.
//      *
//      * The start methods of the HALAL are made to be invoked in an specific
//      * order. As such, its recommended to not use them isolated, and instead
//      use
//      * the ST-LIB::start(). The If for any reason not starting a service from
//      * HALAL was desired, removing the definition of its HAL parent module
//      would
//      * deactivate it. For example, if ADC was needed out of the code,
//      removing
//      * define HAL_ADC_MODULE_ENABLED on stm32h7xx_hal_conf.h would result in
//      * that behavior.
//      */
//     static void start();

//     /**
//      * @brief Activates the ADC represented by the id.
//      *
//      * After the ADC is configured in the ST-LIBstart::() it needs to be
//      * activated with turn_on to start reading. This is made this way so the
//      * user can control exactly when the measures start to be taken. The
//      * get_value() method will only work on a Pin that has both been
//      inscribed,
//      * started, and turned on.
//      *
//      * @param 	id	the id of ADC to be activated.
//      */
//     static void turn_on(uint8_t id);

//     /**
//      * @brief Returns the value of the last DMA read made by the ADC.
//      *
//      * The get_value function doesn t issue a read, but rather pulls the
//      memory
//      * where the last read made is saved, transforms the value with the
//      * reference voltage and returns the voltage represented with that value.
//      * The capture of the value is made automatically by the DMA configured
//      for
//      * the respective ADC channel, and the frequency of the reads is
//      dependant
//      * on the configuration of the DMA itself.
//      *
//      * @param id	the id of the ADC to be read.
//      *
//      * @return the value of the ADC in volts. The ADC_MAX_VOLTAGE needs to be
//      * correctly configured in order for this function to work.
//      */
//     static float get_value(uint8_t id);

//     /**
//      * @brief Returns the value of the last DMA read made by the ADC.
//      *
//      * The get_int_value function doesn t issue a read, but rather pulls the
//      * memory where the last read made is saved and returns that value. The
//      * capture of the value is made automatically by the DMA configured for
//      the
//      * respective ADC channel, and the frequency of the reads is dependant on
//      * the configuration of the DMA itself.
//      *
//      * @param id	the id of the ADC to be read.
//      *
//      * @return the value of the ADC, in uint16_t format. 0 is minimum
//      possible
//      * value and max_uint16_t is the maximum.
//      */
//     static uint16_t get_int_value(uint8_t id);

//     /**
//      * @brief Function that returns the pointer where the DMA of the ADC
//      writes
//      * its value, for maximum efficiency on the access
//      *
//      * This function has no protection of any kind, other that checking that
//      an
//      * adc exists before giving the pointer back. If the ADC is running or
//      not
//      * should be handled by the user. The adcs of the adc3 peripheral are not
//      * aligned in the buffer, and are instead aligned in the get functions.
//      If
//      * the values are accessed from the buffer, is the responsibility of the
//      * user to handle the shift problems.
//      */
//     static uint16_t* get_value_pointer(uint8_t id);

//     static Peripheral<T> peripherals[3];

//    private:
//     static uint32_t ranks[16];
//     static map<Pin, Instance> available_instances;
//     static unordered_map<uint8_t, Instance> active_instances;
//     static uint8_t id_counter;

//     static void init(Peripheral<T>& peripheral);
//     static void initialize_from_registry();
// };

// inline constexpr array<pair<const Pin, uint8_t>, 12>
//     ADC3_pins_channels_availables = {
//         std::make_pair(PF3, 5),
//         std::make_pair(PF4, 9),
//         std::make_pair(PF5, 4),
//         std::make_pair(PF6, 8),
//         std::make_pair(PF7, 3),
//         std::make_pair(PF8, 7),
//         std::make_pair(PF9, 2),
//         std::make_pair(PF10, 6),
//         std::make_pair(PC0, 10),
//         std::make_pair(PC1, 11),
//         std::make_pair(PC2, 0),
//         std::make_pair(
//             PC3, 1)};  // IT SAIS PC2 IN ADC3 CAN BE IN CHANEL 12 or channel
//             0

// inline constexpr array<pair<const Pin, uint8_t>, 16>
//     ADC2_pins_channels_availables = {
//         std::make_pair(PC0, 10), std::make_pair(PC1, 11),
//         std::make_pair(PC2, 12), std::make_pair(PC3, 13),
//         std::make_pair(PA2, 14), std::make_pair(PA3, 15),
//         std::make_pair(PA4, 18), std::make_pair(PA5, 19),
//         std::make_pair(PA6, 3),  std::make_pair(PC4, 4),
//         std::make_pair(PC5, 8),  std::make_pair(PA7, 7),
//         std::make_pair(PB1, 5),  std::make_pair(PF13, 2),
//         std::make_pair(PF14, 6), std::make_pair(PB0, 9)};
// inline constexpr array<pair<const Pin, uint8_t>, 18>
//     ADC1_pins_channels_availables = {
//         std::make_pair(PA0, 16), std::make_pair(PA1, 17),
//         std::make_pair(PC0, 10), std::make_pair(PC1, 11),
//         std::make_pair(PC2, 12), std::make_pair(PC3, 13),
//         std::make_pair(PA2, 14), std::make_pair(PA3, 15),
//         std::make_pair(PA4, 18), std::make_pair(PA5, 19),
//         std::make_pair(PA6, 3),  std::make_pair(PC4, 4),
//         std::make_pair(PC5, 8),  std::make_pair(PA7, 7),
//         std::make_pair(PB1, 5),  std::make_pair(PF11, 2),
//         std::make_pair(PF12, 6), std::make_pair(PB0, 9)};
#endif
