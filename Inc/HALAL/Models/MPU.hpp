/*
 * MPU.hpp
 *
 *  Created on: 10 dic. 2025
 *      Author: Boris
 *
 *  Memory Allocation Strategies:
 *  ==============================
 *  This system supports three distinct memory allocation methods that can coexist:
 *
 *  1. MANUAL ALLOCATION (via macros):
 *     Use D1_NC, D2_NC, D3_NC, D1_C, D2_C, D3_C macros for static/global variables.
 *     Example: D1_NC uint8_t my_buffer[100];
 *     Placed in: .mpu_ram_dX_nc.user or .ram_dX.user sections
 *
 *  2. NEW MPU SYSTEM (compile-time allocation):
 *     Use MPUDomain::Buffer<T> with compile-time buffer management.
 *     Automatically placed and sized by the Domain system.
 *     Placed in: .mpu_ram_dX_nc.buffer or .ram_dX.buffer sections
 *
 *  3. LEGACY MPUManager (runtime allocation):
 *     Use MPUManager::allocate_non_cached_memory() for dynamic allocation.
 *     Currently uses a 2KB pool in D3 non-cached memory.
 *     Placed in: .mpu_ram_d3_nc.legacy section
 *
 *  These three methods are separated in the linker script to prevent overlap.
 *  All memory is properly configured by MPU regions as non-cached or cached.
 */

#ifndef MPU_HPP
#define MPU_HPP

#ifndef HALAL_MPUBUFFERS_MAX_INSTANCES // Define this in your build system if you need a different
                                       // value
#define HALAL_MPUBUFFERS_MAX_INSTANCES 100
#endif

#include "C++Utilities/CppUtils.hpp"
#include "C++Utilities/CppImports.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "stm32h7xx_hal.h"
#include "HALAL/Models/MPUManager/MPUManager.hpp"

#ifdef SIM_ON

#define D1_NC_DATA
#define D1_NC_BSS
#define D1_NC_RODATA
#define D1_NC_DATA_INLINE(name) inline
#define D1_NC_BSS_INLINE(name) inline
#define D1_NC_RODATA_INLINE(name) inline
#define D2_NC_DATA
#define D2_NC_BSS
#define D2_NC_RODATA
#define D2_NC_DATA_INLINE(name) inline
#define D2_NC_BSS_INLINE(name) inline
#define D2_NC_RODATA_INLINE(name) inline
#define D3_NC_DATA
#define D3_NC_BSS
#define D3_NC_RODATA
#define D3_NC_DATA_INLINE(name) inline
#define D3_NC_BSS_INLINE(name) inline
#define D3_NC_RODATA_INLINE(name) inline
#define D1_C_DATA
#define D1_C_BSS
#define D1_C_RODATA
#define D1_C_DATA_INLINE(name) inline
#define D1_C_BSS_INLINE(name) inline
#define D1_C_RODATA_INLINE(name) inline
#define D2_C_DATA
#define D2_C_BSS
#define D2_C_RODATA
#define D2_C_DATA_INLINE(name) inline
#define D2_C_BSS_INLINE(name) inline
#define D2_C_RODATA_INLINE(name) inline
#define D3_C_DATA
#define D3_C_BSS
#define D3_C_RODATA
#define D3_C_DATA_INLINE(name) inline
#define D3_C_BSS_INLINE(name) inline
#define D3_C_RODATA_INLINE(name) inline
#define RAM_CODE
#define RAM_CODE_INLINE(name) inline
#define DTCM_RODATA
#define DTCM_RODATA_INLINE(name) inline

#else

#define MPU_STR2(x) #x
#define MPU_STR(x) MPU_STR2(x)
#define MPU_SECTION_WITH_NAME(base, name) base "." MPU_STR(name)

