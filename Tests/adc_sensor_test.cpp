#include <array>
#include <span>

#include <gtest/gtest.h>

#include "Control/Blocks/MovingAverage.hpp"
#include "HALAL/Models/DMA/DMA2.hpp"
#include "HALAL/Services/ADC/ADC.hpp"
#include "MockedDrivers/NVIC.hpp"
#include "MockedDrivers/mocked_hal_adc.hpp"
#include "MockedDrivers/mocked_hal_dma.hpp"
#include "Sensors/Common/PT100.hpp"
#include "Sensors/LinearSensor/FilteredLinearSensor.hpp"
#include "Sensors/LinearSensor/LinearSensor.hpp"
#include "Sensors/LookupSensor/LookupSensor.hpp"
#include "Sensors/NTC/NTC.hpp"

namespace {

inline float adc_sensor_template_output_0 = 0.0f;
inline float adc_sensor_template_output_1 = 0.0f;

constexpr std::array<ST_LIB::DMADomain::Entry, 2> adc_dma_entries{{
    {.instance = ST_LIB::DMADomain::Peripheral::adc1,
     .stream = ST_LIB::DMADomain::Stream::dma1_stream0,
     .irqn = DMA1_Stream0_IRQn,
     .id = 0},
    {.instance = ST_LIB::DMADomain::Peripheral::adc2,
     .stream = ST_LIB::DMADomain::Stream::dma1_stream1,
     .irqn = DMA1_Stream1_IRQn,
     .id = 0},
}};

constexpr auto adc_dma_cfg =
    ST_LIB::DMADomain::build<2>(std::span<const ST_LIB::DMADomain::Entry, 2>{adc_dma_entries});

constexpr std::array<ST_LIB::ADCDomain::Config, 1> single_adc1_init_cfgs{{
    {.gpio_idx = 0,
     .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
     .channel = ST_LIB::ADCDomain::Channel::CH16,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .dma_request = DMA_REQUEST_ADC1,
     .output = &adc_sensor_template_output_0},
}};

constexpr std::array<ST_LIB::ADCDomain::Config, 2> split_adc12_init_cfgs{{
    {.gpio_idx = 0,
     .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
     .channel = ST_LIB::ADCDomain::Channel::CH16,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .dma_request = DMA_REQUEST_ADC1,
     .output = &adc_sensor_template_output_0},
    {.gpio_idx = 1,
     .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_2,
     .channel = ST_LIB::ADCDomain::Channel::CH2,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_16,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .dma_request = DMA_REQUEST_ADC2,
     .output = &adc_sensor_template_output_1},
}};

template <std::size_t N, const std::array<ST_LIB::ADCDomain::Config, N>& Cfgs>
using ADCInit = ST_LIB::ADCDomain::Init<N, Cfgs>;

using SingleADCInit = ADCInit<1, single_adc1_init_cfgs>;
using SplitADCInit = ADCInit<2, split_adc12_init_cfgs>;

void clear_nvic_enables() {
    for (auto& reg : NVIC->ISER) {
        reg = 0U;
    }
}

void clear_dma_irq_table() {
    for (auto& slot : dma_irq_table) {
        slot = nullptr;
    }
}

class ADCSensorTest : public ::testing::Test {
protected:
    void SetUp() override {
        ST_LIB::ClockDomain::s_tree = &ST_LIB::default_clock_tree;
        ST_LIB::MockedHAL::adc_reset();
        ST_LIB::MockedHAL::dma_reset();
        clear_nvic_enables();
        clear_dma_irq_table();
    }

    template <std::size_t N, const std::array<ST_LIB::ADCDomain::Config, N>& InitCfgs>
    void init_adc_with_dma(const std::array<ST_LIB::ADCDomain::Config, N>& cfgs) {
        ST_LIB::DMADomain::Init<2>::init(adc_dma_cfg);
        ADCInit<N, InitCfgs>::init(
            cfgs,
            std::span<ST_LIB::GPIODomain::Instance>{},
            std::span<ST_LIB::DMADomain::Instance>(ST_LIB::DMADomain::Init<2>::instances)
        );
    }
};

TEST_F(ADCSensorTest, LinearSensorUsesNormalizedADCVoltageForItsTransferFunction) {
    float output = -1.0f;
    const std::array<ST_LIB::ADCDomain::Config, 1> cfgs{{
        {.gpio_idx = 0,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH16,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_10,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &output},
    }};

    init_adc_with_dma<1, single_adc1_init_cfgs>(cfgs);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 512U);

