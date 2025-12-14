#pragma once
#include <cstdint>
#include <iostream>


template<typename EnumType, EnumType Reg>
struct RegisterTraits {
    static void write(uint32_t& target, uint32_t val) {
        target = val; 
    }
};


template<typename EnumType, EnumType Reg>
class RegisterBase {
public:
    uint32_t reg = 0;

    RegisterBase() = default;
    RegisterBase(uint32_t val) : reg(val) {}

    RegisterBase& operator=(uint32_t val) {
        set(val);
        return *this;
    }
    RegisterBase& operator+=(uint32_t val){
        set(reg+val);
        return *this;
    }
    RegisterBase& operator-=(uint32_t val){
        set(reg-val);
        return *this;
    }

    RegisterBase& operator&=(uint32_t mask) {
        set(reg & mask); 
        return *this;
    }

    RegisterBase& operator|=(uint32_t mask) {
        set(reg | mask); 
        return *this;
    }

    RegisterBase& operator^=(uint32_t mask) {
        set(reg ^ mask); 
        return *this;
    }
    RegisterBase& operator<<=(int shift) {
        set(reg << shift); // Triggers Traits
        return *this;
    }

    RegisterBase& operator>>=(int shift) {
        set(reg >> shift); // Triggers Traits
        return *this;
    }

    // --- Shift Read (Does NOT modify Register) ---
    // Usage: uint32_t val = REG >> 4;
    uint32_t operator<<(int shift) const {
        return reg << shift;
    }

    uint32_t operator>>(int shift) const {
        return reg >> shift;
    }
    RegisterBase& operator++() {
        set(reg + 1);
        return *this;
    }

    // --- Postfix Increment (REG++) ---
    // int argument is a dummy flag for C++ to distinguish postfix
    uint32_t operator++(int) {
        uint32_t old_val = reg;
        set(reg + 1);
        return old_val; 
    }

    RegisterBase& operator--() {
        set(reg - 1);
        return *this;
    }

    uint32_t operator--(int) {
        uint32_t old_val = reg;
        set(reg - 1);
        return old_val;
    }
    /**
     * COMPARISON
     */
    bool operator!=(uint32_t val) const {
        return reg != val;
    }
    bool operator==(uint32_t val) const {
        return reg == val;
    }
    /**
     * BITWISE COMPARISONS
     */
    operator uint32_t(){
        return reg;
    }
    operator uint32_t() const volatile {
        return reg;
    }
    operator uint32_t() const {
        return reg;
    }
    
protected:
    void set(uint32_t val) {
        RegisterTraits<EnumType, Reg>::write(this->reg, val);
    }
};