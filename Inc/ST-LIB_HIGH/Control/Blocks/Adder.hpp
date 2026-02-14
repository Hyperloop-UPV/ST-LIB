#pragma once

#include "../FeedbackControlBlock.hpp"

template <typename T, typename U = T>
concept Sumable = requires(T a, U b) {
    { a + b };
    { b + a };
};

template <Sumable SumableType> class Adder : public FeedbackControlBlock<SumableType, SumableType> {
public:
    int sign = 1;
    Adder() = default;
    Adder(SumableType default_input, SumableType default_input_b)
        : FeedbackControlBlock<SumableType, SumableType>(default_input, default_input_b) {}
    void execute() override { this->output_value = this->input_value + sign * this->input_b; }
    void set_sign(char sign) {
        switch (sign) {
        case '+':
            sign = 1;
            break;
        case '-':
            sign = -1;
            break;
        default:
            break;
        }
    }
};
