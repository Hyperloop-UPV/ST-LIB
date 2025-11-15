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



uint8_t MDMA::inscribe(uint8_t* data_buffer,uint8_t* destination_address)
{
    uint8_t id = instances.size();
    MDMA_HandleTypeDef mdma_handle = {};
    mdma_handle.Instance = instance_to_channel[id];
    mdma_handle.Init.Request = MDMA_REQUEST_SW;
    mdma_handle.Init.TransferTriggerMode = MDMA_BLOCK_TRANSFER;
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


    Instance instance(mdma_handle, id, data_buffer, destination_address);
    instances[id] = instance;

    MDMA_LinkNodeConfTypeDef nodeConfig;

    //Both source and destination, as well as block data length will change with the use of the transfer method, this is just a dummy initialisation
    nodeConfig.Init.DataAlignment      = MDMA_DATAALIGN_PACKENABLE;
    nodeConfig.Init.SourceBurst         = MDMA_SOURCE_BURST_SINGLE;
    nodeConfig.Init.DestBurst    = MDMA_DEST_BURST_SINGLE;
    nodeConfig.Init.BufferTransferLength = 1;
    nodeConfig.Init.TransferTriggerMode  = MDMA_BLOCK_TRANSFER;
    nodeConfig.Init.SourceBlockAddressOffset = 0;
    nodeConfig.Init.DestBlockAddressOffset = 0;
    nodeConfig.BlockCount = 1;
    nodeConfig.Init.Priority     = MDMA_PRIORITY_HIGH;
    nodeConfig.Init.Endianness     = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    nodeConfig.Init.Request        = MDMA_REQUEST_SW;
    nodeConfig.Init.SourceInc      = MDMA_SRC_INC_BYTE;
    nodeConfig.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    nodeConfig.Init.SourceDataSize = MDMA_SRC_INC_BYTE;
    nodeConfig.Init.DestDataSize = MDMA_DEST_INC_BYTE;
    nodeConfig.BlockDataLength     = 1;
    nodeConfig.SrcAddress = (uint32_t) instance.data_buffer;
    if(destination_address == nullptr)
    {
        nodeConfig.DstAddress  = (uint32_t) instance.data_buffer;
    }
    else
    {
    nodeConfig.DstAddress = (uint32_t) destination_address;
    }
    
    HAL_StatusTypeDef status = HAL_MDMA_LinkedList_CreateNode(&transfer_node, &nodeConfig);
    if (status != HAL_OK)
    {
        ErrorHandler("Error creating linked list in MDMA");
    }

    return id;
}

void MDMA::start()
{
    __HAL_RCC_MDMA_CLK_ENABLE();

    for(auto& [id, instance] : instances)
    {
        HAL_StatusTypeDef status = HAL_MDMA_Init(&instance.handle);
        if (status != HAL_OK)
        {
            ErrorHandler("Error initialising MDMA instance");
        }
        HAL_MDMA_RegisterCallback(&instance.handle, HAL_MDMA_XFER_CPLT_CB_ID, MDMA::TransferCompleteCallback);

        status = HAL_MDMA_Start_IT(&instance.handle, (uint32_t)instance.data_buffer, (uint32_t)instance.destination_address, 1,1);
        if( status != HAL_OK)
        {
            ErrorHandler("Error starting MDMA instance");
        }
    }
    HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(MDMA_IRQn);

    //demas 
}

template<typename... pointers>
uint8_t MDMA::add_packet(const uint8_t MDMA_id,const std::tuple<pointers...>& values)
{
    Instance& instance = instances[MDMA_id];
    uint32_t offset{0};
    uint8_t i = 0;
    HAL_StatusTypeDef status;
    std::vector<MDMA_LinkNodeTypeDef> nodes;
    MDMA_LinkNodeConfTypeDef nodeConfig;
    nodeConfig.Init.DataAlignment      = MDMA_DATAALIGN_PACKENABLE;
    nodeConfig.Init.SourceBurst         = MDMA_SOURCE_BURST_SINGLE;
    nodeConfig.Init.DestBurst    = MDMA_DEST_BURST_SINGLE;
    nodeConfig.Init.BufferTransferLength = 1;
    nodeConfig.Init.TransferTriggerMode  = MDMA_BLOCK_TRANSFER;
    nodeConfig.Init.SourceBlockAddressOffset = 0;
    nodeConfig.Init.DestBlockAddressOffset = 0;
    nodeConfig.BlockCount = 1;
    nodeConfig.Init.Priority     = MDMA_PRIORITY_HIGH;
    nodeConfig.Init.Endianness     = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    nodeConfig.Init.Request        = MDMA_REQUEST_SW;
    nodeConfig.Init.SourceInc     = MDMA_SRC_INC_DISABLE;
    nodeConfig.Init.DestinationInc      = MDMA_DEST_INC_DISABLE;
    std::apply([&](auto... ptrs) 
    {
        auto create_node = [&](auto ptr)
        {
            if (ptr == nullptr) {
                ErrorHandler("Nullptr given to MDMA");
            }

            using PointerType = std::decay_t<decltype(ptr)>;
            using UnderlyingType = std::remove_pointer_t<PointerType>;
            constexpr size_t type_size = sizeof(UnderlyingType);

            static_assert(type_size == 1 || type_size == 2 || type_size == 4, 
                "MDMA::add_packet: Passed a variable with type size > 4 ");

            MDMA_LinkNodeTypeDef node = {};
            nodeConfig.SrcAddress = (uint32_t)ptr;
            nodeConfig.DstAddress  = (uint32_t)instance.data_buffer + offset;
            nodeConfig.BlockDataLength     = type_size;
            nodeConfig.Init.SourceDataSize      = src_size_to_flags[type_size];
            nodeConfig.Init.DestDataSize = dst_size_to_flags[type_size];
            
            
            status = HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig);
            if (status != HAL_OK)
            {
                ErrorHandler("Error creating linked list in MDMA");
            }
            offset += type_size;
            nodes.push_back(node);
        };

        create_node(ptrs...);
        if(i !=0){
            status =HAL_MDMA_LinkedList_AddNode(&instance.handle, &nodes[i-1], &nodes[i]);
            if (status != HAL_OK)
            {
                ErrorHandler("Error creating linked list in MDMA");
            }
        }
        i++;
    
    }, values);

    if (nodes.empty()) 
    {
        ErrorHandler("Error creating linked list in MDMA");
    }

    if(instance.destination_address == nullptr)
    {
    MDMA_LinkNodeTypeDef node = {};
    nodeConfig.Init.SourceInc      = MDMA_SRC_INC_BYTE;
    nodeConfig.Init.DestinationInc = MDMA_DEST_INC_BYTE;
    nodeConfig.Init.SourceDataSize = MDMA_SRC_INC_BYTE;
    nodeConfig.Init.DestDataSize = MDMA_DEST_INC_BYTE;
    nodeConfig.BlockDataLength     = offset;
    nodeConfig.SrcAddress = (uint32_t) instance.data_buffer;
    nodeConfig.DstAddress = (uint32_t) instance.data_buffer + offset;
    
    status = HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig);
    if (status != HAL_OK)
    {
        ErrorHandler("Error creating linked list in MDMA");
    }
    nodes.push_back(node);
    status = HAL_MDMA_LinkedList_AddNode(&instance.handle, &nodes[nodes.size()-2], &nodes[nodes.size()-1]);
    if (status != HAL_OK)
    {
        ErrorHandler("Error creating linked list in MDMA");
    }
    }

    linked_lists[number_of_packets++] = nodes;
    return number_of_packets;
}

