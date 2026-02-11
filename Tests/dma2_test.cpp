#include <array>
#include <tuple>

#include <gtest/gtest.h>

#include "HALAL/Models/DMA/DMA2.hpp"
#include "MockedDrivers/NVIC.hpp"
#include "MockedDrivers/mocked_hal_dma.hpp"

extern "C" {
void DMA1_Stream0_IRQHandler(void);
void DMA2_Stream7_IRQHandler(void);
}

namespace {

constexpr std::array<ST_LIB::DMA_Domain::Entry, 2> spi_dma_entries{{
    {.instance = ST_LIB::DMA_Domain::Peripheral::spi2,
     .stream = ST_LIB::DMA_Domain::Stream::dma1_stream0,
     .irqn = DMA1_Stream0_IRQn,
     .id = 0},
    {.instance = ST_LIB::DMA_Domain::Peripheral::spi2,
     .stream = ST_LIB::DMA_Domain::Stream::dma1_stream1,
     .irqn = DMA1_Stream1_IRQn,
     .id = 1},
}};

constexpr auto spi_dma_cfg = ST_LIB::DMA_Domain::build<2>(
    std::span<const ST_LIB::DMA_Domain::Entry, 2>{spi_dma_entries});

static_assert(std::get<1>(spi_dma_cfg[0].init_data).Request == DMA_REQUEST_SPI2_RX);
static_assert(std::get<1>(spi_dma_cfg[1].init_data).Request == DMA_REQUEST_SPI2_TX);
static_assert(std::get<1>(spi_dma_cfg[0].init_data).Direction == DMA_PERIPH_TO_MEMORY);
static_assert(std::get<1>(spi_dma_cfg[1].init_data).Direction == DMA_MEMORY_TO_PERIPH);
static_assert(std::get<1>(spi_dma_cfg[0].init_data).FIFOThreshold ==
              DMA_FIFO_THRESHOLD_FULL);
static_assert(std::get<1>(spi_dma_cfg[1].init_data).FIFOThreshold ==
              DMA_FIFO_THRESHOLD_FULL);
static_assert(std::get<3>(spi_dma_cfg[0].init_data) == DMA1_Stream0_IRQn);
static_assert(std::get<3>(spi_dma_cfg[1].init_data) == DMA1_Stream1_IRQn);

constexpr std::array<ST_LIB::DMA_Domain::Entry, 2> auto_stream_entries{{
    {.instance = ST_LIB::DMA_Domain::Peripheral::adc1,
     .stream = ST_LIB::DMA_Domain::Stream::none,
     .irqn = 0,
     .id = 0},
    {.instance = ST_LIB::DMA_Domain::Peripheral::adc2,
     .stream = ST_LIB::DMA_Domain::Stream::none,
     .irqn = 0,
     .id = 0},
}};

constexpr auto auto_stream_cfg = ST_LIB::DMA_Domain::build<2>(
    std::span<const ST_LIB::DMA_Domain::Entry, 2>{auto_stream_entries});
static_assert(std::get<2>(auto_stream_cfg[0].init_data) ==
              ST_LIB::DMA_Domain::Stream::dma1_stream0);
static_assert(std::get<2>(auto_stream_cfg[1].init_data) ==
              ST_LIB::DMA_Domain::Stream::dma1_stream1);
static_assert(std::get<3>(auto_stream_cfg[0].init_data) == DMA1_Stream0_IRQn);
static_assert(std::get<3>(auto_stream_cfg[1].init_data) == DMA1_Stream1_IRQn);

constexpr std::array<ST_LIB::DMA_Domain::Entry, 3> fmac_entries{{
    {.instance = ST_LIB::DMA_Domain::Peripheral::fmac,
     .stream = ST_LIB::DMA_Domain::Stream::dma2_stream0,
     .irqn = DMA2_Stream0_IRQn,
     .id = 0},
    {.instance = ST_LIB::DMA_Domain::Peripheral::fmac,
     .stream = ST_LIB::DMA_Domain::Stream::dma2_stream1,
     .irqn = DMA2_Stream1_IRQn,
     .id = 1},
    {.instance = ST_LIB::DMA_Domain::Peripheral::fmac,
     .stream = ST_LIB::DMA_Domain::Stream::dma2_stream2,
     .irqn = DMA2_Stream2_IRQn,
     .id = 2},
}};

constexpr auto fmac_cfg = ST_LIB::DMA_Domain::build<3>(
    std::span<const ST_LIB::DMA_Domain::Entry, 3>{fmac_entries});
static_assert(std::get<1>(fmac_cfg[0].init_data).Direction == DMA_MEMORY_TO_MEMORY);
static_assert(std::get<1>(fmac_cfg[1].init_data).Direction == DMA_MEMORY_TO_PERIPH);
static_assert(std::get<1>(fmac_cfg[2].init_data).Direction == DMA_PERIPH_TO_MEMORY);
static_assert(std::get<1>(fmac_cfg[0].init_data).Request == DMA_REQUEST_MEM2MEM);
static_assert(std::get<1>(fmac_cfg[1].init_data).Request == DMA_REQUEST_FMAC_WRITE);
static_assert(std::get<1>(fmac_cfg[2].init_data).Request == DMA_REQUEST_FMAC_READ);

constexpr std::array<ST_LIB::DMA_Domain::Entry, 2> irq_entries{{
    {.instance = ST_LIB::DMA_Domain::Peripheral::spi2,
     .stream = ST_LIB::DMA_Domain::Stream::dma1_stream0,
     .irqn = DMA1_Stream0_IRQn,
     .id = 0},
    {.instance = ST_LIB::DMA_Domain::Peripheral::spi2,
     .stream = ST_LIB::DMA_Domain::Stream::dma2_stream7,
     .irqn = DMA2_Stream7_IRQn,
     .id = 1},
}};

constexpr auto irq_cfg = ST_LIB::DMA_Domain::build<2>(
    std::span<const ST_LIB::DMA_Domain::Entry, 2>{irq_entries});

void clear_nvic_enables() {
  for (auto &reg : NVIC->ISER) {
    reg = 0U;
  }
}

void clear_dma_irq_table() {
  for (auto &slot : dma_irq_table) {
    slot = nullptr;
  }
}

} // namespace

