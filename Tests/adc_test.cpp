#include <array>

#include <gtest/gtest.h>

#include "HALAL/Models/DMA/DMA2.hpp"
#include "HALAL/Services/ADC/ADC.hpp"
#include "MockedDrivers/NVIC.hpp"
#include "MockedDrivers/mocked_hal_adc.hpp"
#include "MockedDrivers/mocked_hal_dma.hpp"

namespace ST_LIB::TestPanicReporter {
void reset();
void set_fail_on_error(bool enabled);
extern int call_count;
} // namespace ST_LIB::TestPanicReporter

namespace {

template <std::size_t TotalN, std::size_t... ExtraNs>
consteval std::array<ST_LIB::DMADomain::Entry, TotalN> merge_dma_entries(
    std::span<const ST_LIB::DMADomain::Entry> base_entries,
    const std::array<ST_LIB::DMADomain::Entry, ExtraNs>&... extra_entries
) {
    std::array<ST_LIB::DMADomain::Entry, TotalN> merged{};
    std::size_t cursor = 0;

    for (const auto& entry : base_entries) {
        merged[cursor++] = entry;
    }

    auto append = [&]<std::size_t N>(const std::array<ST_LIB::DMADomain::Entry, N>& entries) {
        for (const auto& entry : entries) {
            merged[cursor++] = entry;
        }
    };
    (append(extra_entries), ...);

    return merged;
}

template <std::size_t TotalN, std::size_t... ExtraNs>
consteval std::array<ST_LIB::DMADomain::Config, TotalN> build_dma_cfgs(
    std::span<const ST_LIB::DMADomain::Entry> base_entries,
    const std::array<ST_LIB::DMADomain::Entry, ExtraNs>&... extra_entries
) {
    const auto merged = merge_dma_entries<TotalN>(base_entries, extra_entries...);
    return ST_LIB::DMADomain::build<TotalN>(std::span<const ST_LIB::DMADomain::Entry, TotalN>{merged
    });
}

inline float compile_time_output = 0.0f;
inline float synthesized_dma_output_0 = 0.0f;
inline float synthesized_dma_output_1 = 0.0f;
inline float adc_test_template_output_0 = 0.0f;
inline float adc_test_template_output_1 = 0.0f;

constexpr std::array<ST_LIB::ADCDomain::Entry, 1> auto_entry{{
    {.gpio_idx = 0,
     .pin = ST_LIB::PA0,
     .peripheral = ST_LIB::ADCDomain::Peripheral::AUTO,
     .channel = ST_LIB::ADCDomain::Channel::AUTO,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .output = &compile_time_output},
}};

constexpr auto auto_cfg =
    ST_LIB::ADCDomain::build<1>(std::span<const ST_LIB::ADCDomain::Entry, 1>{auto_entry});

static_assert(auto_cfg[0].peripheral == ST_LIB::ADCDomain::Peripheral::ADC_1);
static_assert(auto_cfg[0].channel == ST_LIB::ADCDomain::Channel::CH16);
static_assert(ST_LIB::ADCDomain::pin_map.size() == 42);

constexpr std::array<ST_LIB::ADCDomain::Entry, 1> auto_pf13_entry{{
    {.gpio_idx = 0,
     .pin = ST_LIB::PF13,
     .peripheral = ST_LIB::ADCDomain::Peripheral::AUTO,
     .channel = ST_LIB::ADCDomain::Channel::AUTO,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .output = &compile_time_output},
}};

constexpr auto auto_pf13_cfg =
    ST_LIB::ADCDomain::build<1>(std::span<const ST_LIB::ADCDomain::Entry, 1>{auto_pf13_entry});
static_assert(auto_pf13_cfg[0].peripheral == ST_LIB::ADCDomain::Peripheral::ADC_2);
static_assert(auto_pf13_cfg[0].channel == ST_LIB::ADCDomain::Channel::CH2);

constexpr std::array<ST_LIB::ADCDomain::Entry, 1> auto_pc0_16bit_entry{{
    {.gpio_idx = 0,
     .pin = ST_LIB::PC0,
     .peripheral = ST_LIB::ADCDomain::Peripheral::AUTO,
     .channel = ST_LIB::ADCDomain::Channel::AUTO,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_16,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .output = &compile_time_output},
}};

constexpr auto auto_pc0_16bit_cfg =
    ST_LIB::ADCDomain::build<1>(std::span<const ST_LIB::ADCDomain::Entry, 1>{auto_pc0_16bit_entry});
static_assert(auto_pc0_16bit_cfg[0].peripheral == ST_LIB::ADCDomain::Peripheral::ADC_1);
static_assert(auto_pc0_16bit_cfg[0].channel == ST_LIB::ADCDomain::Channel::CH10);

constexpr std::array<ST_LIB::ADCDomain::Entry, 0> no_adc_entries{};
constexpr std::array<ST_LIB::DMADomain::Entry, 2> preexisting_spi_dma_entries{{
    {.instance = ST_LIB::DMADomain::Peripheral::spi2,
     .stream = ST_LIB::DMADomain::Stream::dma1_stream6,
     .irqn = DMA1_Stream6_IRQn,
     .id = 0},
    {.instance = ST_LIB::DMADomain::Peripheral::spi2,
     .stream = ST_LIB::DMADomain::Stream::dma2_stream3,
     .irqn = DMA2_Stream3_IRQn,
     .id = 1},
}};
constexpr std::array<ST_LIB::ADCDomain::Config, 0> no_adc_cfgs{};
static_assert(
    ST_LIB::ADCDomain::dma_contribution_count(
        std::span<const ST_LIB::ADCDomain::Config, 0>{no_adc_cfgs},
        std::span<const ST_LIB::DMADomain::Entry, 2>{preexisting_spi_dma_entries}
    ) == 0
);
constexpr auto passthrough_dma_cfg =
    build_dma_cfgs<2>(std::span<const ST_LIB::DMADomain::Entry, 2>{preexisting_spi_dma_entries});
static_assert(std::get<1>(passthrough_dma_cfg[0].init_data).Request == DMA_REQUEST_SPI2_RX);
static_assert(std::get<1>(passthrough_dma_cfg[1].init_data).Request == DMA_REQUEST_SPI2_TX);
static_assert(
    std::get<2>(passthrough_dma_cfg[0].init_data) == ST_LIB::DMADomain::Stream::dma1_stream6
);
static_assert(
    std::get<2>(passthrough_dma_cfg[1].init_data) == ST_LIB::DMADomain::Stream::dma2_stream3
);

constexpr std::array<ST_LIB::ADCDomain::Config, 2> shared_adc_cfgs{{
    {.gpio_idx = 0,
     .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
     .channel = ST_LIB::ADCDomain::Channel::CH16,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .dma_request = DMA_REQUEST_ADC1,
     .output = &synthesized_dma_output_0},
    {.gpio_idx = 1,
     .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
     .channel = ST_LIB::ADCDomain::Channel::CH15,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .dma_request = DMA_REQUEST_ADC1,
     .output = &synthesized_dma_output_1},
}};

constexpr std::array<ST_LIB::DMADomain::Entry, 0> no_adc_dma_entries{};
static_assert(
    ST_LIB::ADCDomain::dma_contribution_count(
        std::span<const ST_LIB::ADCDomain::Config, 2>{shared_adc_cfgs},
        std::span<const ST_LIB::DMADomain::Entry, 0>{no_adc_dma_entries}
    ) == 1
);
constexpr auto synthesized_shared_adc_dma_entries = ST_LIB::ADCDomain::build_dma_contributions<1>(
    std::span<const ST_LIB::DMADomain::Entry, 0>{no_adc_dma_entries},
    std::span<const ST_LIB::ADCDomain::Config, 2>{shared_adc_cfgs}
);
constexpr auto synthesized_shared_adc_dma_cfg = build_dma_cfgs<1>(
    std::span<const ST_LIB::DMADomain::Entry, 0>{no_adc_dma_entries},
    synthesized_shared_adc_dma_entries
);
static_assert(std::get<1>(synthesized_shared_adc_dma_cfg[0].init_data).Request == DMA_REQUEST_ADC1);
static_assert(
    std::get<2>(synthesized_shared_adc_dma_cfg[0].init_data) ==
    ST_LIB::DMADomain::Stream::dma1_stream0
);
static_assert(!std::get<5>(synthesized_shared_adc_dma_cfg[0].init_data));

constexpr std::array<ST_LIB::DMADomain::Entry, 1> explicit_adc1_dma_entries{{
    {.instance = ST_LIB::DMADomain::Peripheral::adc1,
     .stream = ST_LIB::DMADomain::Stream::dma1_stream5,
     .irqn = DMA1_Stream5_IRQn,
     .id = 0},
}};

static_assert(
    ST_LIB::ADCDomain::dma_contribution_count(
        std::span<const ST_LIB::ADCDomain::Config, 2>{shared_adc_cfgs},
        std::span<const ST_LIB::DMADomain::Entry, 1>{explicit_adc1_dma_entries}
    ) == 0
);
constexpr auto explicit_shared_adc_dma_cfg =
    build_dma_cfgs<1>(std::span<const ST_LIB::DMADomain::Entry, 1>{explicit_adc1_dma_entries});
static_assert(
    std::get<2>(explicit_shared_adc_dma_cfg[0].init_data) == ST_LIB::DMADomain::Stream::dma1_stream5
);
static_assert(
    ST_LIB::ADCDomain::dma_contribution_count(
        std::span<const ST_LIB::ADCDomain::Config, 2>{shared_adc_cfgs},
        std::span<const ST_LIB::DMADomain::Entry, 2>{preexisting_spi_dma_entries}
    ) == 1
);
constexpr auto merged_dma_entries = ST_LIB::ADCDomain::build_dma_contributions<1>(
    std::span<const ST_LIB::DMADomain::Entry, 2>{preexisting_spi_dma_entries},
    std::span<const ST_LIB::ADCDomain::Config, 2>{shared_adc_cfgs}
);
constexpr auto merged_dma_cfg = build_dma_cfgs<3>(
    std::span<const ST_LIB::DMADomain::Entry, 2>{preexisting_spi_dma_entries},
    merged_dma_entries
);
static_assert(std::get<1>(merged_dma_cfg[0].init_data).Request == DMA_REQUEST_SPI2_RX);
static_assert(std::get<1>(merged_dma_cfg[1].init_data).Request == DMA_REQUEST_SPI2_TX);
static_assert(std::get<1>(merged_dma_cfg[2].init_data).Request == DMA_REQUEST_ADC1);

constexpr std::array<ST_LIB::DMADomain::Entry, 3> adc_dma_entries{{
    {.instance = ST_LIB::DMADomain::Peripheral::adc1,
     .stream = ST_LIB::DMADomain::Stream::dma1_stream0,
     .irqn = DMA1_Stream0_IRQn,
     .id = 0},
    {.instance = ST_LIB::DMADomain::Peripheral::adc2,
     .stream = ST_LIB::DMADomain::Stream::dma1_stream1,
     .irqn = DMA1_Stream1_IRQn,
     .id = 0},
    {.instance = ST_LIB::DMADomain::Peripheral::adc3,
     .stream = ST_LIB::DMADomain::Stream::dma1_stream2,
     .irqn = DMA1_Stream2_IRQn,
     .id = 0},
}};

constexpr auto adc_dma_cfg =
    ST_LIB::DMADomain::build<3>(std::span<const ST_LIB::DMADomain::Entry, 3>{adc_dma_entries});
static_assert(std::get<1>(adc_dma_cfg[0].init_data).Request == DMA_REQUEST_ADC1);
static_assert(std::get<1>(adc_dma_cfg[1].init_data).Request == DMA_REQUEST_ADC2);
static_assert(std::get<1>(adc_dma_cfg[2].init_data).Request == DMA_REQUEST_ADC3);
static_assert(std::get<1>(adc_dma_cfg[0].init_data).Mode == DMA_CIRCULAR);
static_assert(std::get<1>(adc_dma_cfg[0].init_data).PeriphDataAlignment == DMA_PDATAALIGN_HALFWORD);
static_assert(!std::get<5>(adc_dma_cfg[0].init_data));
static_assert(!std::get<5>(adc_dma_cfg[1].init_data));
static_assert(!std::get<5>(adc_dma_cfg[2].init_data));
static_assert(std::get<1>(adc_dma_cfg[0].init_data).MemDataAlignment == DMA_MDATAALIGN_HALFWORD);

constexpr std::array<ST_LIB::ADCDomain::Config, 1> single_adc1_init_cfgs{{
    {.gpio_idx = 0,
     .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
     .channel = ST_LIB::ADCDomain::Channel::CH16,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .dma_request = DMA_REQUEST_ADC1,
     .output = &adc_test_template_output_0},
}};

constexpr std::array<ST_LIB::ADCDomain::Config, 2> shared_adc1_init_cfgs{{
    {.gpio_idx = 0,
     .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
     .channel = ST_LIB::ADCDomain::Channel::CH16,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .dma_request = DMA_REQUEST_ADC1,
     .output = &adc_test_template_output_0},
    {.gpio_idx = 1,
     .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
     .channel = ST_LIB::ADCDomain::Channel::CH15,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .dma_request = DMA_REQUEST_ADC1,
     .output = &adc_test_template_output_1},
}};

constexpr std::array<ST_LIB::ADCDomain::Config, 2> split_adc12_init_cfgs{{
    {.gpio_idx = 0,
     .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
     .channel = ST_LIB::ADCDomain::Channel::CH16,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .dma_request = DMA_REQUEST_ADC1,
     .output = &adc_test_template_output_0},
    {.gpio_idx = 1,
     .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_2,
     .channel = ST_LIB::ADCDomain::Channel::CH2,
     .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
     .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
     .sample_rate_hz = 0,
     .dma_request = DMA_REQUEST_ADC2,
     .output = &adc_test_template_output_1},
}};

template <std::size_t N, const std::array<ST_LIB::ADCDomain::Config, N>& Cfgs>
using ADCInit = ST_LIB::ADCDomain::Init<N, Cfgs>;

using SingleADCInit = ADCInit<1, single_adc1_init_cfgs>;
using SharedADCInit = ADCInit<2, shared_adc1_init_cfgs>;
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

} // namespace

