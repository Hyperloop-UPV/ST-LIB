#pragma once
#include "ControlBlock.hpp"

template <class InputType, class OutputType>
class FeedbackControlBlock : public ControlBlock<InputType, OutputType> {
public:
    using Input = InputType;
    using Output = OutputType;
    InputType input_value_b;
    FeedbackControlBlock() = default;
    FeedbackControlBlock(InputType default_input, InputType default_input_b)
        : ControlBlock<InputType, OutputType>(default_input) {}
    FeedbackControlBlock(InputType default_input)
        : ControlBlock<InputType, OutputType>(default_input) {}

    void input(InputType input_a, InputType input_b) {
        this->input_value = input_a;
        this->input_value_b = input_b;
    }

    virtual void execute() = 0;

    void input_b(InputType input) { this->input_value_b = input; }
};
