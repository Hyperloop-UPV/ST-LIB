/*
 * Arena.hpp
 *
 *  Created on: 15 nov. 2025
 *      Author: Boris
 */

#ifndef ARENA_HPP
#define ARENA_HPP

#include "CppImports.hpp"
#include "Stack.hpp"


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
        size_t index = freeIndexes.pop();
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
        if (ele < &elements[0] || ele >= &elements[S]) {
            return false;
        }
        size_t index = ele - &elements[0];
        if (!usedIndexesSet[index]) {
            return false; // Double free detected
        }
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
        if (ele < &elements[0] || ele >= &elements[S]) {
            return false;
        }
        size_t index = ele - &elements[0];
        if (!usedIndexesSet[index]) {
            return false; // Double free detected
        }
        ele->~T();
        return release(ele);
    }

    size_t capacity() const { return S; }
    size_t available() const { return freeIndexes.size(); }
    size_t used() const { return S - freeIndexes.size(); }

    /**
     * @brief Iterator for traversing used elements in the arena.
     */
    class Iterator {
    public:
        Iterator(Arena* arena, size_t index) : arena(arena), index(index) {
            // Skip to the first used element
            while (index < S && !arena->usedIndexesSet[index]) {
                ++index;
            }
            this->index = index;
        }

        T& operator*() { return arena->elements[index]; }
        T* operator->() { return &arena->elements[index]; }

        Iterator& operator++() {
            do {
                ++index;
            } while (index < S && !arena->usedIndexesSet[index]);
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }

    private:
        Arena* arena;
        size_t index;
    };

    /**
     * @brief Const iterator for traversing used elements in the arena.
     */
    class ConstIterator {
    public:
        ConstIterator(const Arena* arena, size_t index) : arena(arena), index(index) {
            // Skip to the first used element
            while (index < S && !arena->usedIndexesSet[index]) {
                ++index;
            }
            this->index = index;
        }

        const T& operator*() const { return arena->elements[index]; }
        const T* operator->() const { return &arena->elements[index]; }

        ConstIterator& operator++() {
            do {
                ++index;
            } while (index < S && !arena->usedIndexesSet[index]);
            return *this;
        }

        bool operator!=(const ConstIterator& other) const {
            return index != other.index;
        }

    private:
        const Arena* arena;
        size_t index;
    };

    Iterator begin() { return Iterator(this, 0); }
    Iterator end() { return Iterator(this, S); }
    ConstIterator begin() const { return ConstIterator(this, 0); }
    ConstIterator end() const { return ConstIterator(this, S); }
    ConstIterator cbegin() const { return ConstIterator(this, 0); }
    ConstIterator cend() const { return ConstIterator(this, S); }

    Arena() {
        // Push indices in reverse order so index 0 is allocated first
        for (int i = S - 1; i >= 0; --i) {
            freeIndexes.push(i);
        }
    }
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena(Arena&& other) noexcept = delete;
    Arena& operator=(Arena&& other) = delete;

   private:
    T elements[S];
    Stack<S> freeIndexes;
    bool usedIndexesSet[S]{false};
};

#endif // ARENA_HPP