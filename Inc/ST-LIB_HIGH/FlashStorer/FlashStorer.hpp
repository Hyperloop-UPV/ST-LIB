#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "FlashVariable.hpp"
#include "HALAL/Models/Concepts/Concepts.hpp"
#include "HALAL/Services/Flash/Flash.hpp"

#define FLASH_STORER_MAX_SIZE ((uint32_t)(1028 * 64))
#define FLASH_STORER_START_ADDRESS (FLASH_STORER_MAX_ADDRESS - FLASH_STORER_MAX_SIZE)
#define FLASH_STORER_MAX_ADDRESS ((uint32_t)0x080DFFF0U)

class FlashStorer {

public:
    static vector<FlashVariable> variable_list;
    static uint32_t total_size;

    template <class... Type> static bool add_variables(Type&... var);
    static void read(void);
    static bool store_all(void);
    template <class Type> static bool store(Type& var);
    template <class... Type>
        requires requires { requires sizeof...(Type) != 1; }
    static bool store(Type&... var);

private:
    template <class Type> static void add_size(Type& var); //?Mirar, si no hace falta borrar
};

template <class Type> bool FlashStorer::store(Type& var) {
    uint32_t number_of_words =
        ((FlashStorer::total_size + FLASH_32BITS_WORLD - 1) / FLASH_32BITS_WORLD
        ); // Fast ceiling the dvision result

    uint32_t* data = (uint32_t*)malloc(number_of_words * FLASH_32BITS_WORLD);
    Flash::read(FLASH_STORER_START_ADDRESS, (uint32_t*)data, number_of_words);

    uint32_t offset = 0;
    for (FlashVariable& v : FlashStorer::variable_list) {
        if (v.get_pointer() == &var) {
            memcpy((((uint8_t*)data) + offset), v.get_pointer(), v.get_size());
        }

        offset += v.get_size();
    }

    if (not Flash::write(data, FLASH_STORER_START_ADDRESS, number_of_words)) {
        return false;
    }

    free(data);
    return true;
}

template <class... Type>
    requires requires { requires sizeof...(Type) != 1; }
bool FlashStorer::store(Type&... var) {
    return (FlashStorer::store(var) & ...);
}

template <class... Type> bool FlashStorer::add_variables(Type&... var) {
    (FlashStorer::variable_list.push_back(FlashVariable(var)), ...);
    FlashStorer::total_size = total_sizeof<Type...>::value;

    return FlashStorer::total_size >= FLASH_STORER_MAX_SIZE;
}