// Initialized data in non-cached D1 RAM
#define D1_NC_DATA __attribute__((section(".ram_d1_nc.user.data"))) volatile
// Uninitialized data in non-cached D1 RAM
#define D1_NC_BSS __attribute__((section(".ram_d1_nc.user.bss"))) volatile
// Read-only data in non-cached D1 RAM (mainly for rx buffers) @note Not protected by MPU
#define D1_NC_RODATA __attribute__((section(".ram_d1_nc.user.rodata"))) const volatile
#define D1_NC_DATA_INLINE(name)                                                                    \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d1_nc.user.data", name)))) volatile
#define D1_NC_BSS_INLINE(name)                                                                     \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d1_nc.user.bss", name)))) volatile
#define D1_NC_RODATA_INLINE(name)                                                                  \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d1_nc.user.rodata", name))))         \
    const volatile
// Initialized data in non-cached D2 RAM
#define D2_NC_DATA __attribute__((section(".ram_d2_nc.user.data"))) volatile
// Uninitialized data in non-cached D2 RAM
#define D2_NC_BSS __attribute__((section(".ram_d2_nc.user.bss"))) volatile
// Read-only data in non-cached D2 RAM (mainly for rx buffers) @note Not protected by MPU
#define D2_NC_RODATA __attribute__((section(".ram_d2_nc.user.rodata"))) const volatile
#define D2_NC_DATA_INLINE(name)                                                                    \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d2_nc.user.data", name)))) volatile
#define D2_NC_BSS_INLINE(name)                                                                     \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d2_nc.user.bss", name)))) volatile
#define D2_NC_RODATA_INLINE(name)                                                                  \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d2_nc.user.rodata", name))))         \
    const volatile
// Initialized data in non-cached D3 RAM
#define D3_NC_DATA __attribute__((section(".ram_d3_nc.user.data"))) volatile
// Uninitialized data in non-cached D3 RAM
#define D3_NC_BSS __attribute__((section(".ram_d3_nc.user.bss"))) volatile
// Read-only data in non-cached D3 RAM (mainly for rx buffers) @note Not protected by MPU
#define D3_NC_RODATA __attribute__((section(".ram_d3_nc.user.rodata"))) const volatile
#define D3_NC_DATA_INLINE(name)                                                                    \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d3_nc.user.data", name)))) volatile
#define D3_NC_BSS_INLINE(name)                                                                     \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d3_nc.user.bss", name)))) volatile
#define D3_NC_RODATA_INLINE(name)                                                                  \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d3_nc.user.rodata", name))))         \
    const volatile
// Initialized data in cached D1 RAM
#define D1_C_DATA __attribute__((section(".ram_d1.user.data")))
// Uninitialized data in cached D1 RAM
#define D1_C_BSS __attribute__((section(".ram_d1.user.bss")))
// Read-only data in cached D1 RAM (mainly for rx buffers) @note Not protected by MPU
#define D1_C_RODATA __attribute__((section(".ram_d1.user.rodata"))) const
#define D1_C_DATA_INLINE(name)                                                                     \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d1.user.data", name))))
#define D1_C_BSS_INLINE(name)                                                                      \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d1.user.bss", name))))
#define D1_C_RODATA_INLINE(name)                                                                   \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d1.user.rodata", name)))) const
// Initialized data in cached D2 RAM
#define D2_C_DATA __attribute__((section(".ram_d2.user.data")))
// Uninitialized data in cached D2 RAM
#define D2_C_BSS __attribute__((section(".ram_d2.user.bss")))
// Read-only data in cached D2 RAM (mainly for rx buffers) @note Not protected by MPU
#define D2_C_RODATA __attribute__((section(".ram_d2.user.rodata"))) const
#define D2_C_DATA_INLINE(name)                                                                     \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d2.user.data", name))))
#define D2_C_BSS_INLINE(name)                                                                      \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d2.user.bss", name))))
#define D2_C_RODATA_INLINE(name)                                                                   \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d2.user.rodata", name)))) const
// Initialized data in cached D3 RAM
#define D3_C_DATA __attribute__((section(".ram_d3.user.data")))
// Uninitialized data in cached D3 RAM
#define D3_C_BSS __attribute__((section(".ram_d3.user.bss")))
// Read-only data in cached D3 RAM (mainly for rx buffers) @note Not protected by MPU
#define D3_C_RODATA __attribute__((section(".ram_d3.user.rodata"))) const
#define D3_C_DATA_INLINE(name)                                                                     \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d3.user.data", name))))
#define D3_C_BSS_INLINE(name)                                                                      \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d3.user.bss", name))))
#define D3_C_RODATA_INLINE(name)                                                                   \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_d3.user.rodata", name)))) const

