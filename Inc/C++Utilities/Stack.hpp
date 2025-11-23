/*
 * Stack.hpp
 *
 *  Created on: 15 nov. 2025
 *      Author: Boris
 */

#ifndef STACK_HPP
#define STACK_HPP

#include "CppImports.hpp"

/**
 * @brief A simple fixed-size stack.
 * @tparam T The type of elements stored in the stack.
 * @tparam S The maximum number of elements.
 */
template<typename T, size_t S>
class Stack {
public:
    Stack() : top_idx(0) {}
    
    bool push(const T& value) {
        if (top_idx < S) {
            data[top_idx++] = value;
            return true;
        }
        return false;
    }
    
    bool pop() {
        if (top_idx == 0) {
            return false;
        }
        top_idx--;
        return true;
    }

    // Returns the top element without removing it. Returns default T{} if stack is empty.
    T top() const {
        if (top_idx == 0) {
            return T{};
        }
        return data[top_idx - 1];
    }
    
    size_t size() const { return top_idx; }
    size_t capacity() const { return S; }
    bool empty() const { return top_idx == 0; }
    
private:
    T data[S];
    size_t top_idx;
};

#endif // STACK_HPP