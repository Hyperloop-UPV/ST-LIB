/*
 * SdLogger.hpp
 *
 * Created on: 16 dec. 2025
 *         Author: Boris
 */

#ifndef SDLOGGER_HPP
#define SDLOGGER_HPP

#include "ST-LIB_LOW/ST-LIB_LOW.hpp"
#include "HALAL/HALAL.hpp"

template<class SdWrapper>
class SdLogger {
   public:
    static constexpr size_t MAX_PACKETS = 32;

    SdLogger(SdWrapper& sd_card) : sd(sd_card) {
        sd.init_card();
        current_buffer_ptr = reinterpret_cast<uint8_t*>(sd.get_current_buffer()->data());
        current_buffer_size = sd.get_current_buffer()->size() * 4; // uint32_t is 4B
        current_buffer_blocks = current_buffer_size / 512;
    }

    uint8_t add_packet(MdmaPacketDomain::MdmaPacketBase* packet) {
        if (packet_count >= MAX_PACKETS) {
            ErrorHandler("Max packets reached in SdLogger");
            return 0;
        }
        packets[packet_count] = packet;
        packet_sizes[packet_count] = packet->get_size(); // Cache size to avoid virtual call in update
        packet_count++;
        return packet_count - 1;
    }

    void log(uint16_t packet_index) {
        request_mask |= (1UL << packet_index);
    }

    void update() {
        /* Sd update logic */
        if (sd_write_ongoing) {
            if (sd_write_complete_flag) {
                sd_write_ongoing = false;
                sd_write_complete_flag = false;
            }
        }

        /* Packets building update logic */
        if (building_mask) {
            uint32_t temp_mask = building_mask;
            while (temp_mask) { // Iterate over the packets being build and check if they've been built already
                uint32_t i = __builtin_ctz(temp_mask);
                if (build_complete_flags[i]) {
                    building_mask &= ~(1UL << i);
                    build_complete_flags[i] = false;
                }
                temp_mask &= ~(1UL << i);
            }
        }

        /* Buffer management update logic */

        // Normal buffer operation
        if (!buffer_flush_pending) {
            uint32_t temp_mask = request_mask;
            while (temp_mask) { // Iterate over all requested to be built packets
                uint32_t i = __builtin_ctz(temp_mask);
                temp_mask &= ~(1UL << i);

                // If it is already being built, don't add it once more (your program is kinda slow then though, should reconsider what you're doing)
                if (building_mask & (1UL << i)) {
                    continue; 
                }

                size_t packet_size = packet_sizes[i];
                // Don't add more packets if they can't fit (this could be optimized to test if there are other packets smaller that can fit, but that's more cycles, this works)
                if (current_buffer_offset + packet_size > current_buffer_size - 2) {
                    buffer_flush_pending = true;

                    // Put "0xFFFF" end of log marker (as if it were the index, but it obviously won't match any real packet)
                    *(uint16_t*)(current_buffer_ptr + current_buffer_offset) = 0xFFFF;
                    break; 
                }

                request_mask &= ~(1UL << i);
                building_mask |= (1UL << i);
                build_complete_flags[i] = false;
                
                packets[i]->build(&build_complete_flags[i], current_buffer_ptr + current_buffer_offset);
                current_buffer_offset += packet_size;
            }
        }
        
        // Have to flush and all packets have been built so we can flush
        if (buffer_flush_pending && building_mask == 0) {
            if (flush_buffer()) {
                buffer_flush_pending = false;
            }
        }
    }

    uint32_t get_current_block() {
        return sd_block_addr;
    }

   private:
    bool flush_buffer() {
        if (sd_write_ongoing) {
            return false;
        }
        
        sd_write_complete_flag = false;
    
        if (!sd.write_blocks(sd_block_addr, current_buffer_blocks, &sd_write_complete_flag)) {
            return false;
        } else {
            sd_write_ongoing = true;
            sd_block_addr += current_buffer_blocks;
            current_buffer_ptr = reinterpret_cast<uint8_t*>(sd.get_current_buffer()->data());
            current_buffer_offset = 0;
            return true;
        }
    }

    SdWrapper& sd;
    std::array<MdmaPacketDomain::MdmaPacketBase*, MAX_PACKETS> packets;
    size_t packet_sizes[MAX_PACKETS];
    uint8_t packet_count = 0;
    uint32_t request_mask = 0;
    
    bool sd_write_ongoing = false;
    bool sd_write_complete_flag = false;
    
    uint32_t building_mask = 0;
    volatile bool build_complete_flags[MAX_PACKETS];
    
    uint8_t* current_buffer_ptr = nullptr;
    uint32_t current_buffer_size = 0;
    uint32_t current_buffer_blocks = 0;
    uint32_t current_buffer_offset = 0;
    
    bool buffer_flush_pending = false;
    uint32_t sd_block_addr = 0;
};

#endif // SDLOGGER_HPP