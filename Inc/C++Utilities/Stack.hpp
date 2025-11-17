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
 * @brief A simple fixed-size stack for arena free list management.
 * @tparam S The maximum number of elements.
 */
template<size_t S>
class Stack {
public:
    Stack() : top(0) {}
    
    void push(size_t value) {
        if (top < S) {
            data[top++] = value;
        }
    }
    
    size_t pop() {
        return data[--top];
    }
    
    size_t size() const { return top; }
    bool empty() const { return top == 0; }
    
private:
    size_t data[S];
    size_t top;
};

#endif // STACK_HPP