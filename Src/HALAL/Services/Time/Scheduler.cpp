/*
 * Scheduler.hpp
 *
 * Created on: 17 nov. 2025
 *     Author: Victor (coauthor Stephan)
 */
#include "HALAL/Services/Time/Scheduler.hpp"

#ifndef TESTING_ENV
    // This is needed to register a TimerPeripheral
    #include "HALAL/Models/TimerPeripheral/TimerPeripheral.hpp"
#endif
#include <algorithm>
#include <limits>


/* NOTE(vic): Pido perdón a Boris pero es la mejor manera que se me ha ocurrido hacer esto */
#define SCHEDULER_RCC_TIMER_ENABLE \
    glue(glue(RCC_APB1LENR_TIM, SCHEDULER_TIMER_IDX), EN)
#define SCHEDULER_GLOBAL_TIMER_IRQn \
    glue(TIM, glue(SCHEDULER_TIMER_IDX, _IRQn))
#define SCHEDULER_GLOBAL_TIMER_CALLBACK() \
    extern "C" void glue(TIM, glue(SCHEDULER_TIMER_IDX, _IRQHandler))(void)

#define Scheduler_global_timer ((TIM_TypeDef*)SCHEDULER_TIMER_BASE)
namespace {
constexpr uint64_t kMaxIntervalUs =
    static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1ULL;
}

std::array<Scheduler::Task, Scheduler::kMaxTasks> Scheduler::tasks_{};
uint64_t Scheduler::sorted_task_ids_ = 0;
std::size_t Scheduler::active_task_count_{0};

