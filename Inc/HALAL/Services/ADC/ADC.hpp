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
#define NUM_PERIPHERALS 3
//handles
LPTIM_HandleTypeDef hlptim1;
LPTIM_HandleTypeDef hlptim2;
LPTIM_HandleTypeDef hlptim3;
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

enum class ADCId: uint8_t {
            peripheral1 = 1,
            peripheral2 = 2,
            peripheral3 = 3
        };
constexpr std::array<uint32_t,16> ranks = {
    ADC_REGULAR_RANK_1,  ADC_REGULAR_RANK_2,  ADC_REGULAR_RANK_3,
    ADC_REGULAR_RANK_4,  ADC_REGULAR_RANK_5,  ADC_REGULAR_RANK_6,
    ADC_REGULAR_RANK_7,  ADC_REGULAR_RANK_8,  ADC_REGULAR_RANK_9,
    ADC_REGULAR_RANK_10, ADC_REGULAR_RANK_11, ADC_REGULAR_RANK_12,
    ADC_REGULAR_RANK_13, ADC_REGULAR_RANK_14, ADC_REGULAR_RANK_15,
    ADC_REGULAR_RANK_16
};
struct InitData {
        ADCId adc;
        uint32_t resolution;
        uint32_t external_trigger;
        const char* name;
        // Guardamos canales + número de canales (array fijo, sin vector)
        DMA::Stream dma_stream;

        consteval InitData() = default;
        consteval InitData(ADCId adc, uint32_t resolution, uint32_t external_trigger, DMA::Stream dma_stream, const char* name):
            adc(adc),resolution(resolution),external_trigger(external_trigger),name(name),dma_stream(dma_stream){};

        ADC_TypeDef* get_adc(){
            using enum ADCId;
            switch (adc){
                case peripheral1: return ADC1;
                case peripheral2: return ADC2;
                case peripheral3: return ADC3;
            }
        }
};
class Peripheral {
    std::array<const Pin*, MAX_ADC_INSTANCES> pins_{};
    std::array<uint16_t, MAX_ADC_INSTANCES> channels_{};
    uint16_t count_{0};
    bool is_on{false};
    alignas(4)  uint16_t dma_buf_[MAX_ADC_INSTANCES]{};

   public:
        ADC_HandleTypeDef* handle;
        LowPowerTimer& timer;
        const InitData& init_data;
        consteval Peripheral() = default;
		consteval Peripheral(ADC_HandleTypeDef* handle,LowPowerTimer& timer, const InitData& init_data):
            handle(handle),timer(timer),init_data(init_data)
        {};
    // Devuelve el índice/rank asignado en la secuencia
    uint32_t consteval attach(const Pin &pin, uint16_t ch) {
        const uint16_t idx = count_;
        pins_[idx] = &pin;
        channels_[idx] = ch;
        ++count_;
//----------------comment this for the test -----------------------------------------------------------------------------------//
       // DMA::inscribe_stream(init_data.dma_stream); 
        return idx;
    }
    inline uint16_t get_adc_count(){
        return count_;
    }
    inline uint16_t get_adc_channel(int idx){
        if(idx >= count_){
            ErrorHandler("Index out of bounds");
        }
        return channels_[idx];
    }
};

inline  LowPowerTimer lptim1(LPTIM1_BASE, hlptim1, LPTIM1_PERIOD, "LPTIM 1");
inline  LowPowerTimer lptim2(LPTIM2_BASE, hlptim2, LPTIM2_PERIOD, "LPTIM 2");
inline  LowPowerTimer lptim3(LPTIM3_BASE, hlptim3, LPTIM3_PERIOD, "LPTIM 3");

inline constexpr InitData init_data1(ADCId::peripheral1, ADC_RESOLUTION_16B, ADC_EXTERNALTRIG_LPTIM1_OUT, DMA::Stream::DMA1Stream0, "ADC 1");
inline constexpr InitData init_data2(ADCId::peripheral2, ADC_RESOLUTION_16B, ADC_EXTERNALTRIG_LPTIM2_OUT, DMA::Stream::DMA1Stream1, "ADC 2");
inline constexpr InitData init_data3(ADCId::peripheral3, ADC_RESOLUTION_12B, ADC_EXTERNALTRIG_LPTIM3_OUT, DMA::Stream::DMA1Stream2, "ADC 3");

inline  Peripheral ADC_P1{&hadc1, lptim1, init_data1};
inline  Peripheral ADC_P2{&hadc2, lptim2, init_data2};
inline  Peripheral ADC_P3{&hadc3, lptim3, init_data3};