class ADCTest : public ::testing::Test {
protected:
    void reset_runtime_state() {
        ST_LIB::MockedHAL::adc_reset();
        ST_LIB::MockedHAL::dma_reset();
        ST_LIB::TestPanicReporter::reset();
        clear_nvic_enables();
        clear_dma_irq_table();
    }

    void SetUp() override {
        reset_runtime_state();
        ST_LIB::ClockDomain::s_tree = &ST_LIB::default_clock_tree;
    }

    template <std::size_t N, const std::array<ST_LIB::ADCDomain::Config, N>& InitCfgs>
    void init_adc_with_dma(const std::array<ST_LIB::ADCDomain::Config, N>& cfgs) {
        ST_LIB::DMADomain::Init<3>::init(adc_dma_cfg);
        ADCInit<N, InitCfgs>::init(
            cfgs,
            std::span<ST_LIB::GPIODomain::Instance>{},
            std::span<ST_LIB::DMADomain::Instance>(ST_LIB::DMADomain::Init<3>::instances)
        );
    }

    void advance_time_to(uint64_t target_time_ns) {
        const uint64_t now = ST_LIB::MockedHAL::adc_get_time_ns();
        if (target_time_ns > now) {
            ST_LIB::MockedHAL::adc_advance_time_ns(target_time_ns - now);
        }
    }
};

