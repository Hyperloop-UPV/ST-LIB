#pragma once

#include "Protection.hpp"
#include "StateMachine/StateMachine.hpp"
#include "HALAL/HALAL.hpp"
#include "Notification.hpp"

#define getname(var) #var
#define add_protection(src,...)  \
		{\
            Protection& ref = ProtectionManager::_add_protection(src,__VA_ARGS__); \
            if (getname(src)[0] == '&'){ \
            	ref.set_name((char*)malloc(sizeof(getname(src))-1));\
            	sprintf(ref.get_name(),"%s",getname(src)+1); \
            }else{\
            	ref.set_name((char*)malloc(sizeof(getname(src))));\
            	sprintf(ref.get_name(),"%s",getname(src)); \
            }\
		}\

#define add_high_frequency_protection(src,...)  \
		{\
            Protection& ref = ProtectionManager::_add_high_frequency_protection(src,__VA_ARGS__); \
            if (getname(src)[0] == '&'){ \
            	ref.set_name((char*)malloc(sizeof(getname(src))-1)); \
            	sprintf(ref.get_name(),"%s",getname(src)+1); \
            }else{\
            	ref.set_name((char*)malloc(sizeof(getname(src))));\
            	sprintf(ref.get_name(),"%s",getname(src)); \
            }\
		}\

class ProtectionManager {
public:
	typedef uint8_t state_id;
	static bool external_trigger;

	static const uint64_t notify_delay_in_nanoseconds = 100'000'000;
	static uint64_t last_notify;

    static void set_id(Boards::ID id);

    static void link_state_machine(StateMachine& general_state_machine, state_id fault_id);

    template<class Type, ProtectionType... Protector, template<class,ProtectionType> class Boundaries>
    static Protection& _add_protection(Type* src, Boundaries<Type,Protector>... protectors){
    	low_frequency_protections.push_back(Protection(src,protectors...));
        return low_frequency_protections.back();
    }

    template<class Type, ProtectionType... Protector, template<class,ProtectionType> class Boundaries>
    static Protection& _add_high_frequency_protection(Type* src, Boundaries<Type,Protector>... protectors){
    	high_frequency_protections.push_back(Protection(src,protectors...));
        return high_frequency_protections.back();
    }
    /**
     * @brief call on startup to initialize the names of the protections
    */
    static void initialize();
    static void add_standard_protections();
    static void check_protections();
    static void check_high_frequency_protections();
    static void warn(string message);
    static void fault_and_propagate();
    static void propagate_fault();
    static void notify(Protection& protection);
private:
	static constexpr uint16_t warning_id = 2;
	static constexpr uint16_t fault_id = 3;
	static char* message;
	static size_t message_size;
    static bool test_fault;
	static constexpr const char* format = "{\"boardId\": %s, \"timestamp\":{%s}, %s}";

    static Boards::ID board_id;
    static vector<Protection> low_frequency_protections;
    static vector<Protection> high_frequency_protections;
    static StateMachine* general_state_machine;
    static state_id fault_state_id;

    static Notification fault_notification;
    static Notification warning_notification;
    static StackOrder<0> fault_order;

    static void tcp_to_fault();
    static void to_fault();
    static void external_to_fault();
};


