/*
 * MPU.hpp
 *
 *  Created on: 10 dic. 2025
 *      Author: Boris
 */

#ifndef MPU_HPP
#define MPU_HPP

#ifndef HALAL_MPUBUFFERS_MAX_INSTANCES
#define HALAL_MPUBUFFERS_MAX_INSTANCES 100
#endif

#include "C++Utilities/CppUtils.hpp"
#include "C++Utilities/CppImports.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "stm32h7xx_hal.h"


struct MPUDomain {

    enum class MemoryDomain : uint8_t {
        D1 = 1, // AXI SRAM (0x24000000)
        D2 = 2, // SRAM 1/2/3 (0x30000000)
        D3 = 3  // SRAM 4 (0x38000000)
    };

    enum class MemoryType : bool {
        Cached = true,
        NonCached = false
    };

    // Buffer Request
    struct Entry {
        MemoryDomain memory_domain;
        MemoryType memory_type;
        std::size_t alignment;
        std::size_t size_in_bytes;
    };

    // Buffer Request Wrapper
    template <typename T>
    struct Buffer {
        using domain = MPUDomain;
        using buffer_type = T;
        Entry e;

        /**
         * @brief Constructs a Buffer entry for a type T.
         * @tparam T The type for which the buffer is requested. Must be a POD type.
         * @param type The memory type (Cached or NonCached).
         * @param domain The memory domain where the buffer should be allocated.
         * @param force_cache_alignment If true, forces the buffer to be cache line aligned (32 bytes, takes the rest as padding).
         */
        consteval Buffer(MemoryType type = MemoryType::NonCached, MemoryDomain domain = MemoryDomain::D2, bool force_cache_alignment = false)
            requires(std::is_standard_layout_v<T> && std::is_trivial_v<T>)
        : e{
            domain,
            type,
            force_cache_alignment ? 32 : alignof(T),
            sizeof(T)}
        {
            static_assert(alignof(T) <= 32, "Requested type has alignment greater than cache line size (32 bytes).");
        }

        template <class Ctx>
        consteval void inscribe(Ctx &ctx) const {
            ctx.template add<MPUDomain>(e);
        }
    };


    static constexpr std::size_t max_instances = HALAL_MPUBUFFERS_MAX_INSTANCES;

    struct Config {
        uint32_t offset;        // Offset relative to the start of the domain buffer
        std::size_t size;
        MemoryDomain domain;
        
        // MPU Configuration Data
        bool is_mpu_leader;     // If true, this entry triggers an MPU region config with the below params
        uint8_t mpu_number;     // MPU Region Number
        uint8_t mpu_size;       // Encoded Size (e.g. MPU_REGION_SIZE_256B)
        std::size_t mpu_region_size; // Actual size in bytes of the MPU region
        uint8_t mpu_subregion;  // Subregion Disable Mask
    };

    // Helper to calculate sizes needed for static arrays
    struct DomainSizes {
        size_t d1_total = 0;
        size_t d2_total = 0;
        size_t d3_total = 0;
        
        size_t d1_nc_size = 0;
        size_t d2_nc_size = 0;
        size_t d3_nc_size = 0;
    };

    static consteval uint32_t align_up(uint32_t val, size_t align) {
        return (val + align - 1) & ~(align - 1); // Align up to 'align' leaving the minimum padding
    }


    static consteval DomainSizes calculate_total_sizes(std::span<const Config> configs) {
        DomainSizes sizes;
        for (const auto& cfg : configs) {
            size_t end = cfg.offset + cfg.size;
            if (cfg.domain == MemoryDomain::D1) sizes.d1_total = std::max(sizes.d1_total, end);
            else if (cfg.domain == MemoryDomain::D2) sizes.d2_total = std::max(sizes.d2_total, end);
            else if (cfg.domain == MemoryDomain::D3) sizes.d3_total = std::max(sizes.d3_total, end);

            if (cfg.is_mpu_leader) {
                if (cfg.domain == MemoryDomain::D1) sizes.d1_nc_size = cfg.mpu_region_size;
                else if (cfg.domain == MemoryDomain::D2) sizes.d2_nc_size = cfg.mpu_region_size;
                else if (cfg.domain == MemoryDomain::D3) sizes.d3_nc_size = cfg.mpu_region_size;
            }
        }
        // Align totals to 32 bytes
        sizes.d1_total = align_up(sizes.d1_total, 32);
        sizes.d2_total = align_up(sizes.d2_total, 32);
        sizes.d3_total = align_up(sizes.d3_total, 32);
        return sizes;
    }

