#include <array>

#include <gtest/gtest.h>

#include "HALAL/Services/ADC/NewADC.hpp"
#include "MockedDrivers/mocked_hal_adc.hpp"

namespace {

inline float compile_time_output = 0.0f;

constexpr std::array<ST_LIB::ADCDomain::Entry, 1> auto_entry{{
    {.gpio_idx = 0,
     .pin = ST_LIB::PA0,
     .peripheral = ST_LIB::ADCDomain::Peripheral::AUTO,
     .channel = ST_LIB::ADCDomain::Channel::AUTO,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
     .sample_rate_hz = 0,
     .output = &compile_time_output},
}};

constexpr auto auto_cfg = ST_LIB::ADCDomain::build<1>(
    std::span<const ST_LIB::ADCDomain::Entry, 1>{auto_entry});

static_assert(auto_cfg[0].peripheral == ST_LIB::ADCDomain::Peripheral::ADC_1);
static_assert(auto_cfg[0].channel == ST_LIB::ADCDomain::Channel::CH16);

constexpr std::array<ST_LIB::ADCDomain::Entry, 1> internal_entry{{
    {.gpio_idx = 0,
     .pin = ST_LIB::PA0,
     .peripheral = ST_LIB::ADCDomain::Peripheral::AUTO,
     .channel = ST_LIB::ADCDomain::Channel::VREFINT,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
     .sample_rate_hz = 0,
     .output = &compile_time_output},
}};

constexpr auto internal_cfg = ST_LIB::ADCDomain::build<1>(
    std::span<const ST_LIB::ADCDomain::Entry, 1>{internal_entry});
#if STLIB_HAS_ADC3
static_assert(internal_cfg[0].peripheral == ST_LIB::ADCDomain::Peripheral::ADC_3);
#else
static_assert(internal_cfg[0].peripheral == ST_LIB::ADCDomain::Peripheral::ADC_2);
#endif

} // namespace

class ADCTest : public ::testing::Test {
protected:
  void SetUp() override { ST_LIB::MockedHAL::adc_reset(); }
};

TEST_F(ADCTest, PollingReadUpdatesOutputValue) {
  float output = -1.0f;
  const std::array<ST_LIB::ADCDomain::Config, 1> cfgs{{
      {.gpio_idx = 0,
       .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
       .channel = ST_LIB::ADCDomain::Channel::CH16,
       .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
       .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
       .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
       .sample_rate_hz = 0,
       .output = &output},
  }};

  ST_LIB::ADCDomain::Init<1>::init(cfgs);
  ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 2048U);

  auto &adc = ST_LIB::ADCDomain::Init<1>::instances[0];
  adc.read(3.3, 1);

  const float expected = (2048.0f / 4095.0f) * 3.3f;
  EXPECT_NEAR(output, expected, 0.001f);
  EXPECT_EQ(ST_LIB::MockedHAL::adc_get_last_channel(ADC1), ADC_CHANNEL_16);
  EXPECT_FALSE(ST_LIB::MockedHAL::adc_is_running(ADC1));
}

TEST_F(ADCTest, PollTimeoutMapsToZeroReading) {
  float output = 1.0f;
  const std::array<ST_LIB::ADCDomain::Config, 1> cfgs{{
      {.gpio_idx = 0,
       .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
       .channel = ST_LIB::ADCDomain::Channel::CH16,
       .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
       .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
       .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
       .sample_rate_hz = 0,
       .output = &output},
  }};

  ST_LIB::ADCDomain::Init<1>::init(cfgs);
  ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 4095U);
  ST_LIB::MockedHAL::adc_set_poll_timeout(ADC1, true);

  auto &adc = ST_LIB::ADCDomain::Init<1>::instances[0];
  adc.read(3.3, 1);

  EXPECT_FLOAT_EQ(output, 0.0f);
}

TEST_F(ADCTest, MultiChannelInstancesReadTheirOwnChannel) {
  float out0 = -1.0f;
  float out1 = -1.0f;
  const std::array<ST_LIB::ADCDomain::Config, 2> cfgs{{
      {.gpio_idx = 0,
       .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
       .channel = ST_LIB::ADCDomain::Channel::CH16,
       .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
       .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
       .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
       .sample_rate_hz = 0,
       .output = &out0},
      {.gpio_idx = 1,
       .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
       .channel = ST_LIB::ADCDomain::Channel::CH15,
       .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
       .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
       .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
       .sample_rate_hz = 0,
       .output = &out1},
  }};

  ST_LIB::ADCDomain::Init<2>::init(cfgs);
  ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 1024U);
  ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_15, 3072U);

  auto &adc0 = ST_LIB::ADCDomain::Init<2>::instances[0];
  auto &adc1 = ST_LIB::ADCDomain::Init<2>::instances[1];
  adc0.read(3.3, 1);
  adc1.read(3.3, 1);

  const float expected0 = (1024.0f / 4095.0f) * 3.3f;
  const float expected1 = (3072.0f / 4095.0f) * 3.3f;
  EXPECT_NEAR(out0, expected0, 0.001f);
  EXPECT_NEAR(out1, expected1, 0.001f);
  EXPECT_EQ(ST_LIB::MockedHAL::adc_get_last_channel(ADC1), ADC_CHANNEL_15);
}