TEST_F(ADCTest, BuildTimeDMASynthesisCreatesSingleDMAForSharedADCPeripheral) {
    constexpr std::size_t dmaN = std::tuple_size_v<decltype(synthesized_shared_adc_dma_cfg)>;
    constexpr std::size_t adcN = std::tuple_size_v<decltype(shared_adc_cfgs)>;

    ST_LIB::DMADomain::Init<dmaN>::init(synthesized_shared_adc_dma_cfg);
    ADCInit<adcN, shared_adc_cfgs>::init(
        shared_adc_cfgs,
        std::span<ST_LIB::GPIODomain::Instance>{},
        std::span<ST_LIB::DMADomain::Instance>(ST_LIB::DMADomain::Init<dmaN>::instances)
    );

    EXPECT_EQ(ST_LIB::MockedHAL::dma_get_call_count(ST_LIB::MockedHAL::DMAOperation::Init), 1U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_call_count(ST_LIB::MockedHAL::ADCOperation::StartDMA), 1U);
    EXPECT_EQ(hadc1.DMA_Handle, &ST_LIB::DMADomain::Init<dmaN>::instances[0].dma);
    EXPECT_EQ(hadc1.Init.NbrOfConversion, 2U);
}

TEST_F(ADCTest, InitWithExternalDMAStartsCircularTransferAndLinksHandle) {
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

    EXPECT_EQ(hadc1.Instance, ADC1);
    EXPECT_EQ(hadc1.Init.ConversionDataManagement, ADC_CONVERSIONDATA_DMA_CIRCULAR);
    EXPECT_EQ(hadc1.Init.NbrOfConversion, 1U);
    EXPECT_EQ(hadc1.DMA_Handle, &ST_LIB::DMADomain::Init<3>::instances[0].dma);
    ASSERT_NE(hadc1.DMA_Handle, nullptr);
    EXPECT_EQ(hadc1.DMA_Handle->Parent, &hadc1);
    EXPECT_EQ(NVIC_GetEnableIRQ(DMA1_Stream0_IRQn), 0U);

    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_call_count(ST_LIB::MockedHAL::ADCOperation::StartDMA), 1U);
    EXPECT_EQ(
        ST_LIB::MockedHAL::adc_get_call_count(ST_LIB::MockedHAL::ADCOperation::PollForConversion),
        0U
    );
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_rank_count(ADC1), 1U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_rank_channel(ADC1, 0), ADC_CHANNEL_16);
    EXPECT_EQ(ST_LIB::MockedHAL::dma_get_call_count(ST_LIB::MockedHAL::DMAOperation::StartIT), 1U);
    EXPECT_TRUE(ST_LIB::MockedHAL::adc_is_dma_running(ADC1));
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_length(ADC1), 1U);
}

