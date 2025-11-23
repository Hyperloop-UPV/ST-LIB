/*
 * Promise.hpp
 *
 *  Created on: 15 nov. 2025
 *      Author: Boris
 */

#ifndef PROMISE_HPP
#define PROMISE_HPP

#include <atomic>
#include "C++Utilities/CppUtils.hpp"

// Maximum number of concurrent Promises allowed in the pool.
// Default is 200, which should be sufficient for most use cases. Increase if you expect higher concurrency.
// You can override this value by defining PROMISE_MAX_CONCURRENT before including this header.
#ifndef PROMISE_MAX_CONCURRENT
#define PROMISE_MAX_CONCURRENT 200
#endif

// Maximum number of Promise updates processed per cycle.
// Default is 50, which balances throughput and responsiveness. Tune this for your workload.
// You can override this value by defining PROMISE_MAX_UPDATES_PER_CYCLE before including this header.
#ifndef PROMISE_MAX_UPDATES_PER_CYCLE
#define PROMISE_MAX_UPDATES_PER_CYCLE 50
#endif

/**
 * @brief A simple Promise implementation for asynchronous programming.
 * @note Promises are allocated from a fixed-size memory pool, so you don't own the memory. Use Promise::release() to release them back to the pool if needed.
 */
class Promise {
    using Callback = void(*)(void*);
    using ChainedCallback = Promise*(*)(void*);
    
    enum class State : uint8_t {
        Pending,
        Resolved,
        Ready,
        Completed,
        ToBeReleased
    };
    
   public:

    /**
     * @brief Create a new Promise.
     * @return Pointer to the newly created Promise, or nullptr if allocation failed (unlikely).
     * @note The returned Promise must be released using Promise::release().
     * @note The Promise lives in a memory pool with a fixed maximum number of Promises (S), so you don't own the memory.
     */
    static Promise* inscribe() {
        Promise* p = Promise::pool.acquire();
        if (!p) {
            return nullptr;
        }
        p->state.store(State::Pending, std::memory_order_release);
        p->callback = nullptr;
        p->context = nullptr;
        p->chainedCallback = nullptr;
        p->chainedContext = nullptr;
        p->counter.store(1, std::memory_order_release);
        return p;
    }

    /**
     * @brief Release a Promise back to the pool. Shouldn't be called manually unless you are sure the Promise is no longer needed.
     * @param p Pointer to the Promise to release.
     * @return True if the Promise was successfully released, false otherwise (shouldn't happen with proper management).
     * @note After calling this function, the Promise pointer is no longer valid and must not be used. Using it after release results in undefined behavior.
     */
    static bool release(Promise* p) {
        return Promise::pool.release(p);
    }

    /**
     * @brief Register a callback to be called when the Promise is resolved.
     * @param cb The callback function.
     * @param ctx The context to be passed to the callback, can only be a pointer, you must manage the memory yourself. You could use a pool for that, or just pass a this pointer.
     * @note You can call then whenever you want, but only one callback can be registered per Promise.
     */
    void then(Callback cb, void* ctx = nullptr) {
        callback = cb;
        context = ctx;
        
        State expected = State::Resolved;
        state.compare_exchange_strong(expected, State::Ready, std::memory_order_acq_rel); // If already resolved, mark as ready
    }
    
    /**
     * @brief Register a Promise-returning chained callback to be called when the Promise is resolved. You can chain multiple Promises together using this method. Be extremely careful with memory management when using chained Promises.
     * @param cb The chained callback function that returns a new Promise.
     * @param ctx The context to be passed to the chained callback, can only be a pointer, you must manage the memory yourself.
     * @return Pointer to the newly created chained Promise.
     * @note If the Promise is already resolved, the chained callback is scheduled to be called in the next update cycle. You can call then whenever you want, but only one chained callback can be registered per Promise.
     * @note You should not store the returned Promise pointer for long-term use, as it is managed by the Promise system. Call then on the returned Promise to register further callbacks.
     * @example
     * ```cpp
     * Promise* p1 = Promise::inscribe();
     * p1->then([](void* ctx) {
     *     std::cout << "Promise 1 resolved!" << std::endl;
     *     auto p2 = Promise::inscribe(); // Return a new Promise
     *     return p2;
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
            if (chained) {
                chained->then(p->next->callback, p->next->context);
            }
            p->next->state.store(State::ToBeReleased, std::memory_order_release);
            p->next->counter.store(0, std::memory_order_release);
        };
        
        State expected = State::Resolved;
        state.compare_exchange_strong(expected, State::Ready, std::memory_order_acq_rel);
        
        return next;
    }

    /**
     * @brief Resolve the Promise, triggering the registered callback. Works in interrupts.
     * @note Calling this after the Promise has been handled can be dangerous, as the Promise may have already been released back to the pool. Just remove the reference to the Promise after resolving it.
     * @note If the Promise is already resolved and the callback has not been called yet, calling this function has no effect.
     */
    void resolve() {
        State expected = State::Pending;
        if (!state.compare_exchange_strong(expected, State::Resolved, std::memory_order_acq_rel)) {
            return;
        }
        
        if (callback) {
            state.store(State::Ready, std::memory_order_release);
        }
    }