    template <std::size_t N>
    static consteval std::array<Config, N> build(std::span<const Entry> entries) {
        std::array<Config, N> cfgs{};
        
        uint32_t offsets[3] = {}; // D1, D2, D3
        uint32_t assigned_offsets[N]; 

        size_t alignments[] = {32, 16, 8, 4, 2, 1};
        
        /* Non-Cached Offsets */
        for (size_t align : alignments) {
            for (size_t i = 0; i < N; i++) {
                if (entries[i].memory_type == MemoryType::NonCached && entries[i].alignment == align) {
                    size_t d_idx = static_cast<size_t>(entries[i].memory_domain) - 1;
                    offsets[d_idx] = align_up(offsets[d_idx], align);
                    assigned_offsets[i] = offsets[d_idx];
                    offsets[d_idx] += entries[i].size_in_bytes;
                }
            }
        }

        // Capture Non-Cached Sizes for MPU and adjust offsets for Cached data
        size_t nc_sizes[3];
        for(int i=0; i<3; i++) {
            if (offsets[i] > 0) {
                auto [r_size, r_sub] = get_size_needed(offsets[i]);
                nc_sizes[i] = offsets[i];
                
                // Move the offset pointer to the end of the MPU region block to have Cached data start after it
                offsets[i] = r_size / 8 * (8 - std::popcount(r_sub)); // Effective used size considering subregions disabled
                offsets[i] = align_up(offsets[i], 32); // Align to 32 bytes just in case
            } else {
                nc_sizes[i] = 0;
            }
        }

        /* Cached Offsets */
        for (size_t align : alignments) {
            for (size_t i = 0; i < N; i++) {
                if (entries[i].memory_type == MemoryType::Cached && entries[i].alignment == align) {
                    size_t d_idx = static_cast<size_t>(entries[i].memory_domain) - 1;
                    offsets[d_idx] = align_up(offsets[d_idx], align);
                    assigned_offsets[i] = offsets[d_idx];
                    offsets[d_idx] += entries[i].size_in_bytes;
                }
            }
        }

        /* Build Configs */
        bool domain_configured[3] = {false, false, false};

        for (std::size_t i = 0; i < N; i++) {
            cfgs[i].size = entries[i].size_in_bytes;
            cfgs[i].domain = entries[i].memory_domain;
            cfgs[i].offset = assigned_offsets[i];
            cfgs[i].is_mpu_leader = false;
            cfgs[i].mpu_region_size = 0;

            if (entries[i].memory_type == MemoryType::NonCached) {
                size_t d_idx = static_cast<size_t>(entries[i].memory_domain) - 1;
                if (!domain_configured[d_idx]) {
                    // This entry is the "Leader" responsible for configuring the MPU region for the whole domain
                    cfgs[i].is_mpu_leader = true;
                    domain_configured[d_idx] = true;

                    auto [r_size, r_sub] = get_size_needed(nc_sizes[d_idx]);
                    cfgs[i].mpu_size = get_region_size_encoding(r_size);
                    cfgs[i].mpu_subregion = r_sub;
                    cfgs[i].mpu_region_size = r_size; // Store for Init alignment
                    cfgs[i].mpu_number = (d_idx == 0) ? MPU_REGION_NUMBER3 : 
                                         (d_idx == 1) ? MPU_REGION_NUMBER5 : MPU_REGION_NUMBER7;
                }
            }
        }

        return cfgs;
    }

    struct Instance {
        void* ptr;
        std::size_t size;

        template <typename T, typename... Args>
        T* construct(Args&&... args) {
            validate<T>();
            return new (ptr) T(std::forward<Args>(args)...);
        }

        template <typename T = void>
        T* as() {
            validate<T>();
            return static_cast<T*>(ptr);
        }