TEST_F(ADCTest, ReadUsesLatestDMABufferValueWithoutPolling) {
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

    auto& adc = SingleADCInit::instances[0];
    adc.read(3.3f, 1U);

    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC1, 0), 2048U);
    EXPECT_NEAR(output, (2048.0f / 4095.0f) * 3.3f, 0.001f);
    EXPECT_EQ(
        ST_LIB::MockedHAL::adc_get_call_count(ST_LIB::MockedHAL::ADCOperation::PollForConversion),
        0U
    );

    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 1024U);
    adc.read(3.3f, 1U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC1, 0), 1024U);
    EXPECT_NEAR(output, (1024.0f / 4095.0f) * 3.3f, 0.001f);
}

TEST_F(ADCTest, MultiChannelDMAUsesSequenceSlotsPerPeripheral) {
    float out0 = -1.0f;
    float out1 = -1.0f;
    const std::array<ST_LIB::ADCDomain::Config, 2> cfgs{{
        {.gpio_idx = 0,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH16,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &out0},
        {.gpio_idx = 1,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH15,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &out1},
    }};

    init_adc_with_dma<2, shared_adc1_init_cfgs>(cfgs);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 1024U);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_15, 3072U);

    auto& adc0 = SharedADCInit::instances[0];
    auto& adc1 = SharedADCInit::instances[1];
    adc0.read(3.3f, 1U);
    adc1.read(3.3f, 1U);

    EXPECT_EQ(hadc1.Init.ScanConvMode, ADC_SCAN_ENABLE);
    EXPECT_EQ(hadc1.Init.EOCSelection, ADC_EOC_SEQ_CONV);
    EXPECT_EQ(hadc1.Init.NbrOfConversion, 2U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_rank_count(ADC1), 2U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_rank_channel(ADC1, 0), ADC_CHANNEL_16);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_rank_channel(ADC1, 1), ADC_CHANNEL_15);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_length(ADC1), 2U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC1, 0), 1024U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC1, 1), 3072U);
    EXPECT_NEAR(out0, (1024.0f / 4095.0f) * 3.3f, 0.001f);
    EXPECT_NEAR(out1, (3072.0f / 4095.0f) * 3.3f, 0.001f);
}

