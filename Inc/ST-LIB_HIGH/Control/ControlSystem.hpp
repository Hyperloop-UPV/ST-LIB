#pragma once
#include "ControlBlock.hpp"

template <class... Types> class ControlSystem;

template <class System>
concept ControlSystemConcept = requires(System system) { system.is_control_system; };

template <> class ControlSystem<> {
public:
    static constexpr bool is_control_system = true;
    static constexpr bool is_empty = true;
    ControlSystem() = default;
    template <class OutputType, class InputType>
    ControlSystem<OutputType, InputType> connect(ControlBlock<InputType, OutputType>& next);
    template <class OutputType, class InputType>
    void execute(ControlSystem<OutputType, InputType>* parent);
    template <class OutputType, class InputType>
    void input(ControlSystem<OutputType, InputType>* parent, InputType input);
    template <class System, class Input, class Output, class... ControlBlocks>
    static auto
    create_control_system(System base, ControlBlock<Input, Output>& block, ControlBlocks&... blocks)
        requires ControlSystemConcept<System>;
    template <class Input, class Output, class... ControlBlocks>
    static auto create_control_system(ControlBlock<Input, Output>& block, ControlBlocks&... blocks);
};

ControlSystem() -> ControlSystem<>;

template <class OutputType, class InputType, class... FollowingTypes>
class ControlSystem<OutputType, InputType, FollowingTypes...>
    : public ControlSystem<FollowingTypes...> {
public:
    static constexpr bool is_control_system = true;
    static constexpr bool is_empty = false;
    ControlBlock<InputType, OutputType>* control_block;
    ControlSystem() = default;
    ControlSystem(
        ControlBlock<InputType, OutputType>& control_block,
        const ControlSystem<FollowingTypes...>& following_system
    );
    template <class IncomingOutputType, class IncomingInputType>
    ControlSystem<IncomingOutputType, IncomingInputType, OutputType, InputType, FollowingTypes...>
    connect(ControlBlock<IncomingInputType, IncomingOutputType>& next);
    template <class IncomingOutputType, class IncomingInputType>
    void execute(ControlSystem<
                 IncomingOutputType,
                 IncomingInputType,
                 OutputType,
                 InputType,
                 FollowingTypes...>* parent);
    void execute();
    template <class FirstInputType>
    void input(
        ControlSystem<OutputType, InputType, FollowingTypes...>* parent,
        FirstInputType input_value
    );
    template <class FirstInputType> void input(FirstInputType input_value);
};

template <class OutputType, class InputType, class... FollowingTypes>
ControlSystem(ControlBlock<InputType, OutputType>&, ...)
    -> ControlSystem<OutputType, InputType, FollowingTypes...>;

template <class OutputType, class InputType>
ControlSystem<OutputType, InputType>
ControlSystem<>::connect(ControlBlock<InputType, OutputType>& next) {
    ControlSystem<OutputType, InputType> new_system(next, *this);
    return new_system;
}

template <class System, class Input, class Output, class... ControlBlocks>
auto ControlSystem<>::create_control_system(
    System base,
    ControlBlock<Input, Output>& block,
    ControlBlocks&... blocks
)
    requires ControlSystemConcept<System>
{
    if constexpr (sizeof...(ControlBlocks) < 1) {
        return base.connect(block);
    } else {
        return create_control_system(base.connect(block), blocks...);
    }
}

template <class Input, class Output, class... ControlBlocks>
auto ControlSystem<>::create_control_system(
    ControlBlock<Input, Output>& block,
    ControlBlocks&... blocks
) {
    ControlSystem<> base;
    return create_control_system(base, block, blocks...);
}

template <class OutputType, class InputType>
void ControlSystem<>::execute(ControlSystem<OutputType, InputType>* parent) {}

template <class OutputType, class InputType>
void ControlSystem<>::input(ControlSystem<OutputType, InputType>* parent, InputType input_value) {
    parent->control_block->input(input_value);
}

template <class OutputType, class InputType, class... FollowingTypes>
ControlSystem<OutputType, InputType, FollowingTypes...>::ControlSystem(
    ControlBlock<InputType, OutputType>& control_block,
    const ControlSystem<FollowingTypes...>& following_system
)
    : ControlSystem<FollowingTypes...>(following_system) {
    if (is_empty == false) {
        this->control_block = &control_block;
    }
}

template <class OutputType, class InputType, class... FollowingTypes>
template <class IncomingOutputType, class IncomingInputType>
void ControlSystem<OutputType, InputType, FollowingTypes...>::execute(
    ControlSystem<IncomingOutputType, IncomingInputType, OutputType, InputType, FollowingTypes...>*
        parent
) {
    if constexpr (is_empty == false) {
        static_cast<ControlSystem<FollowingTypes...>*>(this)->execute(this);
        control_block->execute();
        parent->control_block->input(control_block->output_value);
    }
}

template <class OutputType, class InputType, class... FollowingTypes>
template <class FirstInputType>
void ControlSystem<OutputType, InputType, FollowingTypes...>::input(
    ControlSystem<OutputType, InputType, FollowingTypes...>* parent,
    FirstInputType input_value
) {
    if constexpr (is_empty == false) {
        static_cast<ControlSystem<FollowingTypes...>*>(this)->input(this, input_value);
    }
}

template <class OutputType, class InputType, class... FollowingTypes>
template <class FirstInputType>
void ControlSystem<OutputType, InputType, FollowingTypes...>::input(FirstInputType input_value) {
    input(this, input_value);
}

template <class OutputType, class InputType, class... FollowingTypes>
void ControlSystem<OutputType, InputType, FollowingTypes...>::execute() {
    static_cast<ControlSystem<FollowingTypes...>*>(this)->execute(this);
    control_block->execute();
}

template <class OutputType, class InputType, class... FollowingTypes>
template <class IncomingOutputType, class IncomingInputType>
ControlSystem<IncomingOutputType, IncomingInputType, OutputType, InputType, FollowingTypes...>
ControlSystem<OutputType, InputType, FollowingTypes...>::connect(
    ControlBlock<IncomingInputType, IncomingOutputType>& next
) {
    ControlSystem<IncomingOutputType, IncomingInputType, OutputType, InputType, FollowingTypes...>
        new_system(next, *this);
    return new_system;
}
