#include "MockedDrivers/mocked_hal_adc.hpp"

#include <array>
#include <unordered_map>

ADC_HandleTypeDef hadc1{};
ADC_HandleTypeDef hadc2{};
ADC_HandleTypeDef hadc3{};

namespace {

struct ADCPeripheralState {
    bool initialized = false;
    bool configured = false;
    bool running = false;
    bool conversion_ready = false;
    bool force_poll_timeout = false;
    uint32_t active_channel = ADC_CHANNEL_0;
    uint32_t last_raw = 0;
    std::unordered_map<uint32_t, uint32_t> channel_raw_values{};
};

#if STLIB_HAS_ADC3
constexpr std::size_t kAdcCount = 3;
static std::array<ADC_TypeDef*, kAdcCount> adc_instances{ADC1, ADC2, ADC3};
#else
constexpr std::size_t kAdcCount = 2;
static std::array<ADC_TypeDef*, kAdcCount> adc_instances{ADC1, ADC2};
#endif

static std::array<ADCPeripheralState, kAdcCount> adc_states{};

static ADCPeripheralState& state_for(ADC_TypeDef* instance) {
    for (std::size_t i = 0; i < kAdcCount; ++i) {
        if (adc_instances[i] == instance) {
            return adc_states[i];
        }
    }
    return adc_states[0];
}

static uint32_t resolution_mask(uint32_t resolution) {
    switch (resolution) {
    case ADC_RESOLUTION_16B:
        return 0xFFFFU;
    case ADC_RESOLUTION_14B:
        return 0x3FFFU;
    case ADC_RESOLUTION_12B:
        return 0x0FFFU;
    case ADC_RESOLUTION_10B:
        return 0x03FFU;
    case ADC_RESOLUTION_8B:
        return 0x00FFU;
    default:
        return 0x0FFFU;
    }
}

} // namespace

namespace ST_LIB::MockedHAL {

void adc_reset() {
    for (auto& state : adc_states) {
        state = {};
    }
}

void adc_set_channel_raw(ADC_TypeDef* adc, uint32_t channel, uint32_t raw_value) {
    auto& state = state_for(adc);
    state.channel_raw_values[channel] = raw_value;
}

void adc_set_poll_timeout(ADC_TypeDef* adc, bool enabled) {
    state_for(adc).force_poll_timeout = enabled;
}

uint32_t adc_get_last_channel(ADC_TypeDef* adc) { return state_for(adc).active_channel; }

bool adc_is_running(ADC_TypeDef* adc) { return state_for(adc).running; }

} // namespace ST_LIB::MockedHAL

extern "C" HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef* hadc) {
    if (hadc == nullptr || hadc->Instance == nullptr) {
        return HAL_ERROR;
    }

    auto& state = state_for(hadc->Instance);
    state.initialized = true;
    state.running = false;
    state.configured = false;
    state.conversion_ready = false;
    state.last_raw = 0;

    hadc->State = HAL_ADC_STATE_READY;
    hadc->ErrorCode = HAL_ADC_ERROR_NONE;
    return HAL_OK;
}

extern "C" HAL_StatusTypeDef HAL_ADC_DeInit(ADC_HandleTypeDef* hadc) {
    if (hadc == nullptr || hadc->Instance == nullptr) {
        return HAL_ERROR;
    }

    auto& state = state_for(hadc->Instance);
    state = {};
    hadc->State = HAL_ADC_STATE_RESET;
    hadc->ErrorCode = HAL_ADC_ERROR_NONE;
    return HAL_OK;
}

extern "C" HAL_StatusTypeDef
HAL_ADC_ConfigChannel(ADC_HandleTypeDef* hadc, ADC_ChannelConfTypeDef* sConfig) {
    if (hadc == nullptr || hadc->Instance == nullptr || sConfig == nullptr) {
        return HAL_ERROR;
    }

    auto& state = state_for(hadc->Instance);
    if (!state.initialized) {
        return HAL_ERROR;
    }

    state.active_channel = sConfig->Channel;
    state.configured = true;
    hadc->ErrorCode = HAL_ADC_ERROR_NONE;
    return HAL_OK;
}

extern "C" HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef* hadc) {
    if (hadc == nullptr || hadc->Instance == nullptr) {
        return HAL_ERROR;
    }

    auto& state = state_for(hadc->Instance);
    if (!state.initialized || !state.configured) {
        return HAL_ERROR;
    }
    if (state.running) {
        return HAL_BUSY;
    }

    state.running = true;
    state.conversion_ready = false;
    hadc->State |= HAL_ADC_STATE_REG_BUSY;
    hadc->State &= ~HAL_ADC_STATE_REG_EOC;
    hadc->ErrorCode = HAL_ADC_ERROR_NONE;
    return HAL_OK;
}

extern "C" HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout) {
    (void)Timeout;

    if (hadc == nullptr || hadc->Instance == nullptr) {
        return HAL_ERROR;
    }

    auto& state = state_for(hadc->Instance);
    if (!state.running) {
        return HAL_ERROR;
    }

    if (state.force_poll_timeout) {
        hadc->State |= HAL_ADC_STATE_TIMEOUT;
        return HAL_TIMEOUT;
    }

    const auto it = state.channel_raw_values.find(state.active_channel);
    const uint32_t raw = (it == state.channel_raw_values.end()) ? 0U : it->second;
    state.last_raw = raw & resolution_mask(hadc->Init.Resolution);
    state.conversion_ready = true;

    hadc->State &= ~HAL_ADC_STATE_TIMEOUT;
    hadc->State |= HAL_ADC_STATE_REG_EOC;
    return HAL_OK;
}

extern "C" uint32_t HAL_ADC_GetValue(const ADC_HandleTypeDef* hadc) {
    if (hadc == nullptr || hadc->Instance == nullptr) {
        return 0;
    }
    auto& state = state_for(hadc->Instance);
    return state.last_raw;
}

extern "C" HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef* hadc) {
    if (hadc == nullptr || hadc->Instance == nullptr) {
        return HAL_ERROR;
    }

    auto& state = state_for(hadc->Instance);
    state.running = false;
    state.conversion_ready = false;

    hadc->State &= ~HAL_ADC_STATE_REG_BUSY;
    hadc->State &= ~HAL_ADC_STATE_REG_EOC;
    return HAL_OK;
}

extern "C" uint32_t HAL_ADC_GetState(const ADC_HandleTypeDef* hadc) {
    if (hadc == nullptr) {
        return HAL_ADC_STATE_RESET;
    }
    return hadc->State;
}

extern "C" uint32_t HAL_ADC_GetError(const ADC_HandleTypeDef* hadc) {
    if (hadc == nullptr) {
        return HAL_ADC_ERROR_NONE;
    }
    return hadc->ErrorCode;
}

extern "C" void
HAL_SYSCFG_AnalogSwitchConfig(uint32_t SYSCFG_AnalogSwitch, uint32_t SYSCFG_SwitchState) {
    (void)SYSCFG_AnalogSwitch;
    (void)SYSCFG_SwitchState;
}
