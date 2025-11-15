/*
 * Arena.hpp
 *
 *  Created on: 15 nov. 2025
 *      Author: Boris
 */

#ifndef ARENA_HPP
#define ARENA_HPP

#include "CppImports.hpp"
#include "RingBuffer.hpp"

/**
 * @brief A simple memory arena for fixed-size allocations.
 * @note It works like a heap but with a fixed maximum number of elements, and all the items are of the same type.
 * @tparam S The maximum number of elements in the arena.
 * @tparam T The type of elements stored in the arena.
 */
template<size_t S, typename T>
class Arena {
   public:

    /**
     * @brief Acquire an element from the arena.
     * @return Pointer to the acquired element, or nullptr if the arena is full.
     */
    T* acquire() {
        if (freeIndexes.empty()) {
            return nullptr;
        }
        size_t index = freeIndexes.front();
        freeIndexes.pop();
        usedIndexesSet[index] = true;
        return &elements[index];
    }

    /**
     * @brief Acquire and construct an element in-place in the arena.
     * @param args The constructor arguments.
     * @return Pointer to the constructed element, or nullptr if the arena is full.
     */
    template<typename... Args>
    T* construct(Args&&... args) {
        T* ele = acquire();
        if (ele) {
            new (ele) T(std::forward<Args>(args)...); // Placement new
        }
        return ele;
    }

    /**
     * @brief Release an element back to the arena.
     * @param ele Pointer to the element to release.
     * @return True if the element was successfully released, false otherwise.
     */
    bool release(T* ele) {
        if (ele < &elements[0] || ele >= &elements[S] || !usedIndexesSet[ele - &elements[0]]) {
            return false;
        }
        size_t index = ele - &elements[0];
        freeIndexes.push(index);
        usedIndexesSet[index] = false;
        return true;
    }

    /**
     * @brief Destroy an element and release it back to the arena.
     * @param ele Pointer to the element to destroy.
     * @return True if the element was successfully destroyed and released, false otherwise.
     */
    bool destroy(T* ele) {
        if (release(ele)) {
            ele->~T();
            return true;
        }
        return false;
    }

    size_t capacity() const { return S; }
    size_t available() const { return freeIndexes.size(); }
    size_t used() const { return S - freeIndexes.size(); }

    Arena() {
        std::iota(freeIndexes.begin(), freeIndexes.end(), 0); // Initialize free indexes, {0, 1, 2, ..., S-1}
    }
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena(Arena&& other) noexcept : freeIndexes(std::move(other.freeIndexes)) {
        std::copy(std::begin(other.usedIndexesSet), std::end(other.usedIndexesSet), std::begin(usedIndexesSet));
        std::move(std::begin(other.elements), std::end(other.elements), std::begin(elements));

        std::fill(std::begin(other.usedIndexesSet), std::end(other.usedIndexesSet), false);
        std::iota(other.freeIndexes.begin(), other.freeIndexes.end(), 0);
    }
    Arena& operator=(Arena&& other) noexcept {
        if (this != &other) {
            freeIndexes = std::move(other.freeIndexes);
            std::copy(std::begin(other.usedIndexesSet), std::end(other.usedIndexesSet), std::begin(usedIndexesSet));
            std::move(std::begin(other.elements), std::end(other.elements), std::begin(elements));

            std::fill(std::begin(other.usedIndexesSet), std::end(other.usedIndexesSet), false);
            std::iota(other.freeIndexes.begin(), other.freeIndexes.end(), 0);
        }
        return *this;
    }

   private:
    T elements[S];
    RingBuffer<size_t, S> freeIndexes;
    bool usedIndexesSet[S]{false};
};

#endif // ARENA_HPP