uint8_t MDMA::merge_packets(const uint8_t base_packet_id, const auto... packets_id)
{
    std::vector<MDMA_LinkNodeTypeDef> merged_nodes = linked_lists[base_packet_id];
    uint32_t offset = 0;
    for(const auto& node : merged_nodes)
    {
        offset += node.CBNDTR;
    }

    merged_nodes.pop_back();  //Remove last auxilary node
    std::array<uint8_t, sizeof...(packets_id)> packet_ids{static_cast<uint8_t>(packets_id)...};
    for (size_t idx = 0; idx < packet_ids.size(); ++idx) {
        uint8_t pid = packet_ids[idx];
        std::vector<MDMA_LinkNodeTypeDef> nodes_to_merge = linked_lists[pid];
        for(auto& node : nodes_to_merge)
        {
            node.CSAR = node.CSAR + offset;
            offset += node.CBNDTR;
        }

        if (idx != packet_ids.size() - 1) {
            nodes_to_merge.pop_back();
        }

        nodes_to_merge.back().CLAR = 0; 
        merged_nodes.back().CLAR = (uint32_t)&nodes_to_merge[0];
        merged_nodes.insert(merged_nodes.end(), nodes_to_merge.begin(), nodes_to_merge.end());
    }

    linked_lists[number_of_packets++] = merged_nodes;
    return number_of_packets;
}

void MDMA::transfer_packet(const uint8_t MDMA_id, const uint8_t packet_id,uint8_t* destination_address,Promise* promise)
{
    Instance& instance = instances[MDMA_id];

    while((instance.handle.Instance->CISR &  MDMA_CISR_CRQA) != 0U)
    { //Active wait in case there is an error with the sinchronization
        __NOP();
    }

    if(promise == nullptr)
    {
        instance.using_promise = false;
    }
    else
    {
        instance.promise = promise;
        instance.using_promise = true;
    }

    if(destination_address != nullptr)
    {
        if(instance.destination_address == nullptr)
        {
            ErrorHandler("No destination address provided for MDMA transfer");
        }
        std::vector<MDMA_LinkNodeTypeDef>& nodes = linked_lists[packet_id];
        auto& packet_transfer_node =nodes.back();
        packet_transfer_node.CDAR = (uint32_t)destination_address;
    }
    
    instance.handle.Instance->CLAR = (uint32_t)&linked_lists[packet_id][0];
    HAL_MDMA_GenerateSWRequest(&instance.handle);
}

void MDMA::transfer_data(const uint8_t MDMA_id,uint8_t* source_address, const uint32_t data_length,uint8_t* destination_address,Promise* promise)
{
    Instance& instance = instances[MDMA_id];

    while((instance.handle.Instance->CISR &  MDMA_CISR_CRQA) != 0U)
    { //Active wait in case there is an error with the sinchronization
        __NOP();
    }

    if(promise == nullptr)
    {
        instance.using_promise = false;
    }
    else
    {
        instance.promise = promise;
        instance.using_promise = true;
    }

    transfer_node.CSAR = (uint32_t)source_address;
    transfer_node.CBNDTR = data_length;
    if(destination_address == nullptr)
    {
        if(instance.destination_address ==nullptr)
        {
            ErrorHandler("No destination address provided for MDMA transfer");
        }
        transfer_node.CDAR = (uint32_t)instance.destination_address;
    }
    else
    {
        transfer_node.CDAR = (uint32_t)destination_address;
    }

    instance.handle.Instance->CLAR = (uint32_t)&transfer_node;

    HAL_MDMA_GenerateSWRequest(&instance.handle);

}


void MDMA::TransferCompleteCallback(MDMA_HandleTypeDef *hmdma)
{
	Instance& instance = instances[channel_to_instance[hmdma->Instance]];
    if(instance.using_promise)
    {
        instance.promise->resolve();
        instance.using_promise = false;
    }
}