#define RAM_CODE __attribute__((section(".ram_code")))
#define RAM_CODE_INLINE(name)                                                                      \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".ram_code", name))))

// Constants in DTCM (default is FLASH because DTCM is small) @note Not protected by MPU
#define DTCM_RODATA __attribute__((section(".dtcm.rodata"))) const
#define DTCM_RODATA_INLINE(name)                                                                   \
    inline __attribute__((section(MPU_SECTION_WITH_NAME(".dtcm.rodata", name)))) const

#endif

// Retrocompatibility macros
#define D1_NC D1_NC_BSS
#define D2_NC D2_NC_BSS
#define D3_NC D3_NC_BSS
#define D1_C D1_C_BSS
#define D2_C D2_C_BSS
#define D3_C D3_C_BSS

// Memory Bank Symbols from Linker
extern "C" const char _itcm_base;
extern "C" const char _itcm_size;
extern "C" const char _dtcm_base;
extern "C" const char _dtcm_size;
extern "C" const char _flash_base;
extern "C" const char _flash_size;
extern "C" const char _ram_d1_base;
extern "C" const char _ram_d1_size;
extern "C" const char _ram_d2_base;
extern "C" const char _ram_d2_size;
extern "C" const char _ram_d3_base;
extern "C" const char _ram_d3_size;
extern "C" const char _peripheral_base;
extern "C" const char _peripheral_size;

// MPU Non-Cached Section Symbols from Linker
extern "C" const char _ram_d1_nc_start;
extern "C" const char _ram_d1_nc_end;
extern "C" const char _ram_d2_nc_start;
extern "C" const char _ram_d2_nc_end;
extern "C" const char _ram_d3_nc_start;
extern "C" const char _ram_d3_nc_end;

inline constexpr std::array<std::size_t, 6> mpu_supported_alignments = {32, 16, 8, 4, 2, 1};

consteval bool is_supported_mpu_alignment(std::size_t alignment) {
    for (std::size_t candidate : mpu_supported_alignments) {
        if (candidate == alignment) {
            return true;
        }
    }
    return false;
}

template <typename T>
concept mpu_buffer_request = requires(typename T::domain d) {
    typename T::buffer_type;
    { T{} } -> std::same_as<T>;
};

// POD-like buffers that can safely live in static storage without custom destruction.
template <typename T>
concept mpu_buffer_payload = std::is_standard_layout_v<T> && std::is_trivially_destructible_v<T>;

extern void compile_error(const char* msg);
struct MPUDomain {

    enum class MemoryDomain : uint8_t {
        D1 = 1, // AXI SRAM (0x24000000)
        D2 = 2, // SRAM 1/2/3 (0x30000000)
        D3 = 3  // SRAM 4 (0x38000000)
    };

    enum class MemoryType : bool { Cached = true, NonCached = false };

    // Buffer Request
    struct Entry {
        MemoryDomain memory_domain;
        MemoryType memory_type;
        std::size_t alignment;
        std::size_t size_in_bytes;
    };