struct ADC_Peripherals{
    std::array<Peripheral *,NUM_PERIPHERALS> peripherals = {
        &ADC_P1,&ADC_P2,&ADC_P3
    };
    void start(){
        for(uint8_t i = 0; i < NUM_PERIPHERALS; i++){
            if(peripherals[i]->get_adc_count()){
                init(peripherals[i]);
            }
        }
    }
    void init(Peripheral* peripheral){
        ADC_MultiModeTypeDef multimode = {0};
        ADC_ChannelConfTypeDef sConfig = {0};
        ADC_HandleTypeDef& adc_handle = *peripheral->handle;
        InitData init_data = peripheral->init_data;

        adc_handle.Instance = init_data.get_adc();
        adc_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
        adc_handle.Init.Resolution = init_data.resolution;
        adc_handle.Init.ScanConvMode = ADC_SCAN_ENABLE;
        adc_handle.Init.EOCSelection = ADC_EOC_SEQ_CONV;
        adc_handle.Init.LowPowerAutoWait = DISABLE;
        adc_handle.Init.ContinuousConvMode = DISABLE;
        adc_handle.Init.NbrOfConversion = peripheral->get_adc_count();
        adc_handle.Init.DiscontinuousConvMode = DISABLE;
        adc_handle.Init.ExternalTrigConv = init_data.external_trigger;
        adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
        if (adc_handle.Instance == ADC3) {
            adc_handle.Init.DMAContinuousRequests = ENABLE;
        }else{
            adc_handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
        }
        adc_handle.Init.Overrun = ADC_OVR_DATA_PRESERVED;
        adc_handle.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
        adc_handle.Init.OversamplingMode = DISABLE;
        if (HAL_ADC_Init(&adc_handle) != HAL_OK) {
            ErrorHandler("ADC  - %s - did not start correctly", init_data.name);
            return;
        }
        multimode.Mode = ADC_MODE_INDEPENDENT;
        if(adc_handle.Instance == ADC1){
            if (HAL_ADCEx_MultiModeConfigChannel(&adc_handle, &multimode) != HAL_OK) {
                ErrorHandler("ADC MultiModeConfigChannel - %s - did not start correctly", init_data.name);
                return;
            }
        }
        for(int counter = 0; counter < peripheral->get_adc_count();counter++){
            sConfig.Channel = peripheral->get_adc_channel(counter);
            sConfig.Rank = ranks[counter];
            sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
            sConfig.SingleDiff = ADC_SINGLE_ENDED;
            sConfig.OffsetNumber = ADC_OFFSET_NONE;
            sConfig.Offset = 0;
            sConfig.OffsetSignedSaturation = DISABLE;
            if (HAL_ADC_ConfigChannel(&adc_handle, &sConfig) != HAL_OK) {
                ErrorHandler("ADC ConfigChannel - %s - did not start correctly", init_data.name);
            }
        }
        peripheral->timer.init();
    }
};
struct ADCEntry {
    const Pin& pin;
    Peripheral& adc_peripheral;
    uint8_t ch;
    consteval ADCEntry(const Pin &pin,Peripheral &adc_peripheral,uint8_t ch):
        pin(pin), adc_peripheral(adc_peripheral), ch(ch){}
};

inline constexpr std::array<ADCEntry, 18 + 16 + 13> Catalog = {{
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

}};

enum class Strategy { Auto, Prefer, Only, LowPower};
enum class Resolution: uint32_t{
    BITS_16 = 16,
    BITS_12 = 12,
};

template <Pin& pin, Strategy strategy = Strategy::Auto,ADCId adcId = ADCId::peripheral1,Resolution resolution = Resolution::BITS_16>
//The priority is Strategy > resolution > peripheral
// In case auto or LowPower Resolution > Strategy
consteval ADCEntry select_option(uint32_t* rank) {
    //inscribe the pin as analog
    pin.inscribe<OperationMode::ANALOG>();
    std::array<ADCEntry, 4> opts{};
    std::size_t n = 0;
    for (auto e : Catalog)
        if (e.pin == pin) opts.at(n++) = e;

    if(n == 0){
        throw "No options available";
    }
    if constexpr (strategy == Strategy::Prefer || strategy == Strategy::Only){
        for (std::size_t i = 0; i < n; i++)
            if (opts[i].adc_peripheral.init_data.adc == adcId && opts[i].adc_peripheral.init_data.resolution == static_cast<uint32_t>(resolution)) return opts[i];
        if(strategy == Strategy::Only){
            throw "No options available";
        }
    }
    uint16_t min = opts[0].adc_peripheral.count_;
    size_t best_entry_auto = 0;
    uint32_t resol_temp_auto = opts[0].adc_peripheral.init_data.resolution;
    uint32_t resol_temp_low_power = opts[0].adc_peripheral.init_data.resolution;
    uint16_t max = opts[0].adc_peripheral.count_;
    size_t best_entry_low_power = 0;
    for(size_t i = 1; i < n; i++){
        //check lowPower and Auto 
        if(min > opts[i].adc_peripheral.count_ ||
            (resol_temp_auto  <= static_cast<uint32_t>(resolution)  && static_cast<uint32_t>(resolution) == opts[i].adc_peripheral.init_data.resolution)){
            min = opts[i].adc_peripheral.count_;
            resol_temp_auto = opts[i].adc_peripheral.init_data.resolution;
            best_entry_auto = i;
        }
        if(max < opts[i].adc_peripheral.count_ ||  
            (resol_temp_low_power <= static_cast<uint32_t>(resolution)  && static_cast<uint32_t>(resolution) == opts[i].adc_peripheral.init_data.resolution)){
            max = opts[i].adc_peripheral.count_;
            resol_temp_auto = opts[i].adc_peripheral.init_data.resolution;
            best_entry_low_power = i;
        }
    }
    if(strategy == Strategy::Auto || strategy == Strategy::Prefer) return opts[best_entry_auto];
    if(strategy == Strategy::LowPower) return opts[best_entry_low_power];
    return opts[0];
}

