/*
 * Scheduler.hpp
 *
 * Created on: 17 nov. 2025
 *     Author: Victor (coauthor Stephan)
 */
#include "HALAL/Services/Time/Scheduler.hpp"
#include "HALAL/Models/TimerDomain/TimerDomain.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

#include <stdint.h>

/* NOTE(vic): Pido perdón a Boris pero es la mejor manera que se me ha ocurrido hacer esto */
#define SCHEDULER_RCC_TIMER_ENABLE \
    glue(glue(RCC_APB1LENR_TIM, SCHEDULER_TIMER_IDX), EN)
#define SCHEDULER_GLOBAL_TIMER_IRQn \
    glue(TIM, glue(SCHEDULER_TIMER_IDX, _IRQn))

#define Scheduler_global_timer ((TIM_TypeDef*)SCHEDULER_TIMER_BASE)
namespace {
constexpr uint64_t kMaxIntervalUs =
    static_cast<uint64_t>(UINT32_MAX)/2 + 1ULL;
}

std::array<Scheduler::Task, Scheduler::kMaxTasks> Scheduler::tasks_{};
uint64_t Scheduler::sorted_task_ids_ = 0;
uint32_t Scheduler::active_task_count_{0};

uint32_t Scheduler::ready_bitmap_{0};
uint32_t Scheduler::free_bitmap_{0xFFFF'FFFF};
uint64_t Scheduler::global_tick_us_{0};
uint32_t Scheduler::current_interval_us_{0};
uint16_t Scheduler::timeout_idx_{1};

inline uint8_t Scheduler::get_at(uint8_t idx) {
    int word_idx = idx > 7;
    uint32_t shift = (idx & 7) << 2;
    return (((uint32_t*)&sorted_task_ids_)[word_idx] & (0x0F << shift)) >> shift;
}
inline void Scheduler::set_at(uint8_t idx, uint8_t id) {
    uint32_t shift = idx*4;
    uint64_t clearmask = ~(0xFF << shift);
    Scheduler::sorted_task_ids_ = (sorted_task_ids_ & clearmask) | (id << shift);
    // sorted_task_ids_ |= ((id & 0x0F) << shift); // This is also an option in case id is incorrect, I don't think it's necessary though
}
inline uint8_t Scheduler::front_id() {
    return *((uint8_t*)&sorted_task_ids_) & 0xF;
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
void scheduler_global_timer_callback(void *raw) {
    (void)raw;
    Scheduler::on_timer_update();
}
// ----------------------------

void Scheduler::start() {
    static_assert((Scheduler::FREQUENCY % 1'000'000) == 0u, "frequenct must be a multiple of 1MHz");


    uint32_t prescaler = (SystemCoreClock / Scheduler::FREQUENCY);
    // setup prescaler
    {
        // ref manual: section 8.7.7 RCC domain 1 clock configuration register
        uint32_t ahb_prescaler = RCC->D1CFGR & RCC_D1CFGR_HPRE_Msk;
        if((ahb_prescaler & 0b1000) != 0) {
            switch(ahb_prescaler) {
                case 0b1000: prescaler /= 2; break;
                case 0b1001: prescaler /= 4; break;
                case 0b1010: prescaler /= 8; break;
                case 0b1011: prescaler /= 16; break;
                case 0b1100: prescaler /= 64; break;
                case 0b1101: prescaler /= 128; break;
                case 0b1110: prescaler /= 256; break;
                case 0b1111: prescaler /= 512; break;
            }
        }

        // ref manual: section 8.7.8: RCC domain 2 clock configuration register
        uint32_t apb1_prescaler = (RCC->D2CFGR & RCC_D2CFGR_D2PPRE1_Msk) >> RCC_D2CFGR_D2PPRE1_Pos;
        if((apb1_prescaler & 0b100) != 0) {
            switch(apb1_prescaler) {
                case 0b100: prescaler /= 2; break;
                case 0b101: prescaler /= 4; break;
                case 0b110: prescaler /= 8; break;
                case 0b111: prescaler /= 16; break;
            }
        }
        // tim2clk = 2 x pclk1 when apb1_prescaler != 1
        if(apb1_prescaler != 1) {
            prescaler *= 2;
        }

        if(prescaler > 1) {
            prescaler--;
        }
    }

    // TODO: Fault when any of the next 2 static asserts happen (needs to be runtime bcos of SystemCoreClock)
    if(prescaler == 0 || prescaler > 0xFFFF) {
        ErrorHandler("Invalid prescaler value: %u", prescaler);
    }

    //static_assert(prescaler < 0xFFFF, "Prescaler is 16 bit, so it must be in that range");
    //static_assert(prescaler != 0, "Prescaler must be in the range [1, 65535]");
#ifndef TESTING_ENV
    RCC->APB1LENR |= SCHEDULER_RCC_TIMER_ENABLE;
#endif

    Scheduler_global_timer->PSC = (uint16_t)prescaler;
    Scheduler_global_timer->ARR = 0;
    Scheduler_global_timer->DIER |= LL_TIM_DIER_UIE;
    Scheduler_global_timer->CR1 = LL_TIM_CLOCKDIVISION_DIV1 | (Scheduler_global_timer->CR1 & ~TIM_CR1_CKD);
    // I think this should be cleared at startup. TODO: Look it up in ref manual
    // LL_TIM_DisableExternalClock(Scheduler_global_timer);
    //  |-> does this: Scheduler_global_timer->SMCR &= ~TIM_SMCR_ECE; /* Disable external clock */

    // Temporary solution for TimerDomain
    ST_LIB::TimerDomain::callbacks[ST_LIB::timer_idxmap[SCHEDULER_TIMER_IDX]] = scheduler_global_timer_callback;

    Scheduler_global_timer->CNT = 0; /* Clear counter value */

    NVIC_EnableIRQ(SCHEDULER_GLOBAL_TIMER_IRQn);
    CLEAR_BIT(Scheduler_global_timer->SR, LL_TIM_SR_UIF); /* clear update interrupt flag */

    Scheduler::schedule_next_interval();
}

void Scheduler::update() {
    static TIM_TypeDef *tim = Scheduler_global_timer;
    if(!tim) {
        ErrorHandler("Scheduler global timer not initialized");
    }

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

// void Scheduler::global_timer_callback() { on_timer_update(); }

inline uint8_t Scheduler::allocate_slot() {
    uint32_t idx = __builtin_ffs(Scheduler::free_bitmap_) - 1;
    if(idx > static_cast<int>(Scheduler::kMaxTasks)) [[unlikely]]
        return static_cast<uint8_t>(Scheduler::INVALID_ID);
    Scheduler::free_bitmap_ &= ~(1UL << idx);
    return static_cast<uint8_t>(idx);
}

inline void Scheduler::release_slot(uint8_t id) {
    ready_bitmap_ &= ~(1u << id);
    free_bitmap_ |= (1u << id);
}

void Scheduler::insert_sorted(uint8_t id) {
    Task& task = tasks_[id];

    // binary search on logical range [0, active_task_count_)
    std::size_t left = 0;
    std::size_t right = Scheduler::active_task_count_;
    while (left < right) {
        std::size_t mid = left + ((right - left) / 2);
        const Task& mid_task = tasks_[Scheduler::get_at(mid)];
        if ((int32_t)(task.next_fire_us - mid_task.next_fire_us) >= 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    const std::size_t pos = left;

    uint32_t lo = (uint32_t)sorted_task_ids_;
    uint32_t hi = (uint32_t)(sorted_task_ids_ >> 32);
    
    // take the shift for only high or low 32 bits
    uint32_t shift = (pos & 7) << 2;
    uint32_t id_shifted = id << shift;
    
    uint32_t mask = (1UL << shift) - 1;
    uint32_t inv_mask = ~mask; //Hole mask

    //Calculate both posibilities
    uint32_t lo_modified = ((lo & inv_mask) << 4) | (lo & mask) | id_shifted;
    uint32_t hi_modified = ((hi & inv_mask) << 4) | (hi & mask) | id_shifted;
    
    uint32_t hi_spilled  = (hi << 4) | (lo >> 28);
    
    if (pos >= 8) {
        hi = hi_modified;
        // lo remains unchanged
    } else {
        hi = hi_spilled;
        lo = lo_modified;
    }

    sorted_task_ids_ = ((uint64_t)hi << 32) | lo;
    Scheduler::active_task_count_++;
}

void Scheduler::remove_sorted(uint8_t id) {
    uint64_t nibble_lsb = 0x1111'1111'1111'1111ULL;

    // pattern = nibble_lsb * id   (para obtener id en cada nibble)
    uint32_t pattern_32 = id + (id << 4);
    pattern_32 = pattern_32 + (pattern_32 << 8);
    pattern_32 = pattern_32 + (pattern_32 << 16);
    uint64_t pattern = pattern_32;
    ((uint32_t*)&pattern)[1] = pattern_32;

    // diff becomes 0x..._0_... where 0 is the nibble where id is in sorted_task_ids
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
    Scheduler::active_task_count_--;
}

void Scheduler::schedule_next_interval() {
    if (active_task_count_ == 0) [[unlikely]] {
        Scheduler::global_timer_disable();
        current_interval_us_ = 0;
        return;
    }

    uint8_t next_id = Scheduler::front_id();  // sorted_task_ids_[0]
    Task& next_task = tasks_[next_id];
    int32_t diff = (int32_t)(next_task.next_fire_us - static_cast<uint32_t>(global_tick_us_));
    if (diff >= -1 && diff <= 1) [[unlikely]] {
        current_interval_us_ = 1;
        Scheduler_global_timer->CNT = 1;
        Scheduler_global_timer->ARR = 1;
    } else {
        if (diff < -1) [[unlikely]]{
            current_interval_us_ = static_cast<uint32_t>(0 - diff);
        } else {
            current_interval_us_ = static_cast<uint32_t>(diff);
        }
        Scheduler_global_timer->ARR = static_cast<uint32_t>(current_interval_us_ - 1u);
        while(Scheduler_global_timer->CNT > Scheduler_global_timer->ARR) [[unlikely]] {
            uint32_t offset = Scheduler_global_timer->CNT - Scheduler_global_timer->ARR;
            current_interval_us_ = offset;
            SET_BIT(Scheduler_global_timer->EGR, TIM_EGR_UG); // This should cause an interrupt
            Scheduler_global_timer->CNT += offset;
        }
    }
    Scheduler::global_timer_enable();
}

void Scheduler::on_timer_update() {
    global_tick_us_ += current_interval_us_;

    while (active_task_count_ > 0) { //Pop all due tasks, several might be due in the same tick
        uint8_t candidate_id = Scheduler::front_id();
        Task& task = tasks_[candidate_id];
        int32_t diff = (int32_t)(task.next_fire_us - static_cast<uint32_t>(global_tick_us_));
        if (diff > 0) [[likely]]{
            break; // Task is in the future, stop processing
        }
        pop_front();
        ready_bitmap_ |= (1u << candidate_id); // mark task as ready

        if (task.repeating) [[likely]] {
            task.next_fire_us = static_cast<uint32_t>(global_tick_us_ + task.period_us);
            insert_sorted(candidate_id);
        }
    }

    schedule_next_interval();
}

uint16_t Scheduler::register_task(uint32_t period_us, callback_t func) {
    if(func == nullptr) [[unlikely]] return static_cast<uint8_t>(Scheduler::INVALID_ID);
    if(period_us == 0) [[unlikely]] period_us = 1;
    if(period_us >= kMaxIntervalUs) [[unlikely]] return static_cast<uint8_t>(Scheduler::INVALID_ID);

    uint8_t slot = allocate_slot();
    if(slot == Scheduler::INVALID_ID) return slot;

    Task& task = tasks_[slot];
    task.callback = func;
    task.period_us = period_us;
    task.repeating = true;
    task.next_fire_us = static_cast<uint32_t>(global_tick_us_ + Scheduler_global_timer->CNT + period_us);
    task.id = static_cast<uint32_t>(slot);
    insert_sorted(slot);
    schedule_next_interval();
    return task.id;
}

uint16_t Scheduler::set_timeout(uint32_t microseconds, callback_t func) {
    if (func == nullptr) [[unlikely]] return static_cast<uint8_t>(Scheduler::INVALID_ID);
    if(microseconds == 0) [[unlikely]] microseconds = 1;
    if(microseconds >= kMaxIntervalUs) [[unlikely]] return static_cast<uint8_t>(Scheduler::INVALID_ID);

    uint8_t slot = allocate_slot();
    if(slot == Scheduler::INVALID_ID) return slot;

    Task& task = tasks_[slot];
    task.callback = func;
    task.period_us = microseconds;
    task.repeating = false;
    task.next_fire_us = static_cast<uint32_t>(global_tick_us_ + microseconds);
    task.id = slot + Scheduler::timeout_idx_ * Scheduler::kMaxTasks;

    // Add 2 instead of 1 so overflow doesn't make timeout_idx == 0, 
    // we need it to never be 0
    Scheduler::timeout_idx_ += 2;

    insert_sorted(slot);
    schedule_next_interval();
    return task.id;
}

bool Scheduler::unregister_task(uint16_t id) {
    if(id >= kMaxTasks) return false;
    if(free_bitmap_ & (1UL << id)) return false;

    remove_sorted(id);
    release_slot(id);
    schedule_next_interval();
    return true;
}

bool Scheduler::cancel_timeout(uint16_t id) {
    static_assert((kMaxTasks & (kMaxTasks - 1)) == 0, "kMaxTasks must be a power of two");
    uint32_t idx = id & (Scheduler::kMaxTasks - 1UL);
    if(tasks_[idx].repeating) return false;
    if(tasks_[idx].id != id) return false;
    if(free_bitmap_ & (1UL << idx)) return false;

    remove_sorted(idx);
    release_slot(idx);
    schedule_next_interval();
    return true;
}
