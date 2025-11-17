#include "HALAL/Models/MDMA/MDMA.hpp"



std::unordered_map<uint8_t, uint32_t> MDMA::src_size_to_flags = {
    {1, MDMA_SRC_DATASIZE_BYTE},
    {2, MDMA_SRC_DATASIZE_HALFWORD},
    {4, MDMA_SRC_DATASIZE_WORD}
};
std::unordered_map<uint8_t, uint32_t> MDMA::dst_size_to_flags = {
    {1, MDMA_DEST_DATASIZE_BYTE},
    {2, MDMA_DEST_DATASIZE_HALFWORD},
    {4, MDMA_DEST_DATASIZE_WORD}
};

std::unordered_map<uint8_t, MDMA_Channel_TypeDef*> MDMA::instance_to_channel = {
    {0, MDMA_Channel0},
    {1, MDMA_Channel1},
    {2, MDMA_Channel2},
    {3, MDMA_Channel3},
    {4, MDMA_Channel4},
    {5, MDMA_Channel5},
    {6, MDMA_Channel6},
    {7, MDMA_Channel7}
};

std::unordered_map<MDMA_Channel_TypeDef*, uint8_t> MDMA::channel_to_instance = {
    {MDMA_Channel0, 0},
    {MDMA_Channel1, 1},
    {MDMA_Channel2, 2},
    {MDMA_Channel3, 3},
    {MDMA_Channel4, 4},
    {MDMA_Channel5, 5},
    {MDMA_Channel6, 6},
    {MDMA_Channel7, 7}
};


void MDMA::prepare_transfer(Instance& instance, MDMA_LinkNodeTypeDef* first_node)
{
    if (instance.handle.State == HAL_MDMA_STATE_BUSY || instance.handle.Lock == HAL_LOCKED)
    {
        ErrorHandler("MDMA transfer already in progress");
        return;
    }

    instance.handle.Lock = HAL_LOCKED;

    instance.handle.State = HAL_MDMA_STATE_BUSY;
    instance.handle.ErrorCode = HAL_MDMA_ERROR_NONE;
    instance.handle.FirstLinkedListNodeAddress = first_node;

    __HAL_MDMA_DISABLE(&instance.handle);
    while ((instance.handle.Instance->CCR & MDMA_CCR_EN) != 0U)
    {
        __NOP();
    }

    MDMA_Channel_TypeDef* channel = instance.handle.Instance;

    channel->CTCR = first_node->CTCR;
    channel->CBNDTR = first_node->CBNDTR;
    channel->CSAR = first_node->CSAR;
    channel->CDAR = first_node->CDAR;
    channel->CBRUR = first_node->CBRUR;
    channel->CTBR = first_node->CTBR;
    channel->CMAR = first_node->CMAR;
    channel->CMDR = first_node->CMDR;
    channel->CLAR = first_node->CLAR;

    const uint32_t clear_flags = MDMA_FLAG_TE | MDMA_FLAG_CTC | MDMA_FLAG_BT | MDMA_FLAG_BFTC | MDMA_FLAG_BRT;
    __HAL_MDMA_CLEAR_FLAG(&instance.handle, clear_flags);

    __HAL_MDMA_ENABLE_IT(&instance.handle, MDMA_IT_TE | MDMA_IT_CTC);

    __HAL_MDMA_ENABLE(&instance.handle);

    if (HAL_MDMA_GenerateSWRequest(&instance.handle) != HAL_OK)
    {
        instance.handle.State = HAL_MDMA_STATE_READY;
        instance.handle.Lock = HAL_UNLOCKED;
        ErrorHandler("Error generating MDMA SW request");
        return;
    }

    instance.handle.Lock = HAL_UNLOCKED;
}



uint8_t MDMA::inscribe(uint8_t* data_buffer, uint8_t* destination_address)
{
    const uint8_t id = static_cast<uint8_t>(instances.size());
    MDMA_HandleTypeDef mdma_handle{};
    mdma_handle.Instance = instance_to_channel[id];
    mdma_handle.Init.Request = MDMA_REQUEST_SW;
    mdma_handle.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    mdma_handle.Init.Priority = MDMA_PRIORITY_VERY_HIGH;
    mdma_handle.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    mdma_handle.Init.SourceInc = MDMA_SRC_INC_BYTE;
    mdma_handle.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    mdma_handle.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    mdma_handle.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    mdma_handle.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    mdma_handle.Init.BufferTransferLength = 1;
    mdma_handle.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    mdma_handle.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    mdma_handle.Init.SourceBlockAddressOffset = 0;
    mdma_handle.Init.DestBlockAddressOffset = 0;

    MDMA_LinkNodeConfTypeDef nodeConfig{};
    MDMA_LinkNodeTypeDef transfer_node{};

    nodeConfig.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    nodeConfig.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    nodeConfig.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    nodeConfig.Init.BufferTransferLength = 1;
    nodeConfig.Init.TransferTriggerMode = MDMA_FULL_TRANSFER;
    nodeConfig.Init.SourceBlockAddressOffset = 0;
    nodeConfig.Init.DestBlockAddressOffset = 0;
    nodeConfig.BlockCount = 1;
    nodeConfig.Init.Priority = MDMA_PRIORITY_HIGH;
    nodeConfig.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    nodeConfig.Init.Request = MDMA_REQUEST_SW;
    nodeConfig.Init.SourceInc = MDMA_SRC_INC_BYTE;
    nodeConfig.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    nodeConfig.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    nodeConfig.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    nodeConfig.BlockDataLength = 1;
    nodeConfig.SrcAddress = reinterpret_cast<uint32_t>(data_buffer);
    uint8_t* dst_ptr = (destination_address == nullptr) ? data_buffer : destination_address;
    nodeConfig.DstAddress = reinterpret_cast<uint32_t>(dst_ptr);

    const HAL_StatusTypeDef status = HAL_MDMA_LinkedList_CreateNode(&transfer_node, &nodeConfig);
    if (status != HAL_OK)
    {
        ErrorHandler("Error creating linked list in MDMA");
    }

    Instance instance(mdma_handle, id, data_buffer, destination_address, transfer_node);
    instances[id] = instance;

    return id;
}