template <Pin& pin, Strategy strategy = Strategy::Auto,ADCId adcId = ADCId::peripheral1,Resolution resolution = Resolution::BITS_16>
class ADC {
    ADCEntry adc;
    uint32_t rank{};
    consteval ADC() {
        adc = inscribe(); // choose a peripheric
        rank = adc.adc_peripheral.attach(adc.pin,adc.ch); //Rank is assigned based on instantiation order
    }
    void turn_on(){
        //Activate the peripheral if hasn't been activated yet
        Peripheral* peripheral = &adc.adc_peripheral;
        if(peripheral->is_on){
            return;
        }
        uint32_t buffer_length = peripheral->count_;
        if (HAL_ADC_Start_DMA(peripheral->handle, (uint32_t*) peripheral->dma_buf_, buffer_length) != HAL_OK) {
            ErrorHandler("DMA - %d - of ADC - %s - did not start correctly", peripheral->init_data.dma_stream, adc.adc_peripheral.init_data.name);
            return;
        }

        LowPowerTimer& timer = peripheral->timer;
        if (HAL_LPTIM_TimeOut_Start_IT(&timer.handle, timer.period, timer.period / 2) != HAL_OK) {
            ErrorHandler("LPTIM - %d - of ADC - %d - did not start correctly", timer.name, peripheral->handle);
            return;
        }
        peripheral->is_on = true;
        }

	/**
	 * @brief Returns the value of the last DMA read made by the ADC.
	 *
	 * The get_int_value function doesn t issue a read, but rather pulls the memory where the last read made is saved and returns that value.
	 * The capture of the value is made automatically by the DMA configured for the respective ADC channel, and the frequency of the reads is
	 * dependant on the configuration of the DMA itself.
	 *
	 * @return the value of the ADC, in uint16_t format. 0 is minimum possible value and max_uint16_t is the maximum.
    **/
    uint16_t get_int_value() {
        uint16_t raw = adc.adc_peripheral.dma_buf_[rank];
        if(adc.adc_peripheral.init_data.adc == ADCId::peripheral3){
            return raw << 4;
        }
        return raw;
    }
    /**
	 * @brief Returns the value of the last DMA read made by the ADC.
	 *
	 * The get_value function doesn t issue a read, but rather pulls the memory where the last read made is saved, transforms the value
	 * with the reference voltage and returns the voltage represented with that value.
	 * The capture of the value is made automatically by the DMA configured for the respective ADC channel, and the frequency of the reads is
	 * dependant on the configuration of the DMA itself.
	 *
	 * @return the value of the ADC in volts. The ADC_MAX_VOLTAGE needs to be correctly configured in order for this function to work.
	 */
    float get_value(){
        uint16_t raw = adc.adc_peripheral.dma_buf_[rank];
        if(adc.adc_peripheral.init_data.adc == ADCId::peripheral3){
            return static_cast<float>(raw)/ MAX_12BIT * ADC_MAX_VOLTAGE;
        }
        return static_cast<float>(raw) / MAX_16BIT * ADC_MAX_VOLTAGE;
    }
    /**
	 * @brief Function that returns the pointer where the DMA of the ADC writes its value, for maximum efficiency on the access
	 *
	 * This function has no protection of any kind, other that checking that an adc exists before giving the pointer back.
	 * If the ADC is running or not should be handled by the user.
	 * The adcs of the adc3 peripheral are not aligned in the buffer, and are instead aligned in the get functions.
	 * If the values are accessed from the buffer, is the responsibility of the user to handle the shift problems.
	 */
    std::shared_ptr<uint16_t> get_value_smart_pointer() {
        uint16_t* ptr = &adc.adc_peripheral.dma_buf_[rank];
        auto no_delete = [](uint16_t*) {};
        return std::shared_ptr<uint16_t>(ptr, no_delete);
    }
    uint16_t* get_value_pointer(){
        return &adc.adc_peripheral.dma_buf_[rank];
    }
   private:
    ADCEntry inscribe(){
        return select_option<pin,strategy,adcId,resolution>();
    }
};


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
