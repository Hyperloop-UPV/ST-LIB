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
    Stack() : top(0) {}
    
    bool push(T value) {
        if (top < S) {
            data[top++] = value;
            return true;
        }
        return false;
    }
    
    bool pop() {
        if (top == 0) {
            return false;
        }
        top--;
        return true;
    }

    // Returns the top element without removing it. Returns default T{} if stack is empty.
    T top() const {
        if (top == 0) {
            return T{};
        }
        return data[top - 1];
    }
    
    size_t size() const { return top; }
    bool empty() const { return top == 0; }
    
private:
    T data[S];
    size_t top;
};

#endif // STACK_HPP