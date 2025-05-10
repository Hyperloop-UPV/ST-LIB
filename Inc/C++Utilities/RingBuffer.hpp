#pragma once

#include "CppImports.hpp"

template <typename T, size_t N>
class RingBuffer {
    std::array<T, N> buffer{};

    size_t stored_items{0};

    size_t front{0};
    size_t back{0};

    size_t move_forward(size_t origin, size_t amount) {
        return (origin + amount) % N;
    }

    size_t move_backward(size_t origin, size_t amount) {
        // N * ((amount / N) + 1) makes it so that the operation doesn't
        // overflow (size_t is unsigned) and with the help of modular
        // arithmetic, this won't change the result
        return ((origin + (N * ((amount / N) + 1))) - amount) % N;
    }

   public:
    RingBuffer() {}

    bool push(T item) {
        if (is_full()) return false;

        buffer[front] = item;
        front = move_forward(front, 1);
        ++stored_items;

        return true;
    }

    bool pop() {
        if (is_empty()) return false;

        back = move_forward(back, 1);
        --stored_items;

        return true;
    }

    bool push_pop(T item) {
        if (is_empty()) return false;

        buffer[front] = item;
        front = move_forward(front, 1);
        back = move_forward(back, 1);

        return true;
    }

    T &operator[](size_t index) {
        return buffer[move_backward(front, index + 1)];
    }

    T &last() { return buffer[back]; }

    T &first() { return buffer[move_backward(front, 1)]; }

    constexpr size_t capacity() { return N; }
    size_t size() { return stored_items; }
    bool is_full() { return size() >= capacity(); }
    bool is_empty() { return size() == 0; }
};
