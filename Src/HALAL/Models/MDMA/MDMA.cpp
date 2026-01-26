#include "HALAL/Models/MDMA/MDMA.hpp"

D1_NC MDMA_LinkNodeTypeDef MDMA::internal_nodes[8];

#include <algorithm>

std::bitset<8> MDMA::instance_free_map{};

MDMA_Channel_TypeDef* MDMA::get_channel(uint8_t id) {
    if (id > 15) {
        return nullptr;
    }
    return reinterpret_cast<MDMA_Channel_TypeDef*>(MDMA_BASE + 0x40UL + (id * 0x40UL));
}

uint8_t MDMA::get_instance_id(MDMA_Channel_TypeDef* channel) {
    uint32_t address = reinterpret_cast<uint32_t>(channel);
    return static_cast<uint8_t>((address - (MDMA_BASE + 0x40UL)) / 0x40UL);
}


void MDMA::prepare_transfer(Instance& instance, MDMA_LinkNodeTypeDef* first_node)
{
    if (instance.handle.State == HAL_MDMA_STATE_BUSY )
    {
        ErrorHandler("MDMA transfer already in progress");
        return;
    }
    instance_free_map[instance.id] = false;

    instance.handle.State = HAL_MDMA_STATE_BUSY;
    instance.handle.ErrorCode = HAL_MDMA_ERROR_NONE;
    instance.handle.FirstLinkedListNodeAddress = first_node;

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
        instance.handle.State = HAL_MDMA_STATE_BUSY;
        ErrorHandler("Error generating MDMA SW request");
        return;
    }

}

void MDMA::prepare_transfer(Instance& instance, LinkedListNode* first_node) { MDMA::prepare_transfer(instance, first_node->get_node()); }



MDMA::Instance& MDMA::get_instance(uint8_t id)
{

    Instance& instance = instances[id];

    return instance;
}


void MDMA::inscribe(Instance& instance,uint8_t id)
{
    MDMA_HandleTypeDef mdma_handle{};
    MDMA_Channel_TypeDef* channel = get_channel(id);
    if (channel == nullptr)
    {
        ErrorHandler("MDMA channel mapping not found");
        return;
    }
    mdma_handle.Instance = channel;
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
    MDMA_LinkNodeTypeDef* transfer_node = &internal_nodes[id];

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
    nodeConfig.SrcAddress = reinterpret_cast<uint32_t>(nullptr);
    nodeConfig.DstAddress = reinterpret_cast<uint32_t>(nullptr);

    const HAL_StatusTypeDef status = HAL_MDMA_LinkedList_CreateNode(transfer_node, &nodeConfig);
    if (status != HAL_OK)
    {
        ErrorHandler("Error creating linked list in MDMA");
    }
    instance_free_map[id] = true;

    instance = Instance(mdma_handle, id, nullptr, transfer_node);

}


void MDMA::start()
{
    __HAL_RCC_MDMA_CLK_ENABLE();

    uint8_t id = 0;
    for (auto& instance : instances)
    {
        inscribe(instance,id);
        id++;
        
        if (instance.handle.Instance == nullptr)
        {
            ErrorHandler("MDMA instance not initialised");
        }
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

void MDMA::update()
{
    if(transfer_queue.empty())
    {
        return;
    }

    for(size_t i = 0; i < instances.size(); i++)
    {
        if(transfer_queue.empty())
        {
            return;
        }
        if(instance_free_map[i])
        {
            Instance& instance = get_instance(i);
            auto transfer = transfer_queue.top();
            instance.done = transfer.second;
            prepare_transfer(instance, transfer.first);
            transfer_queue.pop();
        }
    }
}


void MDMA::irq_handler()
{
    for (auto& instance : instances)
    {
        if (instance.handle.Instance == nullptr)
        {
            continue;
        }
        HAL_MDMA_IRQHandler(&instance.handle);
    }
}


void MDMA::transfer_list(MDMA::LinkedListNode* first_node, volatile bool* done)
{
    if(transfer_queue.size() >= TRANSFER_QUEUE_MAX_SIZE)
    {
        ErrorHandler("MDMA transfer queue full");
        return;
    }
    transfer_queue.push({first_node, done});
}



void MDMA::transfer_data(uint8_t* source_address, uint8_t* destination_address, const uint32_t data_length, volatile bool* done)
{
    for(size_t i = 0; i < instances.size(); i++)
    {
        if(instance_free_map[i])
        {
            Instance& instance = get_instance(i);
            instance.done = done;

            instance.transfer_node->CSAR = reinterpret_cast<uint32_t>(source_address);
            instance.transfer_node->CBNDTR = data_length;
            instance.transfer_node->CDAR = reinterpret_cast<uint32_t>(destination_address);

            prepare_transfer(instance, instance.transfer_node);
            return;
        }
        if(done)
    {
        *done = false;
    }
    }
    return;
}


void MDMA::TransferCompleteCallback(MDMA_HandleTypeDef *hmdma)
{
    uint8_t id = get_instance_id(hmdma->Instance);
    if (id >= instances.size())
    {
        ErrorHandler("MDMA channel not registered");
        return;
    }

    Instance& instance = get_instance(id);
    instance.handle.State = HAL_MDMA_STATE_READY;
    instance_free_map[instance.id] = true;
    if(instance.done == nullptr)
    {
        return;
    }
    *(instance.done) = true;

}


void MDMA::TransferErrorCallback(MDMA_HandleTypeDef *hmdma)
{
    uint8_t id = get_instance_id(hmdma->Instance);
    if (id >= instances.size())
    {
        ErrorHandler("MDMA channel not registered");
        return;
    }

    Instance& instance = get_instance(id);
    if(instance.done != nullptr)
    {
        *(instance.done) = false;
    }

    const unsigned long error_code = static_cast<unsigned long>(hmdma->ErrorCode);
    ErrorHandler("MDMA Transfer Error, code: " + std::to_string(error_code));
}


extern "C" void MDMA_IRQHandler(void)
{
    MDMA::irq_handler();
}
