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
    struct Buffer {
        using domain = MPUDomain;
        Entry e;

        /**
         * @brief Constructs a Buffer entry for a type T.
         * @tparam T The type for which the buffer is requested. Must be a POD type.
         * @param type The memory type (Cached or NonCached).
         * @param domain The memory domain where the buffer should be allocated.
         * @param force_cache_alignment If true, forces the buffer to be cache line aligned (32 bytes, takes the rest as padding).
         */
        template <typename T>
        consteval Buffer(MemoryType type = MemoryType::NonCached, MemoryDomain domain = MemoryDomain::D2, bool force_cache_alignment = false)
            requires(std::is_standard_layout_v<T> && std::is_trivial_v<T>)
        : e{
            domain,
            type,
            force_cache_alignment ? std::max(std::size_t(32), alignof(T)) : alignof(T),
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
        uint32_t base_address; 
        
        std::size_t size;
        
        // MPU Config Data
        bool is_mpu_leader;
        MPU_Region_InitTypeDef mpu_init;
    };

    // 5. Build compile-time: Entry[] → Config[]
    //
    // IMPORTANTE:
    // - Esta función es consteval: se ejecuta en tiempo de compilación.
    // - Aquí es donde se hacen TODAS las validaciones estáticas sobre los Entry.
    //
    template <std::size_t N>
    static consteval std::array<Config, N> build(std::span<const Entry> entries) {
        std::array<Config, N> cfgs{};

        for (std::size_t i = 0; i < N; i++) {
            const Entry &e = entries[i];

            // Aquí se pueden hacer checks globales y locales:
            //  - Comprobar que no se repite el mismo recurso.
            //  - Verificar que el recurso es válido para este dominio.
            //  - Validar que la combinación de parámetros tiene sentido.

            // Ejemplo de patrón (pseudo-código):
            // for (std::size_t j = 0; j < i; ++j) {
            //   if (entries[j].pin == e.pin && entries[j].port == e.port) {
            //     struct duplicate_resource {};
            //     throw duplicate_resource{};
            //   }
            // }

            // A partir de 'e' se construye cfgs[i] con los datos necesarios
            // para configurar el hardware en tiempo de ejecución.
            // cfgs[i] = ...;
        }

        return cfgs;
    }

    struct Instance {

        /**
         * @brief Constructs the object of type T in the allocated MPU memory.
         * @param args Arguments to forward to T's constructor.
         * @return Pointer to the constructed object of type T.
         */
        template <typename T, typename... Args>
        T* construct(Args&&... args) {
            is_valid_type<T>();
            return new (ptr) T(std::forward<Args>(args)...); // Placement new
        }

        /**
         * @brief Casts the stored pointer to the desired type T.
         * @tparam T The type to cast the pointer to.
         * @return Pointer of type T.
         */
        template <typename T = void>
        T* as() {
            is_valid_type<T>();
            return static_cast<T*>(ptr);
        }

       private:
        template<std::size_t N> friend struct Init;

        template <typename T>
        void is_valid_type() {
            static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>,
                          "MPU Buffer can only store POD types (standard layout and trivial).");
            if (sizeof(T) > size) {
                ErrorHandler("Requested type size exceeds allocated MPU buffer size.");
            }
            if (reinterpret_cast<uint32_t>(ptr) % alignof(T) != 0) {
                ErrorHandler("Requested type alignment is not satisfied by allocated MPU buffer.");
            }
        }
        
        void* ptr;
        std::size_t size;
        Instance() : ptr{nullptr}, size{0} {}
    };

    // 7. Inicialización runtime: aplica Config → crea Instance[]
    template <std::size_t N>
    struct Init {
        static inline std::array<Instance, N> instances{};

        // Esta función se llama desde Board::init() en TIEMPO DE EJECUCIÓN.
        // Aquí ya no se hacen checks de diseño: solo se aplica la configuración
        // generada en compile-time y se llama a HAL/LL.
        static void init(std::span<const Config, N> cfgs) {
            for (std::size_t i = 0; i < N; i++) {
                const auto &cfg = cfgs[i];
                auto &inst = instances[i];

                // Inicializar 'inst' a partir de 'cfg':
                //  - llamadas a HAL/LL
                //  - set de registros, punteros a periféricos, etc.
            }
        }
    };

   private:
    static consteval uint32_t get_address(MemoryDomain domain, std::size_t offset) {
        switch (domain) {
            case MemoryDomain::D1:
                return 0x24000000 + static_cast<uint32_t>(offset);
            case MemoryDomain::D2:
                return 0x30000000 + static_cast<uint32_t>(offset);
            case MemoryDomain::D3:
                return 0x38000000 + static_cast<uint32_t>(offset);
            default:
                throw "Invalid Memory Domain";
        }
    }

    static consteval uint32_t align_up(uint32_t address, std::size_t alignment) {
        uint32_t mask = static_cast<uint32_t>(alignment - 1);
        return (address + mask) & ~mask;
    }

    static consteval std::pair<std::size_t, std::uint8_t> get_size_needed(std::size_t size) {
        // Get next power of two
        if (popcount(size) == 1) {
            return {size, 0};
        } else {
            size_t power = 1 + countl_zero(size);

            if (power < 5) {
                power = 5; // Minimum MPU region size is 32 bytes
            }

            // MPU can divide to 8 subregions, so try to get a closer size
            size_t subregion_size = 1U << (power - 3);
            size_t num_subregions = (size + subregion_size - 1) / subregion_size; // Round up division
            uint8_t subregion_disable = static_cast<uint8_t>(~((0xFFU << num_subregions) - 1));
            static_assert(subregion_disable != 0xFF, "Something isn't working on the MPU region size calculation.");

            return {subregion_size * num_subregions, subregion_disable};
        }
    }
};

#endif // MPU_HPP