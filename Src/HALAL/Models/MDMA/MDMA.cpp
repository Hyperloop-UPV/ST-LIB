#include "HALAL/Models/MDMA/MDMA.hpp"

std::unordered_map<std::vector<MDMA_LinkNodeTypeDef>,uint8_t> MDMA::linked_lists = {};
std::unordered_map<MDMA::Instance, uint8_t> MDMA::instances = {};

uint8_t MDMA::inscribe(uint8_t* data_buffer)
{
    MDMA_HandleTypeDef mdma_handle = {};
    mdma_handle.Instance = MDMA_Channel0;
    mdma_handle.Init.Request = MDMA_REQUEST_SW; 
    mdma_handle.Init.TriggerMode = MDMA_TRIGGERMODE_HW_LINK; 
    mdma_handle.Init.Priority = MDMA_PRIORITY_HIGH;
    mdma_handle.Init.Endianness = MDMA_ENDIANNESS_LITTLE;
    mdma_handle.Init.DataMux = MDMA_DATA_MUX_ENABLED;      
    mdma_handle.Init.DataAlign = MDMA_DATA_ALIGN_PACKENABLE; 
    mdma_handle.Init.BufferTransferLength = 248; 
    mdma_handle.Init.TransferEndMode = MDMA_BUFFER_TRANSFER;
    mdma_handle.Init.SourceDataSize = MDMA_DATA_SIZE_BYTE;
    mdma_handle.Init.DestDataSize = MDMA_DATA_SIZE_BYTE;
    mdma_handle.Init.SourceInc = MDMA_SOURCE_INC_BYTE;
    mdma_handle.Init.DestinationInc = MDMA_DEST_INC_BYTE;

    uint8_t id = instances.size();
    Instance instance(mdma_handle, id, data_buffer);
    instances[instance] = id;

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
    Instance& intance = instances[MDMA_id];
    uint8_t i{0}
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

            static_assert(tipo_size == 1 || tipo_size == 2 || tipo_size == 4, 
                "MDMA::add_packet: Passed a variable with type size > 4 ");

            MDMA_LinkNodeTypeDef node = {};
            node.CTCR = get_ctcr_flags(tipo_size);
            node.CBNDTR = 1;
            node.CSAR = (uint32_t)ptr; 
            node.CDAR = instance.data_buffer; 
            if(i==0)
            {
                node.CLAR = 0; 
            }
            node.CLAR = (uint32_t)&(m_nodes[i-1]);
            i++;
            m_nodes.push_back(node);
        };

        create_node(ptrs);

    }, values);

    if (m_nodes.empty()) 
    {
        ErrorHandler("Error creating linked list in MDMA")
    }

    m_nodes.back().CLAR = 0;
    m_nodes[0].CLAR = (uint32_t)&(m_nodes[1]);


    return number_of_packets++;
}

const uint32_t MDMA::get_flag(const uint8_t size)
{
    uint32_t flags = MDMA_TRANSFER_TRIGGER_MODE_LINK | 
                       MDMA_SOURCE_INC_DISABLE;     

    switch (size) {
        case 1: 
            flags |= (MDMA_DATA_SIZE_BYTE | MDMA_DEST_INC_BYTE);
            break;
        case 2: 
            flags |= (MDMA_DATA_SIZE_HALFWORD | MDMA_DEST_INC_HALFWORD);
            break;
        case 4: 
            flags |= (MDMA_DATA_SIZE_WORD | MDMA_DEST_INC_WORD);
            break;
        default:
            flags |= (MDMA_DATA_SIZE_BYTE | MDMA_DEST_INC_BYTE);
            break;
    }
    return flags;
}