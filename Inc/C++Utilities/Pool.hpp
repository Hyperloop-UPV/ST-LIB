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
#include <bit>


/**
 * @brief A simple memory pool for fixed-size allocations.
 * @note It works like a heap but with a fixed maximum number of elements, and all the items are of the same type.
 * @note Will use a optimized version for pools with small sizes (S <= 32) using bitmap and CTZ for O(used) iteration.
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
        if (elem < &elements[0] || static_cast<size_t>(elem - &elements[0]) >= S) {
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
        if (elem < &elements[0] || static_cast<size_t>(elem - &elements[0]) >= S) {
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

// ============================================================================
// Optimized specialization for small pools (S <= 32) using bitmap
// ============================================================================

/**
 * @brief Optimized memory pool for small sizes (S <= 32) using bitmap and CTZ.
 * @note Uses bit manipulation for O(used) iteration instead of O(S).
 * @note Optimized for 32-bit systems.
 * @tparam T The type of elements stored in the pool.
 * @tparam S The maximum number of elements in the pool (must be <= 32).
 */
template<typename T, size_t S>
    requires (S <= 32)
class Pool<T, S> {
   public:

    /**
     * @brief Acquire an element from the pool.
     * @return Pointer to the acquired element, or nullptr if the pool is full.
     */
    T* acquire() {
        if (freeIndexes.empty()) {
            return nullptr;
        }
        size_t index = freeIndexes.top();
        freeIndexes.pop();
        usedBitmap |= (1U << index);
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
            new (elem) T(std::forward<Args>(args)...);
        }
        return elem;
    }

    /**
     * @brief Release an element back to the pool.
     * @param elem Pointer to the element to release.
     * @return True if the element was successfully released, false otherwise.
     */
    bool release(T* elem) {
        if (elem < &elements[0] || static_cast<size_t>(elem - &elements[0]) >= S) {
            return false;
        }
        size_t index = elem - &elements[0];
        if (!(usedBitmap & (1U << index))) {
            return false;
        }
        freeIndexes.push(index);
        usedBitmap &= ~(1U << index);
        return true;
    }

    /**
     * @brief Destroy an element and release it back to the pool.
     * @param elem Pointer to the element to destroy.
     * @return True if the element was successfully destroyed and released, false otherwise.
     */
    bool destroy(T* elem) {
        if (elem < &elements[0] || static_cast<size_t>(elem - &elements[0]) >= S) {
            return false;
        }
        size_t index = elem - &elements[0];
        if (!(usedBitmap & (1U << index))) {
            return false;
        }
        elem->~T();
        return release(elem);
    }

    size_t capacity() const { return S; }
    size_t available() const { return freeIndexes.size(); }
    size_t used() const { return S - freeIndexes.size(); }

    /**
     * @brief Fast iterator using bitmap CTZ for O(used) iteration.
     */
    class Iterator {
    public:
        Iterator(Pool* pool, uint32_t bitmap) : pool(pool), bitmap(bitmap) {}

        T& operator*() {
            size_t index = std::countr_zero(bitmap);
            return pool->elements[index];
        }

        T* operator->() {
            size_t index = std::countr_zero(bitmap);
            return &pool->elements[index];
        }

        Iterator& operator++() {
            bitmap &= (bitmap - 1);  // Clear lowest set bit
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return bitmap != other.bitmap;
        }

    private:
        Pool* pool;
        uint32_t bitmap;
    };

    /**
     * @brief Fast const iterator using bitmap CTZ for O(used) iteration.
     */
    class ConstIterator {
    public:
        ConstIterator(const Pool* pool, uint32_t bitmap) : pool(pool), bitmap(bitmap) {}

        const T& operator*() const {
            size_t index = std::countr_zero(bitmap);
            return pool->elements[index];
        }

        const T* operator->() const {
            size_t index = std::countr_zero(bitmap);
            return &pool->elements[index];
        }

        ConstIterator& operator++() {
            bitmap &= (bitmap - 1);  // Clear lowest set bit
            return *this;
        }

        bool operator!=(const ConstIterator& other) const {
            return bitmap != other.bitmap;
        }

    private:
        const Pool* pool;
        uint32_t bitmap;
    };

    Iterator begin() { return Iterator(this, usedBitmap); }
    Iterator end() { return Iterator(this, 0); }
    ConstIterator begin() const { return ConstIterator(this, usedBitmap); }
    ConstIterator end() const { return ConstIterator(this, 0); }
    ConstIterator cbegin() const { return ConstIterator(this, usedBitmap); }
    ConstIterator cend() const { return ConstIterator(this, 0); }

    Pool() : usedBitmap(0) {
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
    uint32_t usedBitmap;  // Bitmap for fast iteration (1 = used, 0 = free)
};

#endif // POOL_HPP