    LinearSensor<float> sensor(SingleADCInit::instances[0], 2.0f, -1.0f, output, 5.0f);
    sensor.read();

    EXPECT_NEAR(output, 2.0f * ((512.0f / 1023.0f) * 5.0f) - 1.0f, 0.001f);
}

TEST_F(ADCSensorTest, FilteredLinearSensorReusesTheSameADCConversionPath) {
    float output = -1.0f;
    const std::array<ST_LIB::ADCDomain::Config, 1> cfgs{{
        {.gpio_idx = 0,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH16,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &output},
    }};

    init_adc_with_dma<1, single_adc1_init_cfgs>(cfgs);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 2048U);

    MovingAverage<1> filter;
    FilteredLinearSensor<float, 1> sensor(SingleADCInit::instances[0], 2.0f, 1.0f, output, filter);

    sensor.read();
    sensor.read();

    EXPECT_NEAR(output, 2.0f * ((2048.0f / 4095.0f) * 3.3f) + 1.0f, 0.001f);
}

TEST_F(ADCSensorTest, LookupSensorMapsEquivalentNormalizedReadingsAcrossResolutions) {
    double out12 = -1.0;
    double out16 = -1.0;
    constexpr std::array<double, 4> table{10.0, 20.0, 30.0, 40.0};
    const std::array<ST_LIB::ADCDomain::Config, 2> cfgs{{
        {.gpio_idx = 0,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH16,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = nullptr},
        {.gpio_idx = 1,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_2,
         .channel = ST_LIB::ADCDomain::Channel::CH2,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_16,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC2,
         .output = nullptr},
    }};

    init_adc_with_dma<2, split_adc12_init_cfgs>(cfgs);

    LookupSensor span_sensor(SplitADCInit::instances[0], std::span<const double>(table), out12);
    LookupSensor ptr_sensor(SplitADCInit::instances[1], table.data(), table.size(), out16);

    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 2048U);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC2, ADC_CHANNEL_2, 32768U);
    span_sensor.read();
    ptr_sensor.read();

    EXPECT_DOUBLE_EQ(out12, 30.0);
    EXPECT_DOUBLE_EQ(out16, 30.0);

    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 4095U);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC2, ADC_CHANNEL_2, 65535U);
    span_sensor.read();
    ptr_sensor.read();

    EXPECT_DOUBLE_EQ(out12, 40.0);
    EXPECT_DOUBLE_EQ(out16, 40.0);
}

TEST_F(ADCSensorTest, PT100ReadsFromADCVoltageAndSupportsFilteredMode) {
    float direct_output = -1.0f;
    float filtered_output = -1.0f;
    const std::array<ST_LIB::ADCDomain::Config, 1> cfgs{{
        {.gpio_idx = 0,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH16,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &direct_output},
    }};

    init_adc_with_dma<1, single_adc1_init_cfgs>(cfgs);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 2048U);

    PT100<1> direct_sensor(SingleADCInit::instances[0], direct_output);
    MovingAverage<1> filter;
    PT100<1> filtered_sensor(SingleADCInit::instances[0], filtered_output, filter);

    direct_sensor.read();
    filtered_sensor.read();
    filtered_sensor.read();

    const float voltage = (2048.0f / 4095.0f) * 3.3f;
    const float expected_temperature = PT100<1>::k / voltage + PT100<1>::offset;

    EXPECT_NEAR(direct_output, expected_temperature, 0.001f);
    EXPECT_NEAR(filtered_output, expected_temperature, 0.001f);
}

TEST_F(ADCSensorTest, NTCUsesNormalizedADCCountsAcrossResolutions) {
    float out12 = -1.0f;
    float out16 = -1.0f;
    const std::array<ST_LIB::ADCDomain::Config, 2> cfgs{{
        {.gpio_idx = 0,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH16,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = nullptr},
        {.gpio_idx = 1,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_2,
         .channel = ST_LIB::ADCDomain::Channel::CH2,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_16,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC2,
         .output = nullptr},
    }};

    init_adc_with_dma<2, split_adc12_init_cfgs>(cfgs);

    NTC ntc12(SplitADCInit::instances[0], out12);
    NTC ntc16(SplitADCInit::instances[1], out16);

    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 2047U);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC2, ADC_CHANNEL_2, 32767U);
    ntc12.read();
    ntc16.read();
    EXPECT_FLOAT_EQ(out12, out16);

    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 4095U);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC2, ADC_CHANNEL_2, 65535U);
    ntc12.read();
    ntc16.read();
    EXPECT_FLOAT_EQ(out12, out16);
}

} // namespace