TEST_F(ADCTest, Resolution8BitUses8BitScaling) {
  float output = -1.0f;
  const std::array<ST_LIB::ADCDomain::Config, 1> cfgs{{
      {.gpio_idx = 0,
       .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
       .channel = ST_LIB::ADCDomain::Channel::CH16,
       .resolution = ST_LIB::ADCDomain::Resolution::BITS_8,
       .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
       .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
       .sample_rate_hz = 0,
       .output = &output},
  }};

  ST_LIB::ADCDomain::Init<1>::init(cfgs);
  ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 255U);

  auto &adc = ST_LIB::ADCDomain::Init<1>::instances[0];
  adc.read(3.3, 1);

  EXPECT_NEAR(output, 3.3f, 0.001f);
}

TEST_F(ADCTest, Resolution10BitClampsRawTo10BitRange) {
  float output = -1.0f;
  const std::array<ST_LIB::ADCDomain::Config, 1> cfgs{{
      {.gpio_idx = 0,
       .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
       .channel = ST_LIB::ADCDomain::Channel::CH16,
       .resolution = ST_LIB::ADCDomain::Resolution::BITS_10,
       .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
       .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
       .sample_rate_hz = 0,
       .output = &output},
  }};

  ST_LIB::ADCDomain::Init<1>::init(cfgs);
  ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 4095U);

  auto &adc = ST_LIB::ADCDomain::Init<1>::instances[0];
  adc.read(3.3, 1);

  EXPECT_NEAR(output, 3.3f, 0.001f);
}

TEST_F(ADCTest, TimeoutRecoversOnNextSuccessfulRead) {
  float output = -1.0f;
  const std::array<ST_LIB::ADCDomain::Config, 1> cfgs{{
      {.gpio_idx = 0,
       .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
       .channel = ST_LIB::ADCDomain::Channel::CH16,
       .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
       .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
       .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
       .sample_rate_hz = 0,
       .output = &output},
  }};

  ST_LIB::ADCDomain::Init<1>::init(cfgs);
  auto &adc = ST_LIB::ADCDomain::Init<1>::instances[0];

  ST_LIB::MockedHAL::adc_set_poll_timeout(ADC1, true);
  adc.read(3.3, 1);
  EXPECT_NE(output, -1.0f);
  EXPECT_NE((HAL_ADC_GetState(&hadc1) & HAL_ADC_STATE_TIMEOUT), 0U);

  ST_LIB::MockedHAL::adc_set_poll_timeout(ADC1, false);
  ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 1000U);
  adc.read(3.3, 1);
  EXPECT_NEAR(output, (1000.0f / 4095.0f) * 3.3f, 0.001f);
  EXPECT_EQ((HAL_ADC_GetState(&hadc1) & HAL_ADC_STATE_TIMEOUT), 0U);
}

TEST_F(ADCTest, SeparatePeripheralsAreIndependent) {
  float out1 = -1.0f;
  float out2 = -1.0f;
  const std::array<ST_LIB::ADCDomain::Config, 2> cfgs{{
      {.gpio_idx = 0,
       .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
       .channel = ST_LIB::ADCDomain::Channel::CH16,
       .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
       .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
       .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
       .sample_rate_hz = 0,
       .output = &out1},
      {.gpio_idx = 1,
       .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_2,
       .channel = ST_LIB::ADCDomain::Channel::CH2,
       .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
       .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
       .prescaler = ST_LIB::ADCDomain::ClockPrescaler::DIV1,
       .sample_rate_hz = 0,
       .output = &out2},
  }};

  ST_LIB::ADCDomain::Init<2>::init(cfgs);
  ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 500U);
  ST_LIB::MockedHAL::adc_set_channel_raw(ADC2, ADC_CHANNEL_2, 3000U);

  auto &adc1 = ST_LIB::ADCDomain::Init<2>::instances[0];
  auto &adc2 = ST_LIB::ADCDomain::Init<2>::instances[1];
  adc1.read(3.3, 1);
  adc2.read(3.3, 1);

  EXPECT_NEAR(out1, (500.0f / 4095.0f) * 3.3f, 0.001f);
  EXPECT_NEAR(out2, (3000.0f / 4095.0f) * 3.3f, 0.001f);
  EXPECT_EQ(ST_LIB::MockedHAL::adc_get_last_channel(ADC1), ADC_CHANNEL_16);
  EXPECT_EQ(ST_LIB::MockedHAL::adc_get_last_channel(ADC2), ADC_CHANNEL_2);
}
