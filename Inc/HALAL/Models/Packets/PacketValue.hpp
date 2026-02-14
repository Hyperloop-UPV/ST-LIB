#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "HALAL/Models/Concepts/Concepts.hpp"

struct empty_type {};

template <class... Types> class PacketValue;

template <> class PacketValue<> {
public:
    using value_type = empty_type;
    PacketValue() = default;
    ~PacketValue() = default;
    virtual void* get_pointer() = 0;
    virtual void set_pointer(void* pointer) = 0;
    virtual size_t get_size() = 0;
    virtual void parse(uint8_t* data) = 0;
    virtual void copy_to(uint8_t* data) = 0;
};

template <class Type>
    requires NotContainer<Type>
class PacketValue<Type> : public PacketValue<> {
public:
    using value_type = Type;
    Type* src = nullptr;
    PacketValue() = default;
    PacketValue(Type* src) : src(src) {}
    ~PacketValue() = default;
    void* get_pointer() override { return src; }

    void set_pointer(void* pointer) { src = (Type*)pointer; }

    size_t get_size() override { return sizeof(Type); }
    void parse(uint8_t* data) override { *src = *((Type*)data); }
    void copy_to(uint8_t* data) override { *((Type*)data) = *src; }
};

template <> class PacketValue<double> : public PacketValue<> {
public:
    using value_type = double;
    double* src = nullptr;
    PacketValue() = default;
    PacketValue(double* src) : src(src) {}
    ~PacketValue() = default;
    void* get_pointer() override { return src; }

    void set_pointer(void* pointer) { src = (double*)pointer; }

    size_t get_size() override { return sizeof(double); }
    void parse(uint8_t* data) override { *src = *((double*)data); }
    void copy_to(uint8_t* data) override { memcpy(data, src, get_size()); }
};

#if __cpp_deduction_guides >= 201606
PacketValue(double*) -> PacketValue<double>;
#endif

template <class Type>
    requires Container<Type>
class PacketValue<Type> : public PacketValue<> {
public:
    using value_type = Type;
    Type* src = nullptr;
    PacketValue() = default;
    PacketValue(Type* src) : src(src) {}
    ~PacketValue() = default;
    void* get_pointer() override { return src->data(); }

    void set_pointer(void* pointer) { src = (Type*)pointer; }

    size_t get_size() override { return src->size() * sizeof(typename Type::value_type); }
    void parse(uint8_t* data) override { memcpy(src->data(), data, get_size()); }
    void copy_to(uint8_t* data) override { memcpy(data, src->data(), get_size()); }
};

#if __cpp_deduction_guides >= 201606
template <class Type> PacketValue(Type*) -> PacketValue<Type>;
#endif

template <> class PacketValue<string> : public PacketValue<> {
public:
    using value_type = string;
    string* src = nullptr;
    PacketValue() = default;
    PacketValue(string* src) : src(src) {}
    ~PacketValue() = default;
    void* get_pointer() override { return src->data(); }

    void set_pointer(void* pointer) { src = (string*)pointer; }

    size_t get_size() override { return strlen(src->c_str()) + 1; }
    void parse(uint8_t* data) override { memcpy(src->data(), data, get_size()); }
    void copy_to(uint8_t* data) override { memcpy(data, src->data(), get_size()); }
};

template <class Type, size_t N> class PacketValue<Type (&)[N]> : public PacketValue<> {
public:
    using value_type = Type;
    Type (*src)[N] = nullptr;
    PacketValue() = default;
    PacketValue(Type (*src)[N]) : src(src) {}
    ~PacketValue() = default;
    void* get_pointer() override { return src; }

    void set_pointer(void* pointer) { src = (Type(*)[N])pointer; }

    size_t get_size() override { return N * sizeof(Type); }
    void parse(uint8_t* data) override { memcpy(src, data, get_size()); }
    void copy_to(uint8_t* data) override { memcpy(data, src, get_size()); }
};

#if __cpp_deduction_guides >= 201606
template <class Type, size_t N> PacketValue(Type (*)[N]) -> PacketValue<Type (&)[N]>;
#endif

template <class Type, size_t N> class PacketValue<Type* (&)[N]> : public PacketValue<> {
public:
    using value_type = Type*;
    Type* (*src)[N] = nullptr;
    PacketValue() = default;
    PacketValue(Type* (*src)[N]) : src(src) {}
    ~PacketValue() = default;
    void* get_pointer() override { return src; }

    void set_pointer(void* pointer) { src = (Type * (*)[N]) pointer; }

    size_t get_size() override { return N * sizeof(Type); }
    void parse(uint8_t* data) override {
        for (Type* i : *src) {
            *i = *(Type*)data;
            data += sizeof(Type);
        }
    }
    void copy_to(uint8_t* data) override {
        for (Type* i : *src) {
            *(Type*)data = *i;
            data += sizeof(Type);
        }
    }
};

#if __cpp_deduction_guides >= 201606
template <class Type, size_t N> PacketValue(Type* (*)[N]) -> PacketValue<Type* (&)[N]>;
#endif
