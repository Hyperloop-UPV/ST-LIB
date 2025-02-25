#pragma once

#include "../ControlBlock.hpp"

enum FilterDerivatorType{
    None,
    ForwardEuler,
    BackwardEuler,
    Trapezoidal,
	Moving_Average
};

template<int N>
class BDFDerivator : public ControlBlock<double,double>{
    public:
        double period;
        static constexpr int order = N-1;
        double buffer[N+1] = {0.0};
        double (&ks)[order];
        int index = 0;
        uint64_t counter = 0;
    public:
        BDFDerivator(double period, double(&ks)[order]): period(period), ks(ks){this->output_value = 0.0;}
        BDFDerivator(double period, double(&&ks)[order]): period(period), ks(move(ks)){this->output_value = 0.0;}
        void execute()override{
            if(counter++<order) {
                buffer[index] = input_value;
                index++;
                index %= (N+1);
                output_value = 0.0;
                return;
            }
            buffer[index] = input_value;
            output_value = 0.0;
            for(int i = 1; i <= order; i++){
                output_value += ks[i-1]*(buffer[index]-buffer[((index-i)%(N+1) + (N+1))%(N+1)])/i;
            }
            output_value /= (period*order);
            index++;
            index %= (N+1);
            counter--;
        }

        void reset(){
        	output_value = 0;
        	counter = 0.0;
        	for(double& e : buffer){
        		e = 0.0;
        	}
        }
};

class SimpleDerivator : public ControlBlock<double,double>{
    public:
        double period;
        static constexpr int N = 2;
        double buffer[N] = {0.0};
        int index = 0;
    public:
        SimpleDerivator(double period): ControlBlock<double,double>(0.0), period(period){ output_value = 0.0;}
        void execute()override{
            buffer[index] = input_value;
            output_value = (buffer[index] - buffer[((index-1)%(N) + (N))%(N)])/period;
            index++;
            index %= N;
            return;
        }
        void reset(){
        	output_value = 0;
            index = 0;
            for(int i = 0; i < N; i++){
                buffer[i] = 0.0;
            }
        }
};

class SimpleFloatDerivator : public ControlBlock<float,float>{
    public:
        float period;
        static constexpr int N = 2;
        float buffer[N] = {0.0};
        int index = 0;
    public:
        SimpleFloatDerivator(float period): ControlBlock<float,float>(0.0), period(period){ output_value = 0.0;}
        void execute()override{
            buffer[index] = input_value;
            output_value = (buffer[index] - buffer[((index-1)%(N) + (N))%(N)])/period;
            index++;
            index %= N;
            return;
        }
        void reset(){
        	output_value = 0;
            index = 0;
            for(int i = 0; i < N; i++){
                buffer[i] = 0.0;
            }
        }
};