TEST_F(ADCTest, SeparatePeripheralsUseIndependentDMAHandlesAndBuffers) {
    float out1 = -1.0f;
    float out2 = -1.0f;
    const std::array<ST_LIB::ADCDomain::Config, 2> cfgs{{
        {.gpio_idx = 0,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH16,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &out1},
        {.gpio_idx = 1,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_2,
         .channel = ST_LIB::ADCDomain::Channel::CH2,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC2,
         .output = &out2},
    }};

    init_adc_with_dma<2, split_adc12_init_cfgs>(cfgs);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 500U);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC2, ADC_CHANNEL_2, 3000U);

    auto& adc1 = SplitADCInit::instances[0];
    auto& adc2 = SplitADCInit::instances[1];
    adc1.read(3.3f, 1U);
    adc2.read(3.3f, 1U);

    EXPECT_EQ(hadc1.DMA_Handle, &ST_LIB::DMADomain::Init<3>::instances[0].dma);
    EXPECT_EQ(hadc2.DMA_Handle, &ST_LIB::DMADomain::Init<3>::instances[1].dma);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC1, 0), 500U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC2, 0), 3000U);
    EXPECT_NEAR(out1, (500.0f / 4095.0f) * 3.3f, 0.001f);
    EXPECT_NEAR(out2, (3000.0f / 4095.0f) * 3.3f, 0.001f);
}