    // Buffer Request Wrapper
    template <mpu_buffer_payload T> struct Buffer {
        using domain = MPUDomain;
        using buffer_type = T;
        Entry e;

        /**
         * @brief Constructs a Buffer entry for a type T with explicit parameter.
         * @tparam T The type for which the buffer is requested. Must be a POD type.
         * @param type The memory type (Cached or NonCached).
         * @param domain The memory domain where the buffer should be allocated (Defaults to D1,
         * since it is the largest and fastest).
         * @param force_cache_alignment If true, forces the buffer to be cache line aligned (32
         * bytes, takes the rest as padding).
         */
        consteval Buffer(
            MemoryType type = MemoryType::NonCached,
            MemoryDomain mem_domain = MemoryDomain::D1,
            bool force_cache_alignment = false
        )
            : e{mem_domain, type, force_cache_alignment ? 32 : alignof(T), sizeof(T)} {
            static_assert(
                alignof(T) <= 32,
                "Requested type has alignment greater than cache line size (32 bytes)."
            );
            static_assert(
                is_supported_mpu_alignment(alignof(T)),
                "Requested type has alignment not supported by MPU buffer system."
            );
        }

        /**
         * @brief Constructs a Buffer from an Entry struct (allows named parameter syntax).
         * @param entry The Entry with all buffer requirements specified.
         */
        consteval Buffer(Entry entry) : e(entry) {
            if (entry.alignment > 32) {
                compile_error("Requested alignment greater than cache line size (32 bytes).");
            }
            if (!is_supported_mpu_alignment(entry.alignment)) {
                compile_error("Requested alignment not supported by MPU buffer system.");
            }
            // Verify size matches sizeof(T)
            if (entry.size_in_bytes != sizeof(T)) {
                compile_error("Entry size_in_bytes must match sizeof(T)");
            }
        }

        template <class Ctx> consteval std::size_t inscribe(Ctx& ctx) const {
            return ctx.template add<MPUDomain>(e, this);
        }
    };

    static constexpr std::size_t max_instances = HALAL_MPUBUFFERS_MAX_INSTANCES;

    struct Config {
        uint32_t offset; // Offset relative to the start of the domain buffer
        std::size_t size;
        MemoryDomain domain;
        MemoryType type;
    };

    // Helper to calculate sizes needed for static arrays
    struct DomainSizes {
        size_t d1_nc_total = 0;
        size_t d1_c_total = 0;
        size_t d2_nc_total = 0;
        size_t d2_c_total = 0;
        size_t d3_nc_total = 0;
        size_t d3_c_total = 0;
    };

    static consteval uint32_t align_up(uint32_t val, size_t align) {
        return (val + align - 1) & ~(align - 1); // Align up to 'align' leaving the minimum padding
    }

    static consteval DomainSizes calculate_total_sizes(std::span<const Config> configs) {
        DomainSizes sizes;
        for (const auto& cfg : configs) {
            size_t end = cfg.offset + cfg.size;
            if (cfg.domain == MemoryDomain::D1) {
                if (cfg.type == MemoryType::NonCached)
                    sizes.d1_nc_total = std::max(sizes.d1_nc_total, end);
                else
                    sizes.d1_c_total = std::max(sizes.d1_c_total, end);
            } else if (cfg.domain == MemoryDomain::D2) {
                if (cfg.type == MemoryType::NonCached)
                    sizes.d2_nc_total = std::max(sizes.d2_nc_total, end);
                else
                    sizes.d2_c_total = std::max(sizes.d2_c_total, end);
            } else if (cfg.domain == MemoryDomain::D3) {
                if (cfg.type == MemoryType::NonCached)
                    sizes.d3_nc_total = std::max(sizes.d3_nc_total, end);
                else
                    sizes.d3_c_total = std::max(sizes.d3_c_total, end);
            }
        }
        // Align totals to 32 bytes, just in case
        sizes.d1_nc_total = align_up(sizes.d1_nc_total, 32);
        sizes.d1_c_total = align_up(sizes.d1_c_total, 32);
        sizes.d2_nc_total = align_up(sizes.d2_nc_total, 32);
        sizes.d2_c_total = align_up(sizes.d2_c_total, 32);
        sizes.d3_nc_total = align_up(sizes.d3_nc_total, 32);
        sizes.d3_c_total = align_up(sizes.d3_c_total, 32);
        return sizes;
    }

