/*
 * SdLogger.hpp
 *
 * Created on: 16 dec. 2025
 *         Author: Boris
 */

#pragma once

#include "ST-LIB_LOW/ST-LIB_LOW.hpp"
#include "HALAL/HALAL.hpp"
#include "Sd/Sd.hpp"
#include "HALAL/Models/Packets/MdmaPacket.hpp"
#include <array>

template<class SdWrapper>
class SdLogger {
public:
    static constexpr size_t MAX_PACKETS = 32;

    SdLogger(SdWrapper& sd_card) : sd(sd_card) {
        sd.init_card();
        current_buffer_ptr = reinterpret_cast<uint8_t*>(sd.get_current_buffer()->data());
        current_buffer_size = sd.get_current_buffer()->size() * 4;
    }

    void add_packet(MdmaPacketDomain::MdmaPacketBase* packet) {
        if (packet_count >= MAX_PACKETS) {
            ErrorHandler("Max packets reached in SdLogger");
            return;
        }
        packets[packet_count] = packet;
        packet_count++;
    }

    void log(uint16_t packet_index) {
        request_mask |= (1UL << packet_index);
    }

    void update() {
        if (sd_write_ongoing) {
            if (sd_write_complete_flag) {
                sd_write_ongoing = false;
                sd_write_complete_flag = false;
            }
        }

        if (building_mask) {
            uint32_t temp_mask = building_mask;
            while (temp_mask) {
                uint32_t i = __builtin_ctz(temp_mask);
                if (build_complete_flags[i]) {
                    building_mask &= ~(1UL << i);
                    build_complete_flags[i] = false;
                }
                temp_mask &= ~(1UL << i);
            }
        }

        if (!buffer_flush_pending) {
            uint32_t temp_mask = request_mask;
            while (temp_mask) {
                uint32_t i = __builtin_ctz(temp_mask);
                temp_mask &= ~(1UL << i);

                size_t packet_size = packets[i]->get_size();
                if (current_buffer_offset + packet_size > current_buffer_size) {
                    buffer_flush_pending = true;
                    break; 
                }

                if (building_mask & (1UL << i)) {
                    continue; 
                }

                request_mask &= ~(1UL << i);
                building_mask |= (1UL << i);
                build_complete_flags[i] = false;
                
                packets[i]->build(&build_complete_flags[i], current_buffer_ptr + current_buffer_offset);
                current_buffer_offset += packet_size;
            }
        }
        
        if (buffer_flush_pending && building_mask == 0) {
             if (flush_buffer()) {
                 buffer_flush_pending = false;
             }
        }
    }

private:
    bool flush_buffer() {
        if (sd_write_ongoing) {
            return false;
        }

        uint32_t bytes_to_write = current_buffer_offset;
        if (bytes_to_write == 0) {
            current_buffer_offset = 0;
            return true;
        }

        uint32_t blocks = (bytes_to_write + 511) / 512;
        
        sd_write_ongoing = true;
        sd_write_complete_flag = false;
        
        if (!sd.write_blocks(sd_block_addr, blocks, &sd_write_complete_flag)) {
            ErrorHandler("Failed to start SD write");
            sd_write_ongoing = false;
            return false;
        } else {
            sd_block_addr += blocks;
            current_buffer_ptr = reinterpret_cast<uint8_t*>(sd.get_current_buffer()->data());
            current_buffer_offset = 0;
            return true;
        }
    }

    SdWrapper& sd;
    std::array<MdmaPacketDomain::MdmaPacketBase*, MAX_PACKETS> packets;
    size_t packet_count = 0;
    uint32_t request_mask = 0;
    
    bool sd_write_ongoing = false;
    bool sd_write_complete_flag = false;
    
    uint32_t building_mask = 0;
    std::array<bool, MAX_PACKETS> build_complete_flags;
    
    uint8_t* current_buffer_ptr = nullptr;
    uint32_t current_buffer_size = 0;
    uint32_t current_buffer_offset = 0;
    
    bool buffer_flush_pending = false;
    uint32_t sd_block_addr = 0;
};
