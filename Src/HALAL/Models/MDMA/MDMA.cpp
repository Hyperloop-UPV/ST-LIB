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

uint8_t MDMA::inscribe(uint8_t* data_buffer,uint8_t* destination_address)
{
    MDMA_HandleTypeDef mdma_handle = {};
    mdma_handle.Instance = MDMA_Channel0;
    mdma_handle.Init.Request = MDMA_REQUEST_SW; 

    uint8_t id = instances.size();
    Instance instance(mdma_handle, id, data_buffer);
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
    nodeConfig.DstAddress = (uint32_t) instance.data_buffer;
    
    HAL_StatusTypeDef status = HAL_MDMA_LinkedList_CreateNode(&transfer_node, &nodeConfig);
    if (status != HAL_OK)
    {
        //ErrorHandler("Error creating linked list in MDMA");
    }

    return id;
}

void MDMA::start()
{
    __HAL_RCC_MDMA_CLK_ENABLE();

    //demas 
}

template<typename... pointers>
uint8_t MDMA::add_packet(const uint8_t MDMA_id,const std::tuple<pointers...>& values)
{
    Instance& instance = instances[MDMA_id];
    uint32_t offset{0};
    uint8_t i = 0;
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
                //ErrorHandler("Nullptr given to MDMA")
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
            
            
            HAL_StatusTypeDef status = HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig);
            if (status != HAL_OK)
            {
                //ErrorHandler("Error creating linked list in MDMA");
            }
            offset += type_size;
            nodes.push_back(node);
        };

        create_node(ptrs...);
        if(i !=0){
            HAL_MDMA_LinkedList_AddNode(&instance.handle, &nodes[i-1], &nodes[i]);
        }
        i++;
    
    }, values);

    if (nodes.empty()) 
    {
        //ErrorHandler("Error creating linked list in MDMA");
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
    
    HAL_StatusTypeDef status = HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig);
    if (status != HAL_OK)
    {
        //ErrorHandler("Error creating linked list in MDMA");
    }
    nodes.push_back(node);
    HAL_MDMA_LinkedList_AddNode(&instance.handle, &nodes[nodes.size()-2], &nodes[nodes.size()-1]);
    }

    linked_lists[number_of_packets++] = nodes;
    return number_of_packets;
}

void MDMA::transfer_packet(const uint8_t MDMA_id, const uint8_t packet_id,uint8_t* destination_address)
{
    Instance& instance = instances[MDMA_id];
    if(destination_address != nullptr)
    {
        std::vector<MDMA_LinkNodeTypeDef>& nodes = linked_lists[packet_id];
        auto transfer_node =nodes.back();
        transfer_node.CDAR = (uint32_t)destination_address;
    }
    //More to do...
}

//Habria que ver si renta esta funcion:
void MDMA::transfer_data(const uint8_t MDMA_id,uint8_t* source_address,uint8_t* destination_address, const uint32_t data_length)
{
    Instance& instance = instances[MDMA_id];

    transfer_node.CSAR = (uint32_t)source_address;
    transfer_node.CDAR = (uint32_t)destination_address;
    transfer_node.CBNDTR = data_length;

    while(false)//Chequear que no esté funcionando);

    HAL_MDMA_LinkedList_AddNode(&instance.handle, nullptr, &transfer_node);

    //Algo....
}