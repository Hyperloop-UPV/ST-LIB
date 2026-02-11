#include <array>

#include <gtest/gtest.h>

#include "HALAL/Models/DMA/DMA2.hpp"
#include "HALAL/Models/SPI/SPI2.hpp"
#include "MockedDrivers/NVIC.hpp"
#include "MockedDrivers/mocked_hal_dma.hpp"
#include "MockedDrivers/mocked_hal_spi.hpp"

extern uint32_t SystemCoreClock;
namespace ST_LIB::TestErrorHandler {
void reset();
void set_fail_on_error(bool enabled);
extern int call_count;
} // namespace ST_LIB::TestErrorHandler

namespace {

struct SPIDomainInstanceLayout {
  SPI_HandleTypeDef hspi;
  SPI_TypeDef *instance;
  volatile bool *operation_flag;
};

struct SPIRequestMock {
  ST_LIB::SPIDomain::SPIMode mode;
  ST_LIB::SPIConfigTypes::SPIConfig config;
};

consteval SPIRequestMock make_request(ST_LIB::SPIDomain::SPIMode mode,
                                      ST_LIB::SPIConfigTypes::DataSize data_size) {
  ST_LIB::SPIConfigTypes::SPIConfig cfg{};
  cfg.data_size = data_size;
  cfg.nss_mode = ST_LIB::SPIConfigTypes::NSSMode::SOFTWARE;
  cfg.direction = ST_LIB::SPIConfigTypes::Direction::FULL_DUPLEX;
  return {.mode = mode, .config = cfg};
}

inline constexpr SPIRequestMock master8_request =
    make_request(ST_LIB::SPIDomain::SPIMode::MASTER,
                 ST_LIB::SPIConfigTypes::DataSize::SIZE_8BIT);
inline constexpr SPIRequestMock master16_request =
    make_request(ST_LIB::SPIDomain::SPIMode::MASTER,
                 ST_LIB::SPIConfigTypes::DataSize::SIZE_16BIT);
inline constexpr SPIRequestMock slave8_request =
    make_request(ST_LIB::SPIDomain::SPIMode::SLAVE,
                 ST_LIB::SPIConfigTypes::DataSize::SIZE_8BIT);

constexpr std::array<ST_LIB::SPIDomain::Entry, 1> compile_time_entry{{
    {.peripheral = ST_LIB::SPIDomain::SPIPeripheral::spi2,
     .mode = ST_LIB::SPIDomain::SPIMode::MASTER,
     .sck_gpio_idx = 0,
     .miso_gpio_idx = 1,
     .mosi_gpio_idx = 2,
     .nss_gpio_idx = std::nullopt,
     .dma_rx_idx = 0,
     .dma_tx_idx = 1,
     .max_baudrate = 1'000'000,
     .config = {}},
}};

constexpr auto compile_time_cfg = ST_LIB::SPIDomain::build<1>(
    std::span<const ST_LIB::SPIDomain::Entry, 1>{compile_time_entry});
static_assert(compile_time_cfg[0].peripheral ==
              ST_LIB::SPIDomain::SPIPeripheral::spi2);
static_assert(compile_time_cfg[0].mode == ST_LIB::SPIDomain::SPIMode::MASTER);

constexpr std::array<ST_LIB::DMA_Domain::Entry, 2> dma_entries{{
    {.instance = ST_LIB::DMA_Domain::Peripheral::spi2,
     .stream = ST_LIB::DMA_Domain::Stream::dma1_stream0,
     .irqn = DMA1_Stream0_IRQn,
     .id = 0},
    {.instance = ST_LIB::DMA_Domain::Peripheral::spi2,
     .stream = ST_LIB::DMA_Domain::Stream::dma1_stream1,
     .irqn = DMA1_Stream1_IRQn,
     .id = 1},
}};

constexpr auto dma_cfg = ST_LIB::DMA_Domain::build<2>(
    std::span<const ST_LIB::DMA_Domain::Entry, 2>{dma_entries});

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

class SPI2Test : public ::testing::Test {
protected:
  void SetUp() override {
    SystemCoreClock = 64'000'000U;
    ST_LIB::MockedHAL::spi_reset();
    ST_LIB::MockedHAL::dma_reset();
    ST_LIB::TestErrorHandler::reset();
    clear_nvic_enables();
    clear_dma_irq_table();
    for (auto &inst : ST_LIB::SPIDomain::spi_instances) {
      inst = nullptr;
    }
    ST_LIB::DMA_Domain::Init<2>::init(dma_cfg);
  }

