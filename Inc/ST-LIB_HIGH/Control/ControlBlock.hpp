#pragma once

#include "C++Utilities/CppUtils.hpp"

template <class InputType, class OutputType> class ControlBlock {
public:
    using Input = InputType;
    using Output = OutputType;
    InputType input_value;
    OutputType output_value;
    ControlBlock() = default;
    ControlBlock(InputType default_input);
    virtual void input(InputType input);
    virtual void execute() = 0;
};

template <class InputType, class OutputType>
ControlBlock<InputType, OutputType>::ControlBlock(InputType default_input) {
    input_value = default_input;
}

template <class InputType, class OutputType>
void ControlBlock<InputType, OutputType>::input(InputType input_value) {
    this->input_value = input_value;
}