TEST_F(ADCTest, Resolution10BitDMAClampsRawBufferToResolutionRange) {
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
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 4095U);

    auto& adc = SingleADCInit::instances[0];
    adc.read(3.3f, 1U);

    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC1, 0), 1023U);
    EXPECT_NEAR(output, 3.3f, 0.001f);
}

TEST_F(ADCTest, InitWithoutDMAInstancesFailsInsteadOfConfiguringDMAAtRuntime) {
    ST_LIB::TestPanicReporter::set_fail_on_error(false);

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

    SingleADCInit::init(cfgs);

    EXPECT_EQ(ST_LIB::TestPanicReporter::call_count, 1);
    EXPECT_EQ(hadc1.DMA_Handle, nullptr);
    EXPECT_FALSE(ST_LIB::MockedHAL::adc_is_dma_running(ADC1));
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_call_count(ST_LIB::MockedHAL::ADCOperation::StartDMA), 0U);
}

TEST_F(ADCTest, DMAStartFailureTriggersErrorPathAndLeavesInstanceUnreadable) {
    ST_LIB::TestPanicReporter::set_fail_on_error(false);
    ST_LIB::MockedHAL::dma_set_start_status(HAL_ERROR);

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

    EXPECT_EQ(ST_LIB::TestPanicReporter::call_count, 1);
    EXPECT_FALSE(ST_LIB::MockedHAL::adc_is_dma_running(ADC1));
    EXPECT_EQ(SingleADCInit::instances[0].handle, nullptr);
    EXPECT_FLOAT_EQ(SingleADCInit::instances[0].get_raw(), 0.0f);
}

TEST_F(ADCTest, UnresolvedConfigDoesNotAliasAResolvedPeripheralInstance) {
    ST_LIB::TestPanicReporter::set_fail_on_error(false);

    float unresolved = -1.0f;
    float resolved = -1.0f;
    const std::array<ST_LIB::ADCDomain::Config, 2> cfgs{{
        {.gpio_idx = 0,
         .peripheral = ST_LIB::ADCDomain::Peripheral::AUTO,
         .channel = ST_LIB::ADCDomain::Channel::AUTO,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &unresolved},
        {.gpio_idx = 1,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH16,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &resolved},
    }};

    init_adc_with_dma<2, shared_adc1_init_cfgs>(cfgs);

    EXPECT_EQ(ST_LIB::TestPanicReporter::call_count, 1);
    EXPECT_EQ(SharedADCInit::instances[0].handle, nullptr);
    EXPECT_EQ(SharedADCInit::instances[0].dma_slot, nullptr);
    EXPECT_EQ(SharedADCInit::instances[1].handle, &hadc1);
    EXPECT_NE(SharedADCInit::instances[1].dma_slot, nullptr);
}

TEST_F(ADCTest, TimedDMAWaitsForSequenceAndTransferCompletionBeforeUpdatingBuffer) {
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

    auto adc_test_tree = *ST_LIB::ClockDomain::s_tree;
    adc_test_tree.adc_src = ST_LIB::ClockDomain::ClockTree::Source::HSI;
    ST_LIB::ClockDomain::s_tree = &adc_test_tree;

    ST_LIB::MockedHAL::adc_enable_timed_dma(ADC1, true);
    ST_LIB::MockedHAL::adc_set_kernel_clock_hz(ADC1, 64'000'000ULL);
    ST_LIB::MockedHAL::dma_set_transfer_timing(50ULL, 25ULL);
    ST_LIB::MockedHAL::adc_set_channel_generator(ADC1, ADC_CHANNEL_16, [](uint64_t time_ns) {
        return (time_ns < 1'000ULL) ? 321U : 654U;
    });

    init_adc_with_dma<1, single_adc1_init_cfgs>(cfgs);

    ST_LIB::ClockDomain::s_tree = &ST_LIB::default_clock_tree;

    auto& adc = SingleADCInit::instances[0];
    const uint64_t sequence_period_ns = ST_LIB::MockedHAL::adc_get_sequence_period_ns(ADC1);
    ASSERT_GT(sequence_period_ns, 0U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC1, 0), 0U);

    advance_time_to(sequence_period_ns - 1ULL);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC1, 0), 0U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_completed_sequence_count(ADC1), 0U);

    advance_time_to(sequence_period_ns);
    const uint64_t completion_time_ns =
        ST_LIB::MockedHAL::adc_get_pending_dma_completion_time_ns(ADC1);
    ASSERT_GT(completion_time_ns, sequence_period_ns);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC1, 0), 0U);

    advance_time_to(completion_time_ns);
    adc.read(3.3f, 1U);

    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_dma_value(ADC1, 0), 321U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_completed_sequence_count(ADC1), 1U);
    EXPECT_EQ(ST_LIB::MockedHAL::adc_get_overrun_count(ADC1), 0U);
    EXPECT_NEAR(output, (321.0f / 4095.0f) * 3.3f, 0.001f);
}

