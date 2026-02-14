#pragma once

#include "FeedbackControlBlock.hpp"

template <class InputType> class SplitterBlock : public ControlBlock<InputType, InputType> {
public:
    using Input = InputType;
    using Output = InputType;
    function<void(InputType)> feedback_input;
    SplitterBlock() = default;
    SplitterBlock(InputType default_input) : ControlBlock<InputType, InputType>(default_input) {}
    void execute() override {
        this->output_value = this->input_value;
        feedback();
    }
    void feedback() {
        if (feedback_input != nullptr) {
            feedback_input(this->output_value);
        }
    }
    template <class AdditionalType>
    void connect(FeedbackControlBlock<InputType, AdditionalType>& next) {
        feedback_input = bind(
            &FeedbackControlBlock<InputType, AdditionalType>::input_b,
            &next,
            placeholders::_1
        );
    }
};