void MDMA::start()
{
    __HAL_RCC_MDMA_CLK_ENABLE();

    for (auto& entry : instances)
    {
        Instance& instance = entry.second;
        const HAL_StatusTypeDef status = HAL_MDMA_Init(&instance.handle);
        if (status != HAL_OK)
        {
            ErrorHandler("Error initialising MDMA instance");
        }

        HAL_MDMA_RegisterCallback(&instance.handle, HAL_MDMA_XFER_CPLT_CB_ID, MDMA::TransferCompleteCallback);
        HAL_MDMA_RegisterCallback(&instance.handle, HAL_MDMA_XFER_ERROR_CB_ID, MDMA::TransferErrorCallback);
        __HAL_MDMA_ENABLE_IT(&instance.handle, MDMA_IT_TE | MDMA_IT_CTC);
    }

    HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);
}

void MDMA::irq_handler()
{
    for (auto& entry : instances)
    {
        HAL_MDMA_IRQHandler(&entry.second.handle);
    }
}

void MDMA::transfer_packet(const uint8_t MDMA_id, const uint8_t packet_id,uint8_t* destination_address,Promise* promise)
{
    Instance& instance = instances[MDMA_id];

    if(promise == nullptr)
    {
        instance.using_promise = false;
    }
    else
    {
        instance.promise = promise;
        instance.using_promise = true;
    }

    auto& nodes = linked_lists[packet_id];
    if (nodes.empty())
    {
        ErrorHandler("No linked list nodes for MDMA packet");
    }

    uint8_t* final_destination = destination_address;
    if (final_destination == nullptr)
    {
        final_destination = instance.destination_address;
    }

    if (final_destination == nullptr)
    {
        ErrorHandler("No destination address provided for MDMA transfer");
    }

    nodes.back().CDAR = reinterpret_cast<uint32_t>(final_destination);
    nodes.back().CBNDTR = packet_sizes[packet_id];
    nodes.back().CSAR = reinterpret_cast<uint32_t>(instance.data_buffer);
    instance.last_destination = nodes.back().CDAR;
    instance.last_size = nodes.back().CBNDTR;

    

    SCB_CleanDCache_by_Addr((uint32_t*)&nodes.back(), sizeof(MDMA_LinkNodeTypeDef) * nodes.size());
    
    prepare_transfer(instance, &nodes[0]);
}

void MDMA::transfer_data(const uint8_t MDMA_id,uint8_t* source_address, const uint32_t data_length,uint8_t* destination_address,Promise* promise)
{
    Instance& instance = instances[MDMA_id];

    if(promise == nullptr)
    {
        instance.using_promise = false;
    }
    else
    {
        instance.promise = promise;
        instance.using_promise = true;
    }

    instance.transfer_node.CSAR = reinterpret_cast<uint32_t>(source_address);
    instance.transfer_node.CBNDTR = data_length;
    if(destination_address == nullptr)
    {
        if(instance.destination_address ==nullptr)
        {
            ErrorHandler("No destination address provided for MDMA transfer");
        }
        instance.transfer_node.CDAR = reinterpret_cast<uint32_t>(instance.destination_address);
    }
    else
    {
        instance.transfer_node.CDAR = reinterpret_cast<uint32_t>(destination_address);
    }

    SCB_CleanDCache_by_Addr((uint32_t*)&instance.transfer_node, sizeof(MDMA_LinkNodeTypeDef));

    prepare_transfer(instance, &instance.transfer_node);
}


void MDMA::TransferCompleteCallback(MDMA_HandleTypeDef *hmdma)
{
	Instance& instance = instances[channel_to_instance[hmdma->Instance]];
    SCB_InvalidateDCache_by_Addr((uint32_t*)instance.last_destination, instance.last_size);
    if(instance.using_promise)
    {
        instance.promise->resolve();
        instance.using_promise = false;
    }

}

void MDMA::TransferErrorCallback(MDMA_HandleTypeDef *hmdma)
{
    Instance& instance = instances[channel_to_instance[hmdma->Instance]];
    if(instance.using_promise)
    {
        instance.using_promise = false;
    }


    const unsigned long error_code = static_cast<unsigned long>(hmdma->ErrorCode);
    ErrorHandler("MDMA Transfer Error, code: " + std::to_string(error_code));
}

extern "C" void MDMA_IRQHandler(void)
{
    MDMA::irq_handler();
}