    template <std::size_t N>
    static consteval std::array<Config, N> build(std::span<const Entry> entries) {
        if constexpr (N == 0) {
            return {};
        } else {
            std::array<Config, N> cfgs{};

            uint32_t offsets_nc[3] = {}; // D1, D2, D3
            uint32_t offsets_c[3] = {};  // D1, D2, D3
            uint32_t assigned_offsets[N];

            for (size_t align : mpu_supported_alignments) {
                for (size_t i = 0; i < N; i++) {
                    if (entries[i].alignment == align) {
                        size_t d_idx = static_cast<size_t>(entries[i].memory_domain) - 1;
                        if (entries[i].memory_type == MemoryType::NonCached) {
                            offsets_nc[d_idx] = align_up(offsets_nc[d_idx], align);
                            assigned_offsets[i] = offsets_nc[d_idx];
                            offsets_nc[d_idx] += entries[i].size_in_bytes;
                        } else {
                            offsets_c[d_idx] = align_up(offsets_c[d_idx], align);
                            assigned_offsets[i] = offsets_c[d_idx];
                            offsets_c[d_idx] += entries[i].size_in_bytes;
                        }
                    }
                }
            }

            /* Build Configs */
            for (std::size_t i = 0; i < N; i++) {
                cfgs[i].size = entries[i].size_in_bytes;
                cfgs[i].domain = entries[i].memory_domain;
                cfgs[i].type = entries[i].memory_type;
                cfgs[i].offset = assigned_offsets[i];
            }

            return cfgs;
        }
    }

    struct Instance {
        void* ptr;
        std::size_t size;

        template <auto& Target, typename... Args> auto& construct(Args&&... args) {
            using Request = std::remove_cvref_t<decltype(Target)>;
            static_assert(mpu_buffer_request<Request>, "Target must be a valid MPUDomain buffer");
            constexpr bool is_nc = Target.e.memory_type == MemoryType::NonCached;
            static_assert(
                !is_nc || std::is_volatile_v<typename Request::buffer_type>,
                "Non cached buffers must be volatile to work as intended"
            );
            using T = typename Request::buffer_type;
            return *new (ptr) T(std::forward<Args>(args)...);
        }

        template <auto& Target> auto* as() {
            using Request = std::remove_cvref_t<decltype(Target)>;
            static_assert(mpu_buffer_request<Request>, "Target must be a valid MPUDomain buffer");
            constexpr bool is_nc = Target.e.memory_type == MemoryType::NonCached;
            static_assert(
                !is_nc || std::is_volatile_v<typename Request::buffer_type>,
                "Non cached buffers must be volatile to work as intended"
            );
            using T = typename Request::buffer_type;
            return static_cast<T*>(ptr);
        }
    };

    template <typename Board, auto& Target, typename... Args>
    static auto& construct(Args&&... args) {
        using Request = std::remove_cvref_t<decltype(Target)>;
        static_assert(mpu_buffer_request<Request>, "Target must be a valid MPUDomain buffer");
        return Board::template instance_of<Target>().template construct<Target>(
            std::forward<Args>(args)...
        );
    }

    template <typename Board, auto& Target> static auto* as() {
        using Request = std::remove_cvref_t<decltype(Target)>;
        static_assert(mpu_buffer_request<Request>, "Target must be a valid MPUDomain buffer");
        return Board::template instance_of<Target>().template as<Target>();
    }