TEST_F(ADCTest, TimedDMADetectsOverrunWhenTransferCannotKeepUp) {
    float out0 = -1.0f;
    float out1 = -1.0f;
    const std::array<ST_LIB::ADCDomain::Config, 2> cfgs{{
        {.gpio_idx = 0,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH16,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_16,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_1_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &out0},
        {.gpio_idx = 1,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH15,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_16,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_1_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &out1},
    }};

    ST_LIB::MockedHAL::adc_enable_timed_dma(ADC1, true);
    ST_LIB::MockedHAL::adc_set_kernel_clock_hz(ADC1, 64'000'000ULL);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 1'000U);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_15, 2'000U);

    init_adc_with_dma<2, shared_adc1_init_cfgs>(cfgs);

    const uint64_t sequence_period_ns = ST_LIB::MockedHAL::adc_get_sequence_period_ns(ADC1);
    ASSERT_GT(sequence_period_ns, 0U);

    ST_LIB::MockedHAL::dma_set_transfer_timing(sequence_period_ns * 2ULL, 0ULL);
    ST_LIB::MockedHAL::adc_advance_time_ns(sequence_period_ns * 12ULL);

    EXPECT_LT(ST_LIB::MockedHAL::adc_get_completed_sequence_count(ADC1), 12U);
    EXPECT_GT(ST_LIB::MockedHAL::adc_get_overrun_count(ADC1), 0U);
    EXPECT_EQ(ST_LIB::MockedHAL::dma_get_last_scheduled_handle(), hadc1.DMA_Handle);
    EXPECT_EQ(ST_LIB::MockedHAL::dma_get_last_schedule_bytes(), 4U);
}

