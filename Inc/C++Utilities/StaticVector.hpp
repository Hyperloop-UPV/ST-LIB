#pragma once
#include <array>
#include "ErrorHandler/ErrorHandler.hpp"

template <typename T, size_t Capacity>
class StaticVector {
 private: 
    std::array<T, Capacity> data{};
    size_t size_ = 0;

public:
    constexpr StaticVector() = default;
    
    template<typename... Args>
    constexpr StaticVector(Args&&... args) : data{std::forward<Args>(args)...}, size_(sizeof...(args)) {}

    constexpr bool operator==(const StaticVector&) const = default;

    constexpr void push_back(const T& value) 
    {
        if (size_ >= Capacity) 
        {
            ErrorHandler("StaticVector capacity exceeded");
            return;
        }        
        data[size_] = value;
        size_++;
    }

    constexpr auto begin() { return data.begin(); }
    constexpr auto begin() const { return data.begin(); }
    constexpr auto end() { return data.begin() + size_; } 
    constexpr auto end() const { return data.begin() + size_; } 
    
    constexpr const std::array<T, Capacity>& get_array() const { return data; }
    constexpr size_t size() const { return size_; }
    constexpr T* get_data() { return data.data(); }
    constexpr const T* get_data() const { return data.data(); }
    constexpr  T& operator[](size_t i) { return data[i]; }
    constexpr  const T& operator[](size_t i) const { return data[i]; }
    constexpr bool contains(const T& value) const 
    {
        for (size_t i = 0; i < size_; ++i) 
        {
            if (data[i] == value) 
            {
                return true;
            }
        }
        return false;
    }
};