    template <std::size_t N, std::array<Config, N> cfgs> struct Init {
        static inline std::array<Instance, N> instances{};

        static constexpr auto Sizes = calculate_total_sizes(cfgs);

#ifdef SIM_ON

        alignas(32) static inline volatile uint8_t
            d1_nc_buffer[Sizes.d1_nc_total > 0 ? Sizes.d1_nc_total : 1];
        alignas(32) static inline uint8_t d1_c_buffer[Sizes.d1_c_total > 0 ? Sizes.d1_c_total : 1];
        alignas(32) static inline volatile uint8_t
            d2_nc_buffer[Sizes.d2_nc_total > 0 ? Sizes.d2_nc_total : 1];
        alignas(32) static inline uint8_t d2_c_buffer[Sizes.d2_c_total > 0 ? Sizes.d2_c_total : 1];
        alignas(32) static inline volatile uint8_t
            d3_nc_buffer[Sizes.d3_nc_total > 0 ? Sizes.d3_nc_total : 1];
        alignas(32) static inline uint8_t d3_c_buffer[Sizes.d3_c_total > 0 ? Sizes.d3_c_total : 1];

#else

        __attribute__((section(".ram_d1_nc.buffer"))) alignas(32) static inline volatile uint8_t
            d1_nc_buffer[Sizes.d1_nc_total > 0 ? Sizes.d1_nc_total : 1];
        __attribute__((section(".ram_d1.buffer"))) alignas(32
        ) static inline uint8_t d1_c_buffer[Sizes.d1_c_total > 0 ? Sizes.d1_c_total : 1];

        __attribute__((section(".ram_d2_nc.buffer"))) alignas(32) static inline volatile uint8_t
            d2_nc_buffer[Sizes.d2_nc_total > 0 ? Sizes.d2_nc_total : 1];
        __attribute__((section(".ram_d2.buffer"))) alignas(32
        ) static inline uint8_t d2_c_buffer[Sizes.d2_c_total > 0 ? Sizes.d2_c_total : 1];

        __attribute__((section(".ram_d3_nc.buffer"))) alignas(32) static inline volatile uint8_t
            d3_nc_buffer[Sizes.d3_nc_total > 0 ? Sizes.d3_nc_total : 1];
        __attribute__((section(".ram_d3.buffer"))) alignas(32
        ) static inline uint8_t d3_c_buffer[Sizes.d3_c_total > 0 ? Sizes.d3_c_total : 1];

#endif

        static void init() {
            HAL_MPU_Disable();
            configure_static_regions();

            // Dynamic Configuration based on Linker Symbols
            configure_dynamic_region(
                reinterpret_cast<uintptr_t>(&_ram_d1_nc_start),
                reinterpret_cast<uintptr_t>(&_ram_d1_nc_end),
                MPU_REGION_NUMBER3
            );
            configure_dynamic_region(
                reinterpret_cast<uintptr_t>(&_ram_d2_nc_start),
                reinterpret_cast<uintptr_t>(&_ram_d2_nc_end),
                MPU_REGION_NUMBER5
            );
            configure_dynamic_region(
                reinterpret_cast<uintptr_t>(&_ram_d3_nc_start),
                reinterpret_cast<uintptr_t>(&_ram_d3_nc_end),
                MPU_REGION_NUMBER7
            );

            // Assign pointers
            volatile uint8_t* bases_nc[3] = {&d1_nc_buffer[0], &d2_nc_buffer[0], &d3_nc_buffer[0]};
            uint8_t* bases_c[3] = {&d1_c_buffer[0], &d2_c_buffer[0], &d3_c_buffer[0]};

            for (std::size_t i = 0; i < N; i++) {
                const auto& cfg = cfgs[i];
                auto& inst = instances[i];

                if (cfg.domain == MemoryDomain::D1 || cfg.domain == MemoryDomain::D2 ||
                    cfg.domain == MemoryDomain::D3) {
                    size_t d_idx = static_cast<size_t>(cfg.domain) - 1;

                    if (cfg.type == MemoryType::NonCached) {
                        inst.ptr = const_cast<uint8_t*>(bases_nc[d_idx] + cfg.offset);
                    } else {
                        inst.ptr = bases_c[d_idx] + cfg.offset;
                    }
                    inst.size = cfg.size;
                }
            }

            HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
            SCB_EnableICache();
            SCB_EnableDCache();
        }
    };

private:
    static void configure_dynamic_region(uintptr_t start, uintptr_t end, uint8_t region_num) {
        if (end <= start)
            return;
        // Dx NC (Normal, Non-Cacheable)
        // TEX=1, C=0, B=0: Normal, Non-Cacheable
        // Shareable since it can be accessed by multiple masters (CPU, DMA, etc)
        configure_region(
            start,
            end - start,
            region_num,
            MPU_TEX_LEVEL1,
            MPU_REGION_FULL_ACCESS,
            MPU_INSTRUCTION_ACCESS_DISABLE,
            MPU_ACCESS_SHAREABLE,
            MPU_ACCESS_NOT_CACHEABLE,
            MPU_ACCESS_NOT_BUFFERABLE
        );
    }

