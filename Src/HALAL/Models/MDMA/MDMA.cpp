#include "HALAL/Models/MDMA/MDMA.hpp"

std::unordered_map<std::vector<MDMA_LinkNodeTypeDef>,uint8_t> MDMA::linked_lists = {};
std::unordered_map<MDMA::Instance, uint8_t> MDMA::instances = {};
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

uint8_t MDMA::inscribe(uint8_t* data_buffer)
{
    MDMA_HandleTypeDef mdma_handle = {};
    mdma_handle.Instance = MDMA_Channel0;
    mdma_handle.Init.Request = MDMA_REQUEST_SW; 

    uint8_t id = instances.size();
    Instance instance(mdma_handle, id, data_buffer);
    instances[id] = instance;

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
    std::vector<MDMA_LinkNodeTypeDef> m_nodes;
    MDMA_LinkNodeConfTypeDef nodeConfig;
    std::apply([&](auto... ptrs) 
    {
        auto create_node = [&](auto ptr)
        {
            if (ptr == nullptr) {
                ErrorHandler("Nullptr given to MDMA")
            }

            using PointerType = std::decay_t<decltype(ptr)>;
            using UnderlyingType = std::remove_pointer_t<PointerType>;
            constexpr size_t type_size = sizeof(UnderlyingType);

            static_assert(type_size == 1 || type_size == 2 || type_size == 4, 
                "MDMA::add_packet: Passed a variable with type size > 4 ");

            MDMA_LinkNodeTypeDef node = {};
            nodeConfig.SourceAddress       = (uint32_t)ptr;
            nodeConfig.DestinationAddress  = (uint32_t)instance.data_buffer + offset;
            nodeConfig.BlockDataLength     = type_size;
            nodeConfig.SourceInc           = MDMA_SRC_INC_DISABLE;
            nodeConfig.DestinationInc      = MDMA_DEST_INC_DISABLE;
            nodeConfig.SourceDataSize      = src_size_to_flags[type_size];
            nodeConfig.DestinationDataSize = dst_size_to_flags[type_size];
            nodeConfig.SourceBurst         = MDMA_SOURCE_BURST_SINGLE;
            nodeConfig.DestinationBurst    = MDMA_DESTINATION_BURST_SINGLE;
            
            HAL_StatusTypeDef status = HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig);
            if (status != HAL_OK)
            {
                ErrorHandler("Error creating linked list in MDMA");
            }
            offset += type_size;
            m_nodes.push_back(node);
        };

        create_node(ptrs);
        if(i !=0){
            HAL_MDMA_LinkedList_AddNode(&instance.handle, &m_nodes[i-1], &m_nodes[i]);
        }
        i++;
    
    }, values);

    if (m_nodes.empty()) 
    {
        ErrorHandler("Error creating linked list in MDMA");
    }

    MDMA_LinkNodeTypeDef node = {};
    nodeConfig.SourceAddress       = (uint32_t)instance.data_buffer;
    nodeConfig.DestinationAddress  = (uint32_t)instance.data_buffer + offset;
    nodeConfig.BlockDataLength     = offset;
    nodeConfig.SourceInc           = MDMA_SRC_INC_ENABLE;
    nodeConfig.DestinationInc      = MDMA_DEST_INC_ENABLE;
    nodeConfig.SourceDataSize      = MDMA_DATA_SIZE_BYTE;
    nodeConfig.DestinationDataSize = MDMA_DATA_SIZE_BYTE;
    nodeConfig.SourceBurst         = MDMA_SOURCE_BURST_SINGLE;
    nodeConfig.DestinationBurst    = MDMA_DESTINATION_BURST_SINGLE;
    
    HAL_StatusTypeDef status = HAL_MDMA_LinkedList_CreateNode(&node, &nodeConfig);
    if (status != HAL_OK)
    {
        ErrorHandler("Error creating linked list in MDMA");
    }
    m_nodes.push_back(node);
    HAL_MDMA_LinkedList_AddNode(&instance.handle, &m_nodes[m_nodes.size()-2], &m_nodes[m_nodes.size()-1]);

    linked_lists[number_of_packets++] = m_nodes;
    return number_of_packets;
}
