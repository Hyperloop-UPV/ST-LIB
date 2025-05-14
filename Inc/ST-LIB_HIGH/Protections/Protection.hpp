#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "Boundary.hpp"



class Protection{
private:
    char* name = nullptr;
    vector<shared_ptr<BoundaryInterface>> boundaries;
    BoundaryInterface* fault_protection = nullptr;
    static constexpr Protections::FaultType fault_type = Protections::FaultType::FAULT;
    uint8_t triggered_protecions_idx[4]{};
    uint8_t triggered_oks_idx[4]{};
    uint64_t last_notify_tick{0};
public:
    const uint64_t get_last_notify_tick()const{
        return last_notify_tick;
    }
    void update_last_notify_tick(uint64_t new_tick){
        last_notify_tick = new_tick;
    }
    vector<shared_ptr<BoundaryInterface>> warnings_triggered;
    vector<shared_ptr<BoundaryInterface>> oks_triggered;
    template<class Type, ProtectionType... Protector, template<class,ProtectionType> class Boundaries>
    Protection(Type* src, Boundaries<Type,Protector>... protectors) {
        (boundaries.push_back(shared_ptr<BoundaryInterface>(new Boundary<Type,Protector>(src,protectors))), ...);
    }

    void set_name(char* name) {
        this->name = name;
    }

    char* get_name() {
        return name;
    }

    Protections::FaultType check_state() {
        uint8_t warning_count = 0;
        uint8_t oks_count = 0;
        //to save the index of the triggered warning
        uint8_t idx = 0;
        for(shared_ptr<BoundaryInterface>& bound: boundaries){
            auto fault_type = bound->check_bounds();
            idx++;
            fault_protection = nullptr;
            switch(fault_type){
                // in case a Protection has more than one boundary, give priority to fault messages
                case Protections::FAULT:
                    fault_protection = bound.get();
                    // adding the fault_protection to the vector is not desired,
                    // the fault signal should propagate as fast as possible
                    if(bound->warning_already_triggered){}
                    else{
                        bound->warning_already_triggered = true;
                    }
                    return Protections::FAULT;
                case Protections::WARNING:
                    //warnings are non fatal, but we cannot waste time, we need to check if any
                    //faults were triggered
                    if(bound->warning_already_triggered)
                        break;
                    bound->warning_already_triggered = true;
                    triggered_protecions_idx[warning_count] = idx-1;
                    warning_count++;
                    break;
                case Protections::OK:
                    if(bound->warning_already_triggered){
                        bound->back_to_normal = true;
                    }
                    bound->warning_already_triggered = false;
                    if(bound->back_to_normal && bound->boundary_type_id != INFO_WARNING -2){
                        triggered_oks_idx[oks_count] = idx-1;
                        oks_count++;
                        bound->back_to_normal = false;
                    }
                    
                    break;
                default:
                    ErrorHandler("INVALID Protection::STATE type");
                    break;
            }
        }
        if(oks_count){
            for(uint8_t i = 0; i < oks_count; i++){
                oks_triggered.push_back(boundaries[triggered_oks_idx[i]]);
            }
        }
        if(warning_count){
            for(uint8_t i = 0; i < warning_count; i++){
                warnings_triggered.push_back(boundaries[triggered_protecions_idx[i]]);
            }
            return Protections::WARNING;
        }
        return Protections::OK;
    }
    friend class ProtectionManager;
};
