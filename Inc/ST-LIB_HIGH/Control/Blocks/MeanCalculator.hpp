#pragma once

#include "../ControlBlock.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
template <int N> class MeanCalculator : public ControlBlock<double, double> {
public:
    int index = 0;
    double mean = 0.0;

public:
    MeanCalculator() : ControlBlock<double, double>(0.0) { output_value = 0.0; }
    void execute() override {
        mean += input_value / N;
        index++;
        if (index > N)
            ErrorHandler("MeanCalculator is receiving just 0");
        if (index == N)
            output_value = mean;
        else
            output_value = 0.0;
        return;
    }
    void reset() {
        mean = 0.0;
        index = 0;
        output_value = 0.0;
    }
};