       private:
        template <typename T>
        void validate() {
            if (sizeof(T) != size) ErrorHandler("MPU: Buffer size mismatch.");
            if (reinterpret_cast<uintptr_t>(ptr) % alignof(T) != 0) ErrorHandler("MPU: Buffer alignment mismatch.");
        }
    };

    
    template <std::size_t N, std::array<Config, N> cfgs>
    struct Init {
        static inline std::array<Instance, N> instances{};
        
        // Calculate sizes at compile time from the template parameter
        static constexpr auto Sizes = calculate_total_sizes(cfgs);

        // --- Actual Storage (Placed by Linker) ---
        // These sections must be defined in the Linker Script.
        // They will be placed automatically, avoiding conflicts with other data.
        // Alignment must match the MPU region size for Non-Cached areas.
        static constexpr size_t d1_align = Sizes.d1_nc_size > 0 ? Sizes.d1_nc_size : 32;
        static constexpr size_t d2_align = Sizes.d2_nc_size > 0 ? Sizes.d2_nc_size : 32;
        static constexpr size_t d3_align = Sizes.d3_nc_size > 0 ? Sizes.d3_nc_size : 32;

        __attribute__((section(".mpu_ram_d1_nc"))) alignas(d1_align)
        static inline uint8_t d1_nc_buffer[Sizes.d1_total > 0 ? Sizes.d1_total : 1];
        __attribute__((section(".mpu_ram_d2_nc"))) alignas(d2_align) 
        static inline uint8_t d2_nc_buffer[Sizes.d2_total > 0 ? Sizes.d2_total : 1];
        __attribute__((section(".mpu_ram_d3_nc"))) alignas(d3_align)
        static inline uint8_t d3_nc_buffer[Sizes.d3_total > 0 ? Sizes.d3_total : 1];

        static void init() {
            HAL_MPU_Disable();
            configure_static_regions();

            uint8_t* bases[3] = { &d1_nc_buffer[0], &d2_nc_buffer[0], &d3_nc_buffer[0] };

            for (std::size_t i = 0; i < N; i++) {
                const auto &cfg = cfgs[i];
                auto &inst = instances[i];

                if (cfg.domain == MemoryDomain::D1 || cfg.domain == MemoryDomain::D2 || cfg.domain == MemoryDomain::D3) {
                    size_t d_idx = static_cast<size_t>(cfg.domain) - 1;
                    // Calculate absolute address: Base + Offset
                    inst.ptr = bases[d_idx] + cfg.offset;
                    inst.size = cfg.size;

                    if (cfg.is_mpu_leader) {
                        MPU_Region_InitTypeDef init = {0};
                        init.Enable = MPU_REGION_ENABLE;
                        init.Number = cfg.mpu_number;
                        init.BaseAddress = (uint32_t)bases[d_idx]; // Base of the whole buffer
                        init.Size = cfg.mpu_size;
                        init.SubRegionDisable = cfg.mpu_subregion;
                        init.TypeExtField = MPU_TEX_LEVEL1; // Normal, Non-Cached
                        init.AccessPermission = MPU_REGION_FULL_ACCESS;
                        init.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
                        init.IsShareable = MPU_ACCESS_SHAREABLE;
                        init.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
                        init.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
                        HAL_MPU_ConfigRegion(&init);
                    }
                }
            }

            HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
            SCB_EnableICache();
            SCB_EnableDCache();
        }
    };

   private:
    static consteval std::pair<std::size_t, std::uint8_t> get_size_needed(std::size_t size) {
        if (size == 0) return {32, 0xFF};
        size_t power = std::bit_width(size);
        if (power < 5) power = 5; // Min 32B
        size_t subregion_size = 1U << (power - 3);
        size_t num_subregions = (size + subregion_size - 1) / subregion_size;
        uint8_t subregion_disable = static_cast<uint8_t>(~((1U << num_subregions) - 1));
        return {(1U << power), subregion_disable};
    }