uint32_t Scheduler::ready_bitmap_{0};
uint32_t Scheduler::free_bitmap_{0xFFFF'FFFF};
uint64_t Scheduler::global_tick_us_{0};
uint64_t Scheduler::current_interval_us_{0};

inline uint8_t Scheduler::get_at(uint8_t idx) {
    int word_idx = idx > 7;
    uint32_t shift = (idx & 7) << 2;
    return (((uint32_t*)sorted_task_ids_)[word_idx] & (0x0F << shift)) >> shift;
}
inline void Scheduler::set_at(uint8_t idx, uint8_t id) {
    uint32_t shift = idx*4;
    uint64_t clearmask = ~(0xFF << shift);
    Scheduler::sorted_task_ids_ = (sorted_task_ids_ & clearmask) | (id << shift);
    // sorted_task_ids_ |= ((id & 0x0F) << shift); // This is also an option in case id is incorrect, I don't think it's necessary though
}
inline uint8_t Scheduler::front_id() {
    return ((uint32_t*)sorted_task_ids_)[0] & 0xF;
}
inline void Scheduler::pop_front() {
    // O(1) remove of logical index 0
    Scheduler::active_task_count_--;
    Scheduler::sorted_task_ids_ >>= 4;
}

// ----------------------------

inline void Scheduler::global_timer_disable() {
    LL_TIM_DisableCounter(Scheduler_global_timer);
    //Scheduler_global_timer->CR1 &= ~TIM_CR1_CEN;
}
inline void Scheduler::global_timer_enable() {
    LL_TIM_EnableCounter(Scheduler_global_timer);
    //Scheduler_global_timer->CR1 |= TIM_CR1_CEN;
}

// ----------------------------
void Scheduler::start() {
    static_assert(kBaseClockHz % 1'000'000u == 0u, "Base clock must be a multiple of 1MHz");

    const uint32_t prescaler = (kBaseClockHz / 1'000'000u) - 1u;
    static_assert(prescaler < 0xFFFF'FFFF, "Prescaler is 16 bit, so it must be in that range");
#ifndef TESTING_ENV

    // Register a TimerPeripheral so it's not used anywhere else
    // hopefully we can move to something better than TimerPeripheral
    TimerPeripheral::InitData init_data(TimerPeripheral::BASE);
    TimerPeripheral perif_reserve(&SCHEDULER_HAL_TIM, std::move(init_data), (std::string)"timer2");

    RCC->APB1LENR |= SCHEDULER_RCC_TIMER_ENABLE;
#endif
    Scheduler_global_timer->PSC = (uint16_t)prescaler;
    Scheduler_global_timer->ARR = 0;
    Scheduler_global_timer->DIER |= LL_TIM_DIER_UIE;
    Scheduler_global_timer->CR1 = LL_TIM_CLOCKDIVISION_DIV1 | (Scheduler_global_timer->CR1 & ~TIM_CR1_CKD);
    // I think this should be cleared at startup. TODO: Look it up in ref manual
    // LL_TIM_DisableExternalClock(Scheduler_global_timer);
    //  |-> does this: Scheduler_global_timer->SMCR &= ~TIM_SMCR_ECE; /* Disable external clock */

    Scheduler_global_timer->CNT = 0; /* Clear counter value */

    NVIC_EnableIRQ(SCHEDULER_GLOBAL_TIMER_IRQn);
    LL_TIM_ClearFlag_UPDATE(Scheduler_global_timer);
    // NOTE(vic): We don't need to set the flag since there won't be any tasks at the start/it will get set in schedule_next_interval()
    Scheduler::global_timer_enable();
    //Scheduler::schedule_next_interval();
}

SCHEDULER_GLOBAL_TIMER_CALLBACK() { 
    /* clear update interrupt flag */
    LL_TIM_ClearFlag_UPDATE(Scheduler_global_timer);
    Scheduler::on_timer_update();
}
void Scheduler::update() {
    while(ready_bitmap_ != 0u) {
        uint32_t bit_index = static_cast<uint32_t>(__builtin_ctz(ready_bitmap_));
        ready_bitmap_ &= ~(1u << bit_index); // Clear the bit
        Task& task = tasks_[bit_index];
        task.callback();
        if (!task.repeating) [[unlikely]] {
            release_slot(static_cast<uint8_t>(bit_index));
        }
    }
}

inline uint64_t Scheduler::get_global_tick() { return global_tick_us_; }

// void Scheduler::global_timer_callback() { on_timer_update(); }

inline uint8_t Scheduler::allocate_slot() {
    /* https://developer.arm.com/documentation/dui0204/j/arm-and-thumb-instructions/general-data-processing-instructions/clz
     * clz(0) = 32          -> 32 - clz(0) = 0
     * clz(0xFFFF'FFFF) = 0 -> 32 - clz(0xFFFF'FFFF) > kMaxTasks
     */
    uint32_t idx = __builtin_ffs(~Scheduler::free_bitmap_) - 1;
    if(idx > static_cast<int>(Scheduler::kMaxTasks)) [[unlikely]]
        return static_cast<uint8_t>(Scheduler::INVALID_ID);
    Scheduler::active_task_count_++;
    Scheduler::free_bitmap_ &= ~(1UL << idx);
    return static_cast<uint8_t>(idx);
}

inline void Scheduler::release_slot(uint8_t id) {
    // NOTE: This condition shouldn't be here since it's an internal function but it could be an assert
    if(id >= kMaxTasks) [[unlikely]] return;
    ready_bitmap_ &= ~(1u << id);
    free_bitmap_ |= (1u << id);
    Scheduler::active_task_count_--;
}

void Scheduler::insert_sorted(uint8_t id) {
    Task& task = tasks_[id];

    // binary search on logical range [0, active_task_count_)
    std::size_t left = 0;
    std::size_t right = active_task_count_;
    while (left < right) {
        std::size_t mid = left + ((right - left) / 2);
        const Task& mid_task = tasks_[Scheduler::get_at(mid)];
        if (mid_task.next_fire_us <= task.next_fire_us) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    const std::size_t pos = left;

    uint32_t lo = (uint32_t)sorted_task_ids_;
    uint32_t hi = (uint32_t)(sorted_task_ids_ >> 32);
    
    uint32_t shift = (pos & 7) << 2;
    uint32_t id_shifted = id << shift;
    
    uint32_t mask = (1UL << shift) - 1;
    uint32_t inv_mask = ~mask; //Hole mask

    //Calculate both posibilities
    uint32_t lo_modified = ((lo & inv_mask) << 4) | (lo & mask) | id_shifted;
    uint32_t hi_modified = ((hi & inv_mask) << 4) | (hi & mask) | id_shifted;
    
    uint32_t hi_spilled  = (hi << 4) | (lo >> 28);
    
    if (pos >= 8) { //this can be done without branching 
        hi = hi_modified;
        // lo remains unchanged
    } else {
        hi = hi_spilled;
        lo = lo_modified;
    }

    sorted_task_ids_ = ((uint64_t)hi << 32) | lo;
}

void Scheduler::remove_sorted(uint8_t id) {
    uint64_t nibble_lsb = 0x1111'1111'1111'1111ULL;

    // pattern = nibble_lsb * id   (para obtener id en cada nibble)
    uint32_t pattern_32 = id + (id << 4);
    pattern_32 = pattern_32 + (pattern_32 << 8);
    pattern_32 = pattern_32 + (pattern_32 << 16);
    uint64_t pattern = pattern_32;
    ((uint32_t*)&pattern)[1] = pattern_32;

    // diff becomes 0xid..id_0_id..id where 0 is the nibble where id is in sorted_task_ids
    uint64_t diff = Scheduler::sorted_task_ids_ ^ pattern;
    
    //https://stackoverflow.com/questions/79058066/finding-position-of-zero-nibble-in-64-bits
    //https://stackoverflow.com/questions/59480527/fast-lookup-of-a-null-nibble-in-a-64-bit-unsigned
    uint64_t nibble_msb = 0x8888'8888'8888'8888ULL;
    uint64_t matches = (diff - nibble_lsb) & (~diff & nibble_msb);

    if(matches == 0) [[unlikely]] return; // not found

    /* split the bm in two 0x0000...FFFFFF where removal index is placed 
     * then invert to keep both sides that surround the discarded index
     */ 
    uint32_t pos_msb = __builtin_ctzll(matches);
    uint32_t pos_lsb = pos_msb - 3;

    uint64_t mask = (1ULL << pos_lsb) - 1;

    // Remove element (lower part | higher pushing nibble out of mask)
    Scheduler::sorted_task_ids_ = (Scheduler::sorted_task_ids_ & mask) | ((Scheduler::sorted_task_ids_ >> 4) & ~mask);
}

void Scheduler::schedule_next_interval() {
    if (active_task_count_ == 0) [[unlikely]] {
        Scheduler::global_timer_disable();
        current_interval_us_ = 0;
        return;
    }

    Scheduler::global_timer_enable();
    uint8_t next_id = Scheduler::front_id();  // sorted_task_ids_[0]
    Task& next_task = tasks_[next_id];
    uint64_t delta = (next_task.next_fire_us > (global_tick_us_ - 1ULL)) 
        ? (next_task.next_fire_us - global_tick_us_) : 1ULL;

    if (delta > kMaxIntervalUs) [[unlikely]] {
        current_interval_us_ = kMaxIntervalUs;
    } else {
        current_interval_us_ = delta;
    }

    configure_timer_for_interval(current_interval_us_);
}

inline void Scheduler::configure_timer_for_interval(uint64_t microseconds) {
    // NOTE(vic): disabling the timer _might_ be necessary to prevent the timer from firing in the middle of configuring it, highly unlikely since it has a period of at least 1 microsecond
    // TODO(vic): Validation: check arr is set correctly here: https://github.com/HyperloopUPV-H8/ST-LIB/pull/534#pullrequestreview-3529132356
    Scheduler_global_timer->ARR = static_cast<uint32_t>(microseconds - 1u);
    Scheduler_global_timer->CNT = 0;
    Scheduler::global_timer_enable();
}

void Scheduler::on_timer_update() {
    global_tick_us_ += current_interval_us_;

    uint8_t candidate_id = Scheduler::front_id();
    Task& task = tasks_[candidate_id];
    pop_front();
    ready_bitmap_ |= (1u << candidate_id); // mark task as ready

    if (task.repeating) [[likely]] {
        task.next_fire_us = global_tick_us_ + task.period_us;
        insert_sorted(candidate_id);
    }

    schedule_next_interval();
}

uint8_t Scheduler::register_task(uint32_t period_us, callback_t func, bool repeating) {
    if (func == nullptr) [[unlikely]] return static_cast<uint8_t>(Scheduler::INVALID_ID);

    uint8_t slot = allocate_slot();
    if(slot == Scheduler::INVALID_ID) return slot;

    Task& task = tasks_[slot];
    task.callback = func;
    task.period_us = period_us;
    task.repeating = repeating;
    task.next_fire_us = global_tick_us_ + period_us;

    insert_sorted(slot);
    schedule_next_interval();
    return slot;
}

bool Scheduler::unregister_task(uint8_t id) {
    if (id >= kMaxTasks) return false;
    if (free_bitmap_ & (1UL << id)) return false;

    remove_sorted(id);
    release_slot(id);
    schedule_next_interval();
    return true;
}
