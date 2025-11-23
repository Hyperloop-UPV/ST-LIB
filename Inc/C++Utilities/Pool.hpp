/*
 * Pool.hpp
 *
 *  Created on: 15 nov. 2025
 *      Author: Boris
 */

#ifndef POOL_HPP
#define POOL_HPP

#include "CppImports.hpp"
#include "Stack.hpp"


/**
 * @brief A simple memory pool for fixed-size allocations.
 * @note It works like a heap but with a fixed maximum number of elements, and all the items are of the same type.
 * @tparam T The type of elements stored in the pool.
 * @tparam S The maximum number of elements in the pool.
 */
template<typename T, size_t S>
class Pool {
   public:

    /**
     * @brief Acquire an element from the pool.
     * @return Pointer to the acquired element, or nullptr if the pool is full (unlikely).
     */
    T* acquire() {
        if (freeIndexes.empty()) {
            return nullptr;
        }
        size_t index = freeIndexes.top();
        freeIndexes.pop();
        usedBitset.set(index);
        return &elements[index];
    }

    /**
     * @brief Acquire and construct an element in-place in the pool.
     * @param args The constructor arguments.
     * @return Pointer to the constructed element, or nullptr if the pool is full.
     */
    template<typename... Args>
    T* construct(Args&&... args) {
        T* elem = acquire();
        if (elem) {
            new (elem) T(std::forward<Args>(args)...); // Placement new
        }
        return elem;
    }

    /**
     * @brief Release an element back to the pool.
     * @param elem Pointer to the element to release.
     * @return True if the element was successfully released, false otherwise (unlikely with proper management).
     * @note Could potentially release an element different than the original one if misused (eg. double free).
     */
    bool release(T* elem) {
        if (elem < &elements[0] || elem - &elements[0] >= S) {
            return false;
        }
        size_t index = elem - &elements[0];
        if (!usedBitset.test(index)) {
            return false;
        }
        freeIndexes.push(index);
        usedBitset.reset(index);
        return true;
    }

    /**
     * @brief Destroy an element and release it back to the pool.
     * @param elem Pointer to the element to destroy.
     * @return True if the element was successfully destroyed and released, false otherwise.
     */
    bool destroy(T* elem) {
        if (elem < &elements[0] || elem - &elements[0] >= S) {
            return false;
        }
        size_t index = elem - &elements[0];
        if (!usedBitset.test(index)) {
            return false;
        }
        elem->~T();
        return release(elem);
    }

    size_t capacity() const { return S; }
    size_t available() const { return freeIndexes.size(); }
    size_t used() const { return S - freeIndexes.size(); }

    /**
     * @brief Iterator for traversing used elements in the pool.
     */
    class Iterator {
    public:
        Iterator(Pool* pool, size_t index) : pool(pool), index(index) {
            // Skip to the first used element
            while (index < S && !pool->usedBitset.test(index)) {
                ++index;
            }
        }

        T& operator*() { return pool->elements[index]; }
        T* operator->() { return &pool->elements[index]; }

        Iterator& operator++() {
            do {
                ++index;
            } while (index < S && !pool->usedBitset.test(index));
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }

    private:
        Pool* pool;
        size_t index;
    };

    /**
     * @brief Const iterator for traversing used elements in the pool.
     */
    class ConstIterator {
    public:
        ConstIterator(const Pool* pool, size_t index) : pool(pool), index(index) {
            // Skip to the first used element
            while (index < S && !pool->usedBitset.test(index)) {
                ++index;
            }
        }

        const T& operator*() const { return pool->elements[index]; }
        const T* operator->() const { return &pool->elements[index]; }

        ConstIterator& operator++() {
            do {
                ++index;
            } while (index < S && !pool->usedBitset.test(index));
            return *this;
        }

        bool operator!=(const ConstIterator& other) const {
            return index != other.index;
        }

    private:
        const Pool* pool;
        size_t index;
    };

    Iterator begin() { return Iterator(this, 0); }
    Iterator end() { return Iterator(this, S); }
    ConstIterator begin() const { return ConstIterator(this, 0); }
    ConstIterator end() const { return ConstIterator(this, S); }
    ConstIterator cbegin() const { return ConstIterator(this, 0); }
    ConstIterator cend() const { return ConstIterator(this, S); }

    Pool() {
        // Push indices in reverse order so index 0 is allocated first
        for (int i = S - 1; i >= 0; --i) {
            freeIndexes.push(i);
        }
    }
    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;
    Pool(Pool&& other) noexcept = delete;
    Pool& operator=(Pool&& other) = delete;

   private:
    T elements[S];
    Stack<size_t, S> freeIndexes;
    std::bitset<S> usedBitset;
};

#endif // POOL_HPP