    static void configure_static_regions() {
        MPU_Region_InitTypeDef MPU_InitStruct = {0};

        // Background (No Access) - Covers all memory not explicitly defined by a further region
        // Doesn't use configure_region helper since that uses an uint32_t size, we need 4GB here...
        MPU_InitStruct.Enable = MPU_REGION_ENABLE;
        MPU_InitStruct.Number = MPU_REGION_NUMBER0;
        MPU_InitStruct.BaseAddress = 0x00;
        MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
        MPU_InitStruct.SubRegionDisable = 0x00;
        MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
        MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
        MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
        MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
        MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
        MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
        HAL_MPU_ConfigRegion(&MPU_InitStruct);

        // Peripherals (Device, Buffered)
        // Guarded against speculative execution and cache
        configure_region(
            reinterpret_cast<uintptr_t>(&_peripheral_base),
            reinterpret_cast<size_t>(&_peripheral_size),
            MPU_REGION_NUMBER8,
            MPU_TEX_LEVEL0,
            MPU_REGION_FULL_ACCESS,
            MPU_INSTRUCTION_ACCESS_DISABLE,
            MPU_ACCESS_SHAREABLE,
            MPU_ACCESS_NOT_CACHEABLE,
            MPU_ACCESS_BUFFERABLE
        );

        // Flash (Normal, Cacheable)
        // TEX=0, C=1, B=0: Normal, Write-Through, No Read-Allocate (Read optimized)
        // Not Shareable to allow full caching
        configure_region(
            reinterpret_cast<uintptr_t>(&_flash_base),
            reinterpret_cast<size_t>(&_flash_size),
            MPU_REGION_NUMBER1,
            MPU_TEX_LEVEL0,
            MPU_REGION_FULL_ACCESS,
            MPU_INSTRUCTION_ACCESS_ENABLE,
            MPU_ACCESS_NOT_SHAREABLE,
            MPU_ACCESS_CACHEABLE,
            MPU_ACCESS_NOT_BUFFERABLE
        );

        // DTCM (Normal, Cacheable)
        // TEX=1, C=1, B=1: Normal, Write-Back, Write and Read Allocate
        // TCMs are like Cache, so they are not really cacheable, and the MPU settings are ignored
        configure_region(
            reinterpret_cast<uintptr_t>(&_dtcm_base),
            reinterpret_cast<size_t>(&_dtcm_size),
            MPU_REGION_NUMBER10,
            MPU_TEX_LEVEL1,
            MPU_REGION_FULL_ACCESS,
            MPU_INSTRUCTION_ACCESS_DISABLE,
            MPU_ACCESS_NOT_SHAREABLE,
            MPU_ACCESS_CACHEABLE,
            MPU_ACCESS_BUFFERABLE
        );

        // ITCM (Normal, Cacheable)
        // TEX=0, C=1, B=0: Normal, Write-Through, No Read-Allocate (Read optimized)
        // TCMs are like Cache, so they are not really cacheable, and the MPU settings are ignored
        configure_region(
            reinterpret_cast<uintptr_t>(&_itcm_base),
            reinterpret_cast<size_t>(&_itcm_size),
            MPU_REGION_NUMBER11,
            MPU_TEX_LEVEL0,
            MPU_REGION_FULL_ACCESS,
            MPU_INSTRUCTION_ACCESS_ENABLE,
            MPU_ACCESS_NOT_SHAREABLE,
            MPU_ACCESS_CACHEABLE,
            MPU_ACCESS_NOT_BUFFERABLE
        );

        // Null-pointer guard: blocks first 32 bytes of ITCM to catch null dereferences
        configure_region(
            0x00000000,
            32,
            MPU_REGION_NUMBER12,
            MPU_TEX_LEVEL0,
            MPU_REGION_NO_ACCESS,
            MPU_INSTRUCTION_ACCESS_DISABLE,
            MPU_ACCESS_NOT_SHAREABLE,
            MPU_ACCESS_NOT_CACHEABLE,
            MPU_ACCESS_NOT_BUFFERABLE
        );

        // D1 RAM Cached (Normal, WBWA)
        // TEX=1, C=1, B=1: Normal, Write-Back, Write-Allocate
        // Shareable since it can be accessed by multiple masters (CPU, DMA, etc)
        configure_region(
            reinterpret_cast<uintptr_t>(&_ram_d1_base),
            reinterpret_cast<size_t>(&_ram_d1_size),
            MPU_REGION_NUMBER2,
            MPU_TEX_LEVEL1,
            MPU_REGION_FULL_ACCESS,
            MPU_INSTRUCTION_ACCESS_DISABLE,
            MPU_ACCESS_SHAREABLE,
            MPU_ACCESS_CACHEABLE,
            MPU_ACCESS_BUFFERABLE
        );

        // D2 RAM Cached (Normal, WBWA)
        // TEX=1, C=1, B=1: Normal, Write-Back, Write-Allocate
        // Shareable since it can be accessed by multiple masters (CPU, DMA, etc)
        configure_region(
            reinterpret_cast<uintptr_t>(&_ram_d2_base),
            reinterpret_cast<size_t>(&_ram_d2_size),
            MPU_REGION_NUMBER4,
            MPU_TEX_LEVEL1,
            MPU_REGION_FULL_ACCESS,
            MPU_INSTRUCTION_ACCESS_DISABLE,
            MPU_ACCESS_SHAREABLE,
            MPU_ACCESS_CACHEABLE,
            MPU_ACCESS_BUFFERABLE
        );

        // D3 RAM Cached (Normal, WBWA)
        // TEX=1, C=1, B=1: Normal, Write-Back, Write-Allocate
        // Shareable since it can be accessed by multiple masters (CPU, DMA, etc)
        configure_region(
            reinterpret_cast<uintptr_t>(&_ram_d3_base),
            reinterpret_cast<size_t>(&_ram_d3_size),
            MPU_REGION_NUMBER6,
            MPU_TEX_LEVEL1,
            MPU_REGION_FULL_ACCESS,
            MPU_INSTRUCTION_ACCESS_DISABLE,
            MPU_ACCESS_SHAREABLE,
            MPU_ACCESS_CACHEABLE,
            MPU_ACCESS_BUFFERABLE
        );
    }