    static consteval uint8_t get_region_size_encoding(std::size_t size) {
        if (size <= 32) return MPU_REGION_SIZE_32B;
        if (size <= 64) return MPU_REGION_SIZE_64B;
        if (size <= 128) return MPU_REGION_SIZE_128B;
        if (size <= 256) return MPU_REGION_SIZE_256B;
        if (size <= 512) return MPU_REGION_SIZE_512B;
        if (size <= 1024) return MPU_REGION_SIZE_1KB;
        if (size <= 2048) return MPU_REGION_SIZE_2KB;
        if (size <= 4096) return MPU_REGION_SIZE_4KB;
        if (size <= 8192) return MPU_REGION_SIZE_8KB;
        if (size <= 16384) return MPU_REGION_SIZE_16KB;
        if (size <= 32768) return MPU_REGION_SIZE_32KB;
        if (size <= 65536) return MPU_REGION_SIZE_64KB;
        if (size <= 131072) return MPU_REGION_SIZE_128KB;
        if (size <= 262144) return MPU_REGION_SIZE_256KB;
        if (size <= 524288) return MPU_REGION_SIZE_512KB;
        if (size <= 1048576) return MPU_REGION_SIZE_1MB;
        if (size <= 2097152) return MPU_REGION_SIZE_2MB;
        if (size <= 4194304) return MPU_REGION_SIZE_4MB;
        return MPU_REGION_SIZE_4GB;
    }

    static void configure_static_regions() {
        MPU_Region_InitTypeDef MPU_InitStruct = {0};
        
        // Background (No Access)
        MPU_InitStruct.Enable = MPU_REGION_ENABLE;
        MPU_InitStruct.Number = MPU_REGION_NUMBER0;
        MPU_InitStruct.BaseAddress = 0x0;
        MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
        MPU_InitStruct.SubRegionDisable = 0x87;
        MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
        MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
        MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
        MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);

        // Flash (Non-cached, Executable)
        MPU_InitStruct.Enable = MPU_REGION_ENABLE;
        MPU_InitStruct.Number = MPU_REGION_NUMBER1;
        MPU_InitStruct.BaseAddress = 0x08000000;
        MPU_InitStruct.Size = MPU_REGION_SIZE_1MB;
        MPU_InitStruct.SubRegionDisable = 0x0;
        MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
        MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
        MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE; // This should be scrutinized to see why it was non-cacheable before changing to cacheable
        MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);

        // D1 RAM (Cached)
        MPU_InitStruct.Enable = MPU_REGION_ENABLE;
        MPU_InitStruct.Number = MPU_REGION_NUMBER2;
        MPU_InitStruct.BaseAddress = 0x24000000;
        MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
        MPU_InitStruct.SubRegionDisable = 0xE0; // Only 320KB available
        MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
        MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
        MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
        MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);

        // D2 RAM (Cached)
        MPU_InitStruct.Enable = MPU_REGION_ENABLE;
        MPU_InitStruct.Number = MPU_REGION_NUMBER4;
        MPU_InitStruct.BaseAddress = 0x30000000;
        MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
        MPU_InitStruct.SubRegionDisable = 0x0;
        MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
        MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
        MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
        MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);

        // Ethernet Descriptors (D2 Base) - Legacy, should change Ethernet driver to use MPU buffers
        MPU_InitStruct.Enable = MPU_REGION_ENABLE;
        MPU_InitStruct.Number = MPU_REGION_NUMBER8;
        MPU_InitStruct.BaseAddress = 0x30000000;
        MPU_InitStruct.Size = MPU_REGION_SIZE_512B;
        MPU_InitStruct.SubRegionDisable = 0x0;
        MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
        MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
        MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
        MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE; // Device
        HAL_MPU_ConfigRegion(&MPU_InitStruct);

        // D3 RAM (Cached)
        MPU_InitStruct.Enable = MPU_REGION_ENABLE;
        MPU_InitStruct.Number = MPU_REGION_NUMBER6; // FIX: Changed from 5 to 6 to avoid collision with D2 NC
        MPU_InitStruct.BaseAddress = 0x38000000;
        MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
        MPU_InitStruct.SubRegionDisable = 0x0;
        MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
        MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
        MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
        MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);

        /**
         * Other regions are:
         * 3. D1 RAM (Non-Cached) - Configured dynamically
         * 5. D2 RAM (Non-Cached) - Configured dynamically
         * 7. D3 RAM (Non-Cached) - Configured dynamically
         */

        /**
         * Other memory areas (not configured explicitly):
         * - Peripheral space (0x40000000 - 0x5FFFFFFF): Defaults to device memory
         */
    }
};

#endif // MPU_HPP