    /**
     * @brief Update the Promise system, processing all resolved Promises and calling their callbacks.
     * @note This function should be called regularly in the main loop of your application.
     */
    static void update() {
        uint16_t count = 0;
        Promise* toRelease[PROMISE_MAX_UPDATES_PER_CYCLE];
        uint16_t releaseCount = 0;
        
        for (Promise& p : pool) {
            if (count >= PROMISE_MAX_UPDATES_PER_CYCLE) {
                break;
            }
            
            State expected = State::Ready;
            if (p.state.compare_exchange_strong(expected, State::Completed, std::memory_order_acq_rel)) {
                if (p.callback) {
                    p.callback(p.context);
                }
                p.counter.fetch_sub(1, std::memory_order_acq_rel);
                p.state.store(State::ToBeReleased, std::memory_order_release);
            }
            if (p.state.load(std::memory_order_acquire) == State::ToBeReleased && p.counter.load(std::memory_order_acquire) == 0) {
                toRelease[releaseCount++] = &p;
                count++;
            }
        }
        
        // Release all completed Promises after iteration
        for (uint16_t i = releaseCount; i > 0; i--) {
            Promise::pool.release(toRelease[i-1]);
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
        // Check if any promise already has a callback registered
        for (Promise* p : {promises...}) {
            if (p->callback != nullptr || p->chainedCallback != nullptr) {
                return nullptr;
            }
        }

        auto allPromise = Promise::inscribe();
        if (!allPromise) {
            return nullptr;
        }
        allPromise->counter.store(sizeof...(promises) + 1, std::memory_order_release);

        for (Promise* p : {promises...}) {
            p->then([](void* ctx) {
                Promise* allPromise = static_cast<Promise*>(ctx);
                int remaining = allPromise->counter.fetch_sub(1, std::memory_order_acq_rel) - 2; // -2 because fetch_sub returns the previous value, and we want to check if it is 1 after decrement (normal value for normal promises)
                if (remaining == 0) {
                    allPromise->resolve();
                }
            }, allPromise);
        }
        return allPromise;
    }

    /**
     * @brief Create a new Promise that resolves when any of the given Promises is resolved.
     * @param promises The Promises to wait for.
     * @return Pointer to the newly created Promise that resolves when any given Promise is resolved.
     * @note The promises will fail to create the any Promise and return nullptr if any of them already have a callback registered.
     */
    template<typename... Args>
    static Promise* any(Args*... promises) {
        // Check if any promise already has a callback registered
        for (Promise* p : {promises...}) {
            if (p->callback != nullptr || p->chainedCallback != nullptr) {
                return nullptr;
            }
        }

        auto anyPromise = Promise::inscribe();
        if (!anyPromise) {
            return nullptr;
        }
        anyPromise->counter.store(sizeof...(promises) + 1, std::memory_order_release);

        for (Promise* p : {promises...}) {
            p->then([](void* ctx) {
                Promise* anyPromise = static_cast<Promise*>(ctx);
                anyPromise->counter.fetch_sub(1, std::memory_order_acq_rel);
                State expected = State::Pending;
                if (anyPromise->state.compare_exchange_strong(expected, State::Resolved, std::memory_order_acq_rel)) {
                    if (anyPromise->callback) {
                        anyPromise->state.store(State::Ready, std::memory_order_release);
                    }
                }
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
    Callback callback = nullptr;
    ChainedCallback chainedCallback = nullptr;
    void* context = nullptr;
    void* chainedContext = nullptr;
    std::atomic<State> state{State::Pending};
    std::atomic<int> counter{0};
    Promise* next = nullptr;
    static Pool<Promise, PROMISE_MAX_CONCURRENT> pool;
};
inline Pool<Promise, PROMISE_MAX_CONCURRENT> Promise::pool;

#endif // PROMISE_HPP