class DMA2Test : public ::testing::Test {
protected:
  void SetUp() override {
    ST_LIB::MockedHAL::dma_reset();
    clear_nvic_enables();
    clear_dma_irq_table();
  }
};

TEST_F(DMA2Test, InitConfiguresStreamsNVICAndLookupTable) {
  ST_LIB::DMA_Domain::Init<2>::init(spi_dma_cfg);

  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_call_count(ST_LIB::MockedHAL::DMAOperation::Init), 2U);

  auto &dma0 = ST_LIB::DMA_Domain::Init<2>::instances[0].dma;
  auto &dma1 = ST_LIB::DMA_Domain::Init<2>::instances[1].dma;
  EXPECT_EQ(dma0.Instance, DMA1_Stream0);
  EXPECT_EQ(dma1.Instance, DMA1_Stream1);

  EXPECT_EQ(dma0.Init.Request, DMA_REQUEST_SPI2_RX);
  EXPECT_EQ(dma1.Init.Request, DMA_REQUEST_SPI2_TX);
  EXPECT_EQ(dma0.Init.Direction, DMA_PERIPH_TO_MEMORY);
  EXPECT_EQ(dma1.Init.Direction, DMA_MEMORY_TO_PERIPH);

  EXPECT_EQ(dma_irq_table[0], &dma0);
  EXPECT_EQ(dma_irq_table[1], &dma1);
}

TEST_F(DMA2Test, StartForwardsTransferParametersToHALDMA) {
  ST_LIB::DMA_Domain::Init<2>::init(spi_dma_cfg);
  auto &instance = ST_LIB::DMA_Domain::Init<2>::instances[0];

  instance.start(0x1111U, 0x2222U, 128U);

  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_call_count(ST_LIB::MockedHAL::DMAOperation::StartIT),
            1U);
  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_last_start_handle(), &instance.dma);
  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_last_start_src(), 0x1111U);
  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_last_start_dst(), 0x2222U);
  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_last_start_length(), 128U);
}

TEST_F(DMA2Test, IRQHandlersDispatchMappedDMAHandles) {
  ST_LIB::DMA_Domain::Init<2>::init(irq_cfg);
  auto &dma0 = ST_LIB::DMA_Domain::Init<2>::instances[0].dma;
  auto &dma1 = ST_LIB::DMA_Domain::Init<2>::instances[1].dma;

  DMA1_Stream0_IRQHandler();
  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_call_count(ST_LIB::MockedHAL::DMAOperation::IRQHandler),
            1U);
  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_last_irq_handle(), &dma0);

  DMA2_Stream7_IRQHandler();
  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_call_count(ST_LIB::MockedHAL::DMAOperation::IRQHandler),
            2U);
  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_last_irq_handle(), &dma1);
}