TEST_F(ADCTest, TimedDMASharedBusContentionShowsUpWithSimultaneousADCs) {
    float out1 = -1.0f;
    float out2 = -1.0f;
    const std::array<ST_LIB::ADCDomain::Config, 2> cfgs{{
        {.gpio_idx = 0,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
         .channel = ST_LIB::ADCDomain::Channel::CH16,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_1_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC1,
         .output = &out1},
        {.gpio_idx = 1,
         .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_2,
         .channel = ST_LIB::ADCDomain::Channel::CH2,
         .resolution = ST_LIB::ADCDomain::Resolution::BITS_12,
         .sample_time = ST_LIB::ADCDomain::SampleTime::CYCLES_1_5,
         .sample_rate_hz = 0,
         .dma_request = DMA_REQUEST_ADC2,
         .output = &out2},
    }};

    ST_LIB::MockedHAL::adc_enable_timed_dma(ADC1, true);
    ST_LIB::MockedHAL::adc_enable_timed_dma(ADC2, true);
    ST_LIB::MockedHAL::adc_set_kernel_clock_hz(ADC1, 64'000'000ULL);
    ST_LIB::MockedHAL::adc_set_kernel_clock_hz(ADC2, 64'000'000ULL);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 1'111U);
    ST_LIB::MockedHAL::adc_set_channel_raw(ADC2, ADC_CHANNEL_2, 2'222U);

    init_adc_with_dma<2, split_adc12_init_cfgs>(cfgs);

    const uint64_t sequence_period_ns = ST_LIB::MockedHAL::adc_get_sequence_period_ns(ADC1);
    ASSERT_EQ(sequence_period_ns, ST_LIB::MockedHAL::adc_get_sequence_period_ns(ADC2));
    ASSERT_GT(sequence_period_ns, 0U);

    ST_LIB::MockedHAL::dma_set_transfer_timing(sequence_period_ns * 3ULL / 4ULL, 0ULL);
    ST_LIB::MockedHAL::adc_advance_time_ns(sequence_period_ns * 10ULL);

    const uint64_t total_completed = ST_LIB::MockedHAL::adc_get_completed_sequence_count(ADC1) +
                                     ST_LIB::MockedHAL::adc_get_completed_sequence_count(ADC2);
    const uint64_t total_overruns = ST_LIB::MockedHAL::adc_get_overrun_count(ADC1) +
                                    ST_LIB::MockedHAL::adc_get_overrun_count(ADC2);

    EXPECT_LT(total_completed, 20U);
    EXPECT_GT(total_overruns, 0U);
}

TEST_F(ADCTest, TimedDMAFrequencySweepSeparatesStableAndUnstableOperatingRegions) {
    constexpr std::array resolutions{
        ST_LIB::ADCDomain::Resolution::BITS_16,
        ST_LIB::ADCDomain::Resolution::BITS_12,
        ST_LIB::ADCDomain::Resolution::BITS_10,
        ST_LIB::ADCDomain::Resolution::BITS_8,
    };
    constexpr std::array sample_times{
        ST_LIB::ADCDomain::SampleTime::CYCLES_1_5,
        ST_LIB::ADCDomain::SampleTime::CYCLES_8_5,
        ST_LIB::ADCDomain::SampleTime::CYCLES_32_5,
        ST_LIB::ADCDomain::SampleTime::CYCLES_387_5,
    };

    for (const auto resolution : resolutions) {
        for (const auto sample_time : sample_times) {
            SCOPED_TRACE(static_cast<int>(resolution));
            SCOPED_TRACE(static_cast<int>(sample_time));

            reset_runtime_state();

            float output = -1.0f;
            const std::array<ST_LIB::ADCDomain::Config, 1> cfgs{{
                {.gpio_idx = 0,
                 .peripheral = ST_LIB::ADCDomain::Peripheral::ADC_1,
                 .channel = ST_LIB::ADCDomain::Channel::CH16,
                 .resolution = resolution,
                 .sample_time = sample_time,
                 .sample_rate_hz = 0,
                 .dma_request = DMA_REQUEST_ADC1,
                 .output = &output},
            }};

            ST_LIB::MockedHAL::adc_enable_timed_dma(ADC1, true);
            ST_LIB::MockedHAL::adc_set_kernel_clock_hz(ADC1, 64'000'000ULL);
            ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 777U);
            init_adc_with_dma<1, single_adc1_init_cfgs>(cfgs);

            const uint64_t sequence_period_ns = ST_LIB::MockedHAL::adc_get_sequence_period_ns(ADC1);
            ASSERT_GT(sequence_period_ns, 0U);

            ST_LIB::MockedHAL::dma_set_transfer_timing(0ULL, 0ULL);
            ST_LIB::MockedHAL::adc_advance_time_ns(sequence_period_ns * 8ULL);
            EXPECT_EQ(ST_LIB::MockedHAL::adc_get_overrun_count(ADC1), 0U);
            EXPECT_EQ(ST_LIB::MockedHAL::adc_get_completed_sequence_count(ADC1), 8U);

            reset_runtime_state();

            ST_LIB::MockedHAL::adc_enable_timed_dma(ADC1, true);
            ST_LIB::MockedHAL::adc_set_kernel_clock_hz(ADC1, 64'000'000ULL);
            ST_LIB::MockedHAL::adc_set_channel_raw(ADC1, ADC_CHANNEL_16, 777U);
            init_adc_with_dma<1, single_adc1_init_cfgs>(cfgs);

            const uint64_t unstable_period_ns = ST_LIB::MockedHAL::adc_get_sequence_period_ns(ADC1);
            ST_LIB::MockedHAL::dma_set_transfer_timing(unstable_period_ns * 2ULL, 0ULL);
            ST_LIB::MockedHAL::adc_advance_time_ns(unstable_period_ns * 8ULL);
            EXPECT_GT(ST_LIB::MockedHAL::adc_get_overrun_count(ADC1), 0U);
            EXPECT_LT(ST_LIB::MockedHAL::adc_get_completed_sequence_count(ADC1), 8U);
        }
    }
}
