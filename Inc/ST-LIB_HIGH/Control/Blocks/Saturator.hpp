#include "../ControlBlock.hpp"
#pragma once

template <typename T, typename U = T>
concept Comparable = requires(T a, U b) {
    { a < b };
    { b > a };
};

template <Comparable ComparableType>
class Saturator : public ControlBlock<ComparableType, ComparableType> {
public:
    ComparableType lower_bound = 0, upper_bound = 0;
    Saturator(ComparableType default_input, ComparableType lower_bound, ComparableType upper_bound)
        : ControlBlock<ComparableType, ComparableType>(default_input), lower_bound(lower_bound),
          upper_bound(upper_bound) {}
    Saturator(Saturator<ComparableType>&& new_ref)
        : ControlBlock<ComparableType, ComparableType>(new_ref.default_input) {
        upper_bound = new_ref.upper_bound;
        lower_bound = new_ref.lower_bound;
    }
    void execute() override {
        if (this->input_value > upper_bound) {
            this->output_value = upper_bound;
        } else if (this->input_value < lower_bound) {
            this->output_value = lower_bound;
        } else {
            this->output_value = this->input_value;
        }
    }
};

#if __cpp_deduction_guides >= 201606
template <Comparable ComparableType>
Saturator(ComparableType, ComparableType, ComparableType) -> Saturator<ComparableType>;
#endif
