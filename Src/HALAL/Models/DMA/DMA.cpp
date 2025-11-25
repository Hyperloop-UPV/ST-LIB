#include "HALAL/Models/DMA/DMA.hpp"

template<typename PeriphHandle>
DMA::DmaLinkEntry DMA::make_dma_entry(
    PeriphHandle* periph,
    DMA_HandleTypeDef* dma,
    DMA_HandleTypeDef* PeriphHandle::* member,
    IRQn_Type irqn
){
    return DmaLinkEntry{
        periph,
        dma,
        [periph, member](DMA_HandleTypeDef* d) {
            periph->*member = d;
            d->Parent = periph;
        },
        irqn
    };
}

template<auto Instance, auto... Streams>
requires (
    (DMA::is_adc(Instance)  && sizeof...(Streams) == 1) ||
    (DMA::is_fmac(Instance) && sizeof...(Streams) == 3) ||
    (!DMA::is_adc(Instance) && !DMA::is_fmac(Instance) && sizeof...(Streams) == 2)
)
void DMA::inscribe_stream(PeriphVariant handle) {
    const std::size_t N = sizeof...(Streams);
    if (inscribed_index + N > MAX_STREAMS){
        ErrorHandler("Too many streams inscribed");
    }
    
    unsigned long periph_addr = reinterpret_cast<unsigned long>(Instance);
    if (!is_peripherial_available(periph_addr)){
        ErrorHandler("Peripheral already in use");
    }
    
    std::array<DMA_Stream_TypeDef*, N> streams = {(DMA_Stream_TypeDef*)Streams... };

    for (uint8_t i = 0; i < N; i++){
        uintptr_t stream_addr = reinterpret_cast<uintptr_t>(streams[i]);
        if (!is_stream_available(stream_addr)){
            ErrorHandler("DMA stream already in use");
        }
    }

    for (uint8_t i = 0; i < N; i++){
        used_streams.insert(reinterpret_cast<uintptr_t>(streams[i]));
    }
    used_peripherials.insert(periph_addr);

    for (uint8_t i = 0; i < N; i++){
        DMA_HandleTypeDef* dma = new DMA_HandleTypeDef{};
        dma->Instance = streams[i];
        dma->Init.Request             = get_Request(Instance, i);
        dma->Init.Direction           = get_Direction(Instance, i);
        dma->Init.PeriphInc           = get_PeriphInc(Instance, i);
        dma->Init.MemInc              = get_MemInc(Instance, i);
        dma->Init.PeriphDataAlignment = get_PeriphDataAlignment(Instance, i);
        dma->Init.MemDataAlignment    = get_MemDataAlignment(Instance, i);
        dma->Init.Mode                = get_Mode(Instance, i);
        dma->Init.Priority            = get_Priority(Instance, i);
        dma->Init.FIFOMode            = get_FIFOMode(Instance, i);
        dma->Init.FIFOThreshold       = get_FIFOThreshold(Instance, i);
        dma->Init.MemBurst            = get_MemBurst(Instance, i);
        dma->Init.PeriphBurst         = get_PeriphBurst(Instance, i);
        IRQn_Type irq = get_irqn(streams[i]);

        if constexpr (is_spi(Instance)) {
            auto* spi_handle = std::get<SPI_HandleTypeDef*>(handle);
            auto member = (i == 0) ? &SPI_HandleTypeDef::hdmarx : &SPI_HandleTypeDef::hdmatx;
            inscribed_streams[inscribed_index] = make_dma_entry(spi_handle, dma, member, irq);
        }
        else if constexpr (is_i2c(Instance)) {
            auto* i2c_handle = std::get<I2C_HandleTypeDef*>(handle);
            auto member = (i == 0) ? &I2C_HandleTypeDef::hdmarx : &I2C_HandleTypeDef::hdmatx;
            inscribed_streams[inscribed_index] = make_dma_entry(i2c_handle, dma, member, irq);
        }
        else if constexpr (is_adc(Instance)) {
            auto* adc_handle = std::get<ADC_HandleTypeDef*>(handle);
            auto member = &ADC_HandleTypeDef::DMA_Handle;
            inscribed_streams[inscribed_index] = make_dma_entry(adc_handle, dma, member, irq);
        }
        else if constexpr (is_fmac(Instance)) {
            auto* fmac_handle = std::get<FMAC_HandleTypeDef*>(handle);
            auto member = (i == 0) ? &FMAC_HandleTypeDef::hdmaPreload :
                          (i == 1) ? &FMAC_HandleTypeDef::hdmaIn :
                                     &FMAC_HandleTypeDef::hdmaOut;
            inscribed_streams[inscribed_index] = make_dma_entry(fmac_handle, dma, member, irq);
        }
        
        inscribed_index++;
    }
}

void DMA::start() {
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    for (uint8_t i = 0; i < inscribed_index; i++) {
        auto& [periph, dma, linker, irq] = inscribed_streams[i];
        
        if (HAL_DMA_Init(dma) != HAL_OK) {
            ErrorHandler("DMA Init failed");
        }
        
        linker(dma);
        
        HAL_NVIC_SetPriority(irq, 0, 0);
        HAL_NVIC_EnableIRQ(irq);
    }
}