  template <ST_LIB::SPIDomain::SPIMode Mode, ST_LIB::SPIConfigTypes::DataSize DataSize>
  ST_LIB::SPIDomain::Instance &init_spi(uint32_t max_baudrate) {
    ST_LIB::SPIConfigTypes::SPIConfig config{};
    config.nss_mode = ST_LIB::SPIConfigTypes::NSSMode::SOFTWARE;
    config.data_size = DataSize;
    config.direction = ST_LIB::SPIConfigTypes::Direction::FULL_DUPLEX;

    const std::array<ST_LIB::SPIDomain::Config, 1> cfgs{{
        {.peripheral = ST_LIB::SPIDomain::SPIPeripheral::spi2,
         .mode = Mode,
         .sck_gpio_idx = 0,
         .miso_gpio_idx = 1,
         .mosi_gpio_idx = 2,
         .nss_gpio_idx = std::nullopt,
         .dma_rx_idx = 0,
         .dma_tx_idx = 1,
         .max_baudrate = max_baudrate,
         .config = config},
    }};

    ST_LIB::SPIDomain::Init<1>::init(
        cfgs, std::span<ST_LIB::GPIODomain::Instance>{},
        std::span<ST_LIB::DMA_Domain::Instance>(ST_LIB::DMA_Domain::Init<2>::instances));
    return ST_LIB::SPIDomain::Init<1>::instances[0];
  }
};

TEST_F(SPI2Test, InitMasterConfiguresPeripheralDMAAndNVIC) {
  auto &instance =
      init_spi<ST_LIB::SPIDomain::SPIMode::MASTER, ST_LIB::SPIConfigTypes::DataSize::SIZE_8BIT>(
          20'000'000U);

  auto *hspi = ST_LIB::MockedHAL::spi_get_last_handle();
  ASSERT_NE(hspi, nullptr);
  EXPECT_EQ(hspi->Instance, reinterpret_cast<SPI_TypeDef *>(SPI2_BASE));
  EXPECT_EQ(hspi->Init.Mode, SPI_MODE_MASTER);
  EXPECT_EQ(hspi->Init.NSS, SPI_NSS_SOFT);
  EXPECT_EQ(hspi->Init.DataSize, 7U);
  EXPECT_EQ(hspi->Init.BaudRatePrescaler, SPI_BAUDRATEPRESCALER_4);

  EXPECT_EQ(hspi->hdmarx, &ST_LIB::DMA_Domain::Init<2>::instances[0].dma);
  EXPECT_EQ(hspi->hdmatx, &ST_LIB::DMA_Domain::Init<2>::instances[1].dma);
  EXPECT_EQ(hspi->hdmarx->Parent, hspi);
  EXPECT_EQ(hspi->hdmatx->Parent, hspi);

  EXPECT_EQ(ST_LIB::DMA_Domain::Init<2>::instances[0].dma.Init.PeriphDataAlignment,
            DMA_PDATAALIGN_BYTE);
  EXPECT_EQ(ST_LIB::DMA_Domain::Init<2>::instances[0].dma.Init.MemDataAlignment,
            DMA_MDATAALIGN_BYTE);
  EXPECT_EQ(ST_LIB::DMA_Domain::Init<2>::instances[1].dma.Init.PeriphDataAlignment,
            DMA_PDATAALIGN_BYTE);
  EXPECT_EQ(ST_LIB::DMA_Domain::Init<2>::instances[1].dma.Init.MemDataAlignment,
            DMA_MDATAALIGN_BYTE);

  EXPECT_EQ(NVIC_GetEnableIRQ(SPI2_IRQn), 1U);
  EXPECT_EQ(ST_LIB::SPIDomain::spi_instances[1], &instance);
  EXPECT_EQ(ST_LIB::MockedHAL::dma_get_call_count(ST_LIB::MockedHAL::DMAOperation::Init), 4U);
}

TEST_F(SPI2Test, InitWith32BitDataUsesWordAlignmentAndPrescaler) {
  init_spi<ST_LIB::SPIDomain::SPIMode::MASTER, ST_LIB::SPIConfigTypes::DataSize::SIZE_32BIT>(
      8'000'000U);

  auto *hspi = ST_LIB::MockedHAL::spi_get_last_handle();
  ASSERT_NE(hspi, nullptr);
  EXPECT_EQ(hspi->Init.DataSize, 31U);
  EXPECT_EQ(hspi->Init.BaudRatePrescaler, SPI_BAUDRATEPRESCALER_8);

  EXPECT_EQ(ST_LIB::DMA_Domain::Init<2>::instances[0].dma.Init.PeriphDataAlignment,
            DMA_PDATAALIGN_WORD);
  EXPECT_EQ(ST_LIB::DMA_Domain::Init<2>::instances[0].dma.Init.MemDataAlignment,
            DMA_MDATAALIGN_WORD);
  EXPECT_EQ(ST_LIB::DMA_Domain::Init<2>::instances[1].dma.Init.PeriphDataAlignment,
            DMA_PDATAALIGN_WORD);
  EXPECT_EQ(ST_LIB::DMA_Domain::Init<2>::instances[1].dma.Init.MemDataAlignment,
            DMA_MDATAALIGN_WORD);
}

TEST_F(SPI2Test, InitFailureOnHALSPIInitDoesNotRegisterOrEnableNVIC) {
  ST_LIB::TestErrorHandler::set_fail_on_error(false);
  ST_LIB::MockedHAL::spi_set_status(HAL_ERROR);

  init_spi<ST_LIB::SPIDomain::SPIMode::MASTER, ST_LIB::SPIConfigTypes::DataSize::SIZE_8BIT>(
      20'000'000U);

  EXPECT_EQ(ST_LIB::TestErrorHandler::call_count, 1);
  EXPECT_EQ(ST_LIB::SPIDomain::spi_instances[1], nullptr);
  EXPECT_EQ(NVIC_GetEnableIRQ(SPI2_IRQn), 0U);
}

TEST_F(SPI2Test, SPI2IRQHandlerDispatchesToHALIRQ) {
  init_spi<ST_LIB::SPIDomain::SPIMode::MASTER, ST_LIB::SPIConfigTypes::DataSize::SIZE_8BIT>(
      20'000'000U);
  auto *hspi = ST_LIB::MockedHAL::spi_get_last_handle();
  ASSERT_NE(hspi, nullptr);

  const auto before = ST_LIB::MockedHAL::spi_get_call_count(
      ST_LIB::MockedHAL::SPIOperation::IRQHandler);
  SPI2_IRQHandler();
  EXPECT_EQ(ST_LIB::MockedHAL::spi_get_call_count(ST_LIB::MockedHAL::SPIOperation::IRQHandler),
            before + 1U);
  EXPECT_EQ(ST_LIB::MockedHAL::spi_get_last_handle(), hspi);
}

TEST_F(SPI2Test, MasterWrapperUsesFrameWordCountsAndMinTransactionSize) {
  auto &instance =
      init_spi<ST_LIB::SPIDomain::SPIMode::MASTER, ST_LIB::SPIConfigTypes::DataSize::SIZE_16BIT>(
          20'000'000U);
  ST_LIB::SPIDomain::SPIWrapper<master16_request> spi(instance);

  std::array<uint8_t, 4> tx{1, 2, 3, 4};
  EXPECT_TRUE(spi.send(std::span<uint8_t, 4>{tx}));
  EXPECT_EQ(ST_LIB::MockedHAL::spi_get_last_size_words(), 2U);
  ASSERT_EQ(ST_LIB::MockedHAL::spi_get_last_tx_size_bytes(), 4U);

  std::array<uint8_t, 2> pattern{0xA5, 0x5A};
  ST_LIB::MockedHAL::spi_set_rx_pattern(pattern);
  std::array<uint8_t, 4> rx{0, 0, 0, 0};
  EXPECT_TRUE(spi.receive(std::span<uint8_t, 4>{rx}));
  EXPECT_EQ(ST_LIB::MockedHAL::spi_get_last_size_words(), 2U);
  EXPECT_EQ(rx[0], 0xA5);
  EXPECT_EQ(rx[1], 0x5A);
  EXPECT_EQ(rx[2], 0xA5);
  EXPECT_EQ(rx[3], 0x5A);

  std::array<uint8_t, 6> tx_long{9, 8, 7, 6, 5, 4};
  std::array<uint8_t, 4> rx_short{0, 0, 0, 0};
  EXPECT_TRUE(spi.transceive(std::span<uint8_t, 6>{tx_long},
                             std::span<uint8_t, 4>{rx_short}));
  EXPECT_EQ(ST_LIB::MockedHAL::spi_get_last_size_words(), 2U);
  ASSERT_EQ(ST_LIB::MockedHAL::spi_get_last_tx_size_bytes(), 4U);
}

TEST_F(SPI2Test, DMACompletionCallbacksSetOperationFlag) {
  auto &instance =
      init_spi<ST_LIB::SPIDomain::SPIMode::MASTER, ST_LIB::SPIConfigTypes::DataSize::SIZE_8BIT>(
          20'000'000U);
  ST_LIB::SPIDomain::SPIWrapper<master8_request> spi(instance);

  std::array<uint8_t, 4> tx{1, 2, 3, 4};
  std::array<uint8_t, 4> rx{0, 0, 0, 0};
  volatile bool done = false;

  EXPECT_TRUE(spi.send_DMA(std::span<uint8_t, 4>{tx}, &done));
  EXPECT_FALSE(done);
  HAL_SPI_TxCpltCallback(ST_LIB::MockedHAL::spi_get_last_handle());
  EXPECT_TRUE(done);

  done = false;
  EXPECT_TRUE(spi.receive_DMA(std::span<uint8_t, 4>{rx}, &done));
  EXPECT_FALSE(done);
  HAL_SPI_RxCpltCallback(ST_LIB::MockedHAL::spi_get_last_handle());
  EXPECT_TRUE(done);

  done = false;
  EXPECT_TRUE(
      spi.transceive_DMA(std::span<uint8_t, 4>{tx}, std::span<uint8_t, 4>{rx}, &done));
  EXPECT_FALSE(done);
  HAL_SPI_TxRxCpltCallback(ST_LIB::MockedHAL::spi_get_last_handle());
  EXPECT_TRUE(done);
}

TEST_F(SPI2Test, CallbacksWithUnknownHandleTriggerErrorPath) {
  ST_LIB::TestErrorHandler::set_fail_on_error(false);

  SPI_HandleTypeDef unknown{};
  HAL_SPI_TxCpltCallback(&unknown);
  EXPECT_EQ(ST_LIB::TestErrorHandler::call_count, 1);
}

TEST_F(SPI2Test, ErrorCallbackOnKnownHandleTriggersErrorPath) {
  ST_LIB::TestErrorHandler::set_fail_on_error(false);
  init_spi<ST_LIB::SPIDomain::SPIMode::MASTER, ST_LIB::SPIConfigTypes::DataSize::SIZE_8BIT>(
      20'000'000U);

  auto *hspi = ST_LIB::MockedHAL::spi_get_last_handle();
  ASSERT_NE(hspi, nullptr);
  hspi->ErrorCode = 0x55U;
  HAL_SPI_ErrorCallback(hspi);

  EXPECT_EQ(ST_LIB::TestErrorHandler::call_count, 1);
}

TEST_F(SPI2Test, SlaveWrapperDMAOperations) {
  auto &instance =
      init_spi<ST_LIB::SPIDomain::SPIMode::SLAVE, ST_LIB::SPIConfigTypes::DataSize::SIZE_8BIT>(
          1'000'000U);
  ST_LIB::SPIDomain::SPIWrapper<slave8_request> spi(instance);

  std::array<uint8_t, 4> tx{4, 3, 2, 1};
  std::array<uint8_t, 4> rx{0, 0, 0, 0};
  volatile bool done = false;

  EXPECT_TRUE(spi.listen(std::span<uint8_t, 4>{rx}, &done));
  EXPECT_FALSE(done);
  HAL_SPI_RxCpltCallback(ST_LIB::MockedHAL::spi_get_last_handle());
  EXPECT_TRUE(done);

  done = false;
  EXPECT_TRUE(spi.arm(std::span<uint8_t, 4>{tx}, &done));
  EXPECT_FALSE(done);
  HAL_SPI_TxCpltCallback(ST_LIB::MockedHAL::spi_get_last_handle());
  EXPECT_TRUE(done);

  done = false;
  EXPECT_TRUE(spi.transceive(std::span<uint8_t, 4>{tx}, std::span<uint8_t, 4>{rx}, &done));
  EXPECT_FALSE(done);
  HAL_SPI_TxRxCpltCallback(ST_LIB::MockedHAL::spi_get_last_handle());
  EXPECT_TRUE(done);
}

TEST_F(SPI2Test, SlaveSoftwareNSSTogglesSSIWithSafePointerMapping) {
  auto &instance =
      init_spi<ST_LIB::SPIDomain::SPIMode::SLAVE, ST_LIB::SPIConfigTypes::DataSize::SIZE_8BIT>(
          1'000'000U);

  auto *layout = reinterpret_cast<SPIDomainInstanceLayout *>(&instance);
  layout->instance = SPI2;

  ST_LIB::SPIDomain::SPIWrapper<slave8_request> spi(instance);
  layout->instance->CR1 |= SPI_CR1_SSI;

  spi.set_software_nss(true);
  EXPECT_EQ(layout->instance->CR1 & SPI_CR1_SSI, 0U);

  spi.set_software_nss(false);
  EXPECT_NE(layout->instance->CR1 & SPI_CR1_SSI, 0U);
}

TEST_F(SPI2Test, BusyStatusReturnsFalseInBlockingAndDMAPaths) {
  auto &instance =
      init_spi<ST_LIB::SPIDomain::SPIMode::MASTER, ST_LIB::SPIConfigTypes::DataSize::SIZE_8BIT>(
          20'000'000U);
  ST_LIB::SPIDomain::SPIWrapper<master8_request> spi(instance);
  ST_LIB::MockedHAL::spi_set_busy(true);

  std::array<uint8_t, 4> tx{1, 2, 3, 4};
  std::array<uint8_t, 4> rx{0, 0, 0, 0};

  EXPECT_FALSE(spi.send(std::span<uint8_t, 4>{tx}));
  EXPECT_FALSE(spi.receive(std::span<uint8_t, 4>{rx}));
  EXPECT_FALSE(spi.transceive(std::span<uint8_t, 4>{tx}, std::span<uint8_t, 4>{rx}));

  EXPECT_FALSE(spi.send_DMA(std::span<uint8_t, 4>{tx}));
  EXPECT_FALSE(spi.receive_DMA(std::span<uint8_t, 4>{rx}));
  EXPECT_FALSE(spi.transceive_DMA(std::span<uint8_t, 4>{tx}, std::span<uint8_t, 4>{rx}));
}
