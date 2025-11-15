/*
 * Promise.hpp
 *
 *  Created on: 15 nov. 2025
 *      Author: Boris
 */

#ifndef PROMISE_HPP
#define PROMISE_HPP

#include "C++Utilities/Arena.hpp"
#include "C++Utilities/RingBuffer.hpp"

/**
 * @brief A simple Promise implementation for asynchronous programming.
 * @note Promises are allocated from a fixed-size memory arena, so you don't own the memory. Use Promise::release() to release them back to the arena if needed.
 * @tparam S The maximum number of Promises that can be allocated simultaneously. Use the default "PromiseDefault" alias for a standard size.
 */
template<size_t S>
class Promise {
    using Callback = void(*)(void*);
    using ChainedCallback = Promise*(*)(void*);
    
   public:

    /**
     * @brief Create a new Promise.
     * @return Pointer to the newly created Promise, or nullptr if allocation failed.
     * @note The returned Promise must be released using Promise::release().
     * @note The Promise lives in a memory arena with a fixed maximum number of Promises (S), so you don't own the memory.
     */
    static Promise* inscribe() {
        Promise* p = Promise::arena.acquire();
        if (!p) {
            return nullptr;
        }
        p->isResolved = false;
        p->callback = nullptr;
        p->context = nullptr;
        return p;
    }

    /**
     * @brief Release a Promise back to the arena.
     * @param p Pointer to the Promise to release.
     * @return True if the Promise was successfully released, false otherwise.
     * @note After calling this function, the Promise pointer is no longer valid and must not be used.
     */
    static bool release(Promise* p) {
        return Promise::arena.release(p);
    }

    /**
     * @brief Register a callback to be called when the Promise is resolved.
     * @param cb The callback function.
     * @param ctx The context to be passed to the callback, can only be a pointer, you must manage the memory yourself. You could use an Arena for that.
     * @note If the Promise is already resolved, the callback is scheduled to be called in the next update cycle. You can call then whenever you want, but only one callback can be registered per Promise.
     */
    void then(Callback cb, void* ctx = nullptr) {
        callback = cb;
        context = ctx;
        if (isResolved) {
            readyList.push(this);
        }
    }
    /**
     * @brief Register a Promise-returning chained callback to be called when the Promise is resolved. You can chain multiple Promises together using this method.
     * @param cb The chained callback function that returns a new Promise.
     * @param ctx The context to be passed to the chained callback, can only be a pointer, you must manage the memory yourself.
     * @return Pointer to the newly created chained Promise.
     * @note If the Promise is already resolved, the chained callback is scheduled to be called in the next update cycle. You can call then whenever you want, but only one chained callback can be registered per Promise.
     * @example
     * ```cpp
     * Promise* p1 = Promise::inscribe();
     * p1->then([](void* ctx) {
     *     std::cout << "Promise 1 resolved!" << std::endl;
     *     auto p2 = Promise::inscribe(); // Return a new Promise
     *     // Simulate some async work
     * })->then([](void* ctx) {
     *     std::cout << "Chained Promise resolved!" << std::endl;
     * });
     * p1->resolve(); // This will trigger the first callback, and when the second Promise resolves, the chained callback will be called.
     * ```
     */
    Promise* then(ChainedCallback cb, void* ctx = nullptr) {
        next = Promise::inscribe();
        if (!next) {
            return nullptr;
        }
        chainedCallback = cb;
        chainedContext = ctx;
        context = this;
        callback = [](void* thisPtr) {
            Promise* p = static_cast<Promise*>(thisPtr);
            Promise* chained = p->chainedCallback(p->chainedContext);
            chained->then(p->next->callback, p->next->context);
            release(p->next);
        };
        if (isResolved) {
            readyList.push(this);
        }
        return next;
    }

    /**
     * @brief Resolve the Promise, triggering the registered callback. Works in interruptions.
     * @note Calling this after the Promise has been handled can be dangerous, as the Promise may have already been released back to the arena. Just remove the reference to the Promise after resolving it.
     * @note If the Promise is already resolved and the callback has not been called yet, calling this function has no effect.
     */
    void resolve() {
        if (isResolved) {
            return;
        }
        isResolved = true;
        if (callback) {
            readyList.push(this);
        }
    }

    /**
     * @brief Update the Promise system, processing all resolved Promises and calling their callbacks.
     * @note This function should be called regularly in the main loop of your application.
     */
    static void update() {
        while (!readyList.empty()) {
            Promise *p = readyList.front();
            readyList.pop();
            p->callback(p->context);
            Promise::arena.release(p);
        }
    }

    /**
     * @brief Create a new Promise that resolves when all the given Promises are resolved.
     * @param promises The Promises to wait for.
     * @return Pointer to the newly created Promise that resolves when all given Promises are resolved.
     * @note The promises will fail to create the all Promise and return nullptr if any of them already have a callback registered.
     */
    template<typename... Promises>
    static Promise* all(Promises*... promises) {
        auto chained = Promise::inscribe();
        chained->counter = sizeof...(promises);
        for (Promise* p : {promises...}) {
            if (p->callback != nullptr || p->chainedCallback != nullptr) {
                release(chained);
                for (Promise* released : {promises...}) {
                    if (released == p) {
                        break;
                    }
                    release(released);
                }
                return nullptr;
            }
            p->then([](void* ctx) {
                Promise* chained = static_cast<Promise*>(ctx);
                chained->counter--;
                if (chained->counter == 0) {
                    chained->resolve();
                }
            }, chained);
        }
        return chained;
    }

    /**
     * @brief Create a new Promise that resolves when any of the given Promises is resolved.
     * @param promises The Promises to wait for.
     * @return Pointer to the newly created Promise that resolves when any given Promise is resolved.
     * @note The promises will fail to create the any Promise and return nullptr if any of them already have a callback registered.
     */
    template<typename... Args>
    static Promise* any(Args*... promises) {
        auto anyPromise = Promise::inscribe();
        for (Promise* p : {promises...}) {
            if (p->callback != nullptr || p->chainedCallback != nullptr) {
                release(anyPromise);
                for (Promise* released : {promises...}) {
                    if (released == p) {
                        break;
                    }
                    release(released);
                }
                return nullptr;
            }
            p->then([](void* ctx) {
                Promise* anyPromise = static_cast<Promise*>(ctx);
                anyPromise->resolve();
            }, anyPromise);
        }
        return anyPromise;
    }

    Promise() = default;
    ~Promise() = default;
    Promise(Promise&&) = delete;
    Promise(const Promise&) = delete;
    Promise& operator=(Promise&&) = delete;
    Promise& operator=(const Promise&) = delete;

   private:
    Callback callback;
    ChainedCallback chainedCallback;
    void* context;
    void* chainedContext;
    bool isResolved = false;
    int counter = 0;
    Promise* next = nullptr;
    static Arena<S, Promise<S>> arena;
    static RingBuffer<Promise<S>*, S> readyList;
};
template<size_t S>
inline Arena<S, Promise<S>> Promise<S>::arena;
template<size_t S>
inline RingBuffer<Promise<S>*, S> Promise<S>::readyList;

using PromiseDefault = Promise<200>;

#endif // PROMISE_HPP