    static void configure_region(
        uintptr_t base,
        size_t size,
        uint8_t region_num,
        uint8_t tex,
        uint8_t access,
        uint8_t no_exec,
        uint8_t shareable,
        uint8_t cacheable,
        uint8_t bufferable
    ) {
        if (size == 0)
            return;

        // Find smallest power of 2 >= size, starting at 32 bytes (MPU minimum)
        // Enforce MPU minimum (32 bytes = 2^(4+1))
        uint8_t mpu_size = 4;

        if (size > 32) {
            // Calculate ceil(log2(size)) - 1
            // __builtin_clz(x) returns leading zeros. For 32, it's 26: 31 - 26 = 5
            mpu_size = (sizeof(size) * 8 - 1) - __builtin_clz(size - 1);
        }

        // Calculate SubRegion Disable (SRD)
        uint32_t sub_size = (1 << (mpu_size + 1)) / 8;
        // Number of subregions needed to cover 'size'
        uint8_t needed_subs = (size + sub_size - 1) / sub_size;

        // We want first 'needed_subs' enabled (0), rest disabled (1)
        uint8_t srd = 0xFF << needed_subs;

        MPU_Region_InitTypeDef MPU_InitStruct = {0};
        MPU_InitStruct.Enable = MPU_REGION_ENABLE;
        MPU_InitStruct.Number = region_num;
        MPU_InitStruct.BaseAddress = static_cast<decltype(MPU_InitStruct.BaseAddress)>(base
        ); // Should be uint32_t, but may be different when mocking, so this is just to make sure it
           // accepts it
        MPU_InitStruct.Size = mpu_size;
        MPU_InitStruct.SubRegionDisable = srd;
        MPU_InitStruct.TypeExtField = tex;
        MPU_InitStruct.AccessPermission = access;
        MPU_InitStruct.DisableExec = no_exec;
        MPU_InitStruct.IsShareable = shareable;
        MPU_InitStruct.IsCacheable = cacheable;
        MPU_InitStruct.IsBufferable = bufferable;

        HAL_MPU_ConfigRegion(&MPU_InitStruct);
    }
};

#endif // MPU_HPP
