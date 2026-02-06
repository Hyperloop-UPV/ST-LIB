#pragma once
#define PROTECTIONTYPE_LENGTH 8
#include "C++Utilities/CppUtils.hpp"
#include "Control/Blocks/MeanCalculator.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/Packets/Order.hpp"
#include "HALAL/Services/InfoWarning/InfoWarning.hpp"
#include "HALAL/Services/Time/RTC.hpp"

using type_id_t = void (*)();
template <typename>
void type_id() {}

namespace Protections {
enum FaultType : uint8_t { FAULT = 0, WARNING, OK };
}

enum ProtectionType : uint8_t {
    BELOW = 0,
    ABOVE,
    OUT_OF_RANGE,
    EQUALS,
    NOT_EQUALS,
    ERROR_HANDLER,
    TIME_ACCUMULATION,
    INFO_WARNING
};

struct BoundaryInterface {
   public:
    virtual Protections::FaultType check_bounds() = 0;
    HeapOrder* fault_message{nullptr};
    HeapOrder* warn_message{nullptr};
    HeapOrder* ok_message{nullptr};
    void update_name(char* n) {
        name = n;
        if (strlen(n) > NAME_MAX_LEN) {
            ErrorHandler("Variable name is too long, max length is %d",
                         NAME_MAX_LEN);
            return;
        }
        string_len = name.size();
    }
    virtual void update_error_handler_message(
        [[maybe_unused]] const char* err_message) {}
    virtual void update_warning_message(
        [[maybe_unused]] const char* warn_message) {}
    static const char* get_error_handler_string() {
        return ErrorHandlerModel::description.c_str();
    }
    static const char* get_warning_string() {
        return InfoWarning::description.c_str();
    }
    uint8_t boundary_type_id{};
    // used to send messages only on raising/failing edges
    bool warning_already_triggered{false};
    bool warning_is_up{false};
    bool back_to_normal{false};

   protected:
    static const map<type_id_t, uint8_t> format_look_up;
    static int get_error_handler_string_size() {
        return ErrorHandlerModel::description.size();
    }
    static int get_warning_string_size() {
        return InfoWarning::description.size();
    }

    // this will store the name of the variable
    string name;
    // max variable name
    static constexpr uint8_t NAME_MAX_LEN = 40;
    uint8_t format_id{255};
    uint8_t string_len{0};
};

template <class Type, ProtectionType Protector>
struct Boundary;

template <class Type>
struct Boundary<Type, BELOW> : public BoundaryInterface {
    static constexpr ProtectionType Protector = BELOW;
    bool has_warning_level{false};
    Type* src = nullptr;
    Type boundary;
    Type warning_threshold{std::numeric_limits<Type>::max()};
    // to get a snapshot of the value when the protection is triggered
    Type frozen_value{};
    constexpr Boundary(const Type warn, const Type bound)
        : has_warning_level{true}, boundary(bound), warning_threshold(warn) {
        // i havent been able to find a way to do a static_assertion.
        if (warn < bound) {
            ErrorHandler("Warning threshold is below boundary");
        }
    };
    Boundary(Type boundary) : boundary(boundary) {}
    Boundary(Type* src, Boundary<Type, Protector> boundary)
        : has_warning_level(boundary.has_warning_level),
          src(src),
          boundary(boundary.boundary) {
        // we have to do this because we cannot take address of rvalue
        // (ProtectionType::BELOW)
        boundary_type_id = Protector;
        format_id = BoundaryInterface::format_look_up.at(type_id<Type>);
        // we have to preallocate space, otherwise the might get moved around,
        // invalidating the pointer, better safe than sorry
        name.reserve(NAME_MAX_LEN);
        if (this->has_warning_level) {
            warning_threshold = boundary.warning_threshold;
            warn_message = new HeapOrder(
                uint16_t{2000}, &format_id, &boundary_type_id, &name,
                &this->warning_threshold, &this->frozen_value,
                &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
                &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
                &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
                &Global_RTC::global_RTC.year);
            ok_message = new HeapOrder(
                uint16_t{3000}, &format_id, &boundary_type_id, &name,
                &this->warning_threshold, &this->frozen_value,
                &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
                &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
                &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
                &Global_RTC::global_RTC.year);
        } else {
            ok_message = new HeapOrder(
                uint16_t{3000}, &format_id, &boundary_type_id, &name,
                &this->boundary, &this->frozen_value,
                &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
                &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
                &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
                &Global_RTC::global_RTC.year);
        }

        fault_message = new HeapOrder(
            uint16_t{1000}, &format_id, &boundary_type_id, &name,
            &this->boundary, &this->frozen_value,
            &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
            &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
            &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
            &Global_RTC::global_RTC.year);
    }

    Boundary(Type* src, Type boundary) : src(src), boundary(boundary) {}
    Protections::FaultType check_bounds() override {
        frozen_value = *src;
        if (*src < boundary) {
            return Protections::FAULT;
        }
        if (has_warning_level && *src < warning_threshold) {
            return Protections::WARNING;
        }
        return Protections::OK;
    }
};

template <class Type>
struct Boundary<Type, ABOVE> : public BoundaryInterface {
    static constexpr ProtectionType Protector = ABOVE;
    bool has_warning_level{false};
    Type* src = nullptr;
    Type boundary{};
    Type warning_threshold{};
    Type frozen_value{};

    Boundary(Type warning_threshold, Type boundary)
        : has_warning_level{true},
          boundary(boundary),
          warning_threshold(warning_threshold) {
        if (warning_threshold > boundary) {
            ErrorHandler("Warning threshold is above boundary");
        }
    };
    Boundary(Type boundary) : boundary(boundary) {};
    Boundary(Type* src, Boundary<Type, Protector> boundary)
        : has_warning_level(boundary.has_warning_level),
          src(src),
          boundary(boundary.boundary) {
        // we have to do this because we cannot take address of rvalue
        // (ProtectionType::BELOW)
        boundary_type_id = Protector;
        format_id = BoundaryInterface::format_look_up.at(type_id<Type>);
        // we have to preallocate space, otherwise the might get moved around,
        // invalidating the pointer, better safe than sorry
        name.reserve(NAME_MAX_LEN);
        if (this->has_warning_level) {
            warning_threshold = boundary.warning_threshold;
            warn_message = new HeapOrder(
                uint16_t{2111}, &format_id, &boundary_type_id, &name,
                &this->warning_threshold, &this->frozen_value,
                &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
                &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
                &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
                &Global_RTC::global_RTC.year);
            ok_message = new HeapOrder(
                uint16_t{3111}, &format_id, &boundary_type_id, &name,
                &this->warning_threshold, &this->frozen_value,
                &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
                &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
                &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
                &Global_RTC::global_RTC.year);

        } else {
            ok_message = new HeapOrder(
                uint16_t{3111}, &format_id, &boundary_type_id, &name,
                &this->boundary, &this->frozen_value,
                &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
                &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
                &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
                &Global_RTC::global_RTC.year);
        }

        fault_message = new HeapOrder(
            uint16_t{1111}, &format_id, &boundary_type_id, &name,
            &this->boundary, &this->frozen_value,
            &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
            &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
            &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
            &Global_RTC::global_RTC.year);
    }
    Boundary(Type* src, Type boundary) : src(src), boundary(boundary) {}
    Protections::FaultType check_bounds() override {
        frozen_value = *src;
        if (*src > boundary) return Protections::FAULT;
        if (has_warning_level && *src > warning_threshold) {
            return Protections::WARNING;
        }
        return Protections::OK;
    }
};

template <class Type>
struct Boundary<Type, EQUALS> : public BoundaryInterface {
    static constexpr ProtectionType Protector = EQUALS;
    Type* src = nullptr;
    Type boundary;
    Type frozen_value{};

    Boundary(Type boundary) : boundary(boundary) {};
    Boundary(Type* src, Boundary<Type, Protector> boundary)
        : src(src), boundary(boundary.boundary) {
        boundary_type_id = Protector;
        format_id = BoundaryInterface::format_look_up.at(type_id<Type>);
        name.reserve(NAME_MAX_LEN);
        fault_message = new HeapOrder(
            uint16_t{1333}, &format_id, &boundary_type_id, &name,
            &this->boundary, &this->frozen_value,
            &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
            &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
            &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
            &Global_RTC::global_RTC.year);
        ok_message = new HeapOrder(
            uint16_t{2333}, &format_id, &boundary_type_id, &name,
            &this->boundary, &this->frozen_value,
            &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
            &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
            &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
            &Global_RTC::global_RTC.year);
    }
    Boundary(Type* src, Type boundary) : src(src), boundary(boundary) {}
    Protections::FaultType check_bounds() override {
        if (*src == boundary) return Protections::FAULT;
        return Protections::OK;
    }
};

template <class Type>
struct Boundary<Type, NOT_EQUALS> : public BoundaryInterface {
    static constexpr ProtectionType Protector = NOT_EQUALS;
    Type* src = nullptr;
    Type boundary;
    Type frozen_value{};

    Boundary(Type boundary) : boundary(boundary) {};
    Boundary(Type* src, Boundary<Type, Protector> boundary)
        : src(src), boundary(boundary.boundary) {
        boundary_type_id = Protector;
        format_id = BoundaryInterface::format_look_up.at(type_id<Type>);
        name.reserve(NAME_MAX_LEN);
        fault_message = new HeapOrder(
            uint16_t{1444}, &format_id, &boundary_type_id, &name,
            &this->boundary, &this->frozen_value,
            &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
            &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
            &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
            &Global_RTC::global_RTC.year);
        ok_message = new HeapOrder(
            uint16_t{2444}, &format_id, &boundary_type_id, &name,
            &this->boundary, &this->frozen_value,
            &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
            &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
            &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
            &Global_RTC::global_RTC.year);
    }
    Boundary(Type* src, Type boundary) : src(src), boundary(boundary) {}
    Protections::FaultType check_bounds() override {
        frozen_value = *src;
        if (*src != boundary) return Protections::FAULT;
        return Protections::OK;
    }
};

template <class Type>
struct Boundary<Type, OUT_OF_RANGE> : public BoundaryInterface {
    static constexpr ProtectionType Protector = OUT_OF_RANGE;
    Type* src = nullptr;
    Type lower_warning;
    Type upper_warning;
    Type lower_boundary;
    Type upper_boundary;
    Type frozen_value{};

    bool has_warning_level{false};
    Boundary(Type lower_warning, Type upper_warning, Type lower_boundary,
             Type upper_boundary)
        : lower_warning(lower_warning),
          upper_warning(upper_warning),
          lower_boundary(lower_boundary),
          upper_boundary(upper_boundary),
          has_warning_level{true}
        {
        if (lower_warning < lower_boundary || upper_warning > upper_boundary) {
            ErrorHandler("Warning thresholds are outside of boundaries");
        }
    };
    Boundary(Type lower_boundary, Type upper_boundary)
        :   lower_warning(std::numeric_limits<Type>::min()),
            upper_warning(std::numeric_limits<Type>::max()),  
            lower_boundary(lower_boundary),
            upper_boundary(upper_boundary)
        {};
    Boundary(Type* src, Boundary<Type, Protector> boundary)
        : src(src),
          lower_boundary(boundary.lower_boundary),
          upper_boundary(boundary.upper_boundary) {
        boundary_type_id = Protector;
        format_id = BoundaryInterface::format_look_up.at(type_id<Type>);
        name.reserve(NAME_MAX_LEN);
        if (boundary.has_warning_level) {
            lower_warning = boundary.lower_warning;
            upper_warning = boundary.upper_warning;
            warn_message = new HeapOrder(
                uint16_t{2222}, &format_id, &boundary_type_id, &name,
                &boundary.lower_boundary, &boundary.upper_boundary,
                &this->frozen_value, &Global_RTC::global_RTC.counter,
                &Global_RTC::global_RTC.second, &Global_RTC::global_RTC.minute,
                &Global_RTC::global_RTC.hour, &Global_RTC::global_RTC.day,
                &Global_RTC::global_RTC.month, &Global_RTC::global_RTC.year);
            ok_message = new HeapOrder(
                uint16_t{3222}, &format_id, &boundary_type_id, &name,
                &boundary.lower_warning, &boundary.upper_warning,
                &this->frozen_value, &Global_RTC::global_RTC.counter,
                &Global_RTC::global_RTC.second, &Global_RTC::global_RTC.minute,
                &Global_RTC::global_RTC.hour, &Global_RTC::global_RTC.day,
                &Global_RTC::global_RTC.month, &Global_RTC::global_RTC.year);

        } else {
            ok_message = new HeapOrder(
                uint16_t{3222}, &format_id, &boundary_type_id, &name,
                &this->lower_boundary, &this->upper_boundary,
                &this->frozen_value, &Global_RTC::global_RTC.counter,
                &Global_RTC::global_RTC.second, &Global_RTC::global_RTC.minute,
                &Global_RTC::global_RTC.hour, &Global_RTC::global_RTC.day,
                &Global_RTC::global_RTC.month, &Global_RTC::global_RTC.year);
        }

        fault_message = new HeapOrder(
            uint16_t{1222}, &format_id, &boundary_type_id, &name,
            &this->lower_boundary, &this->upper_boundary, &this->frozen_value,
            &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
            &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
            &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
            &Global_RTC::global_RTC.year);
    }
    Boundary(Type* src, Type lower_boundary, Type upper_boundary)
        : src(src),
          lower_boundary(lower_boundary),
          upper_boundary(upper_boundary) {}
    Protections::FaultType check_bounds() override {
        frozen_value = *src;
        if (*src < lower_boundary || *src > upper_boundary)
            return Protections::FAULT;
        if (has_warning_level &&
            ((*src < lower_boundary) || (*src > upper_boundary))) {
            return Protections::WARNING;
        }
        return Protections::OK;
    }
};

template <>
struct Boundary<void, ERROR_HANDLER> : public BoundaryInterface {
    static constexpr ProtectionType Protector = ERROR_HANDLER;
    Boundary(void*) {
        boundary_type_id = Protector;
        error_handler_string.reserve(ERROR_HANDLER_MSG_MAX_LEN);
        fault_message = new HeapOrder(
            uint16_t{1555}, &padding, &boundary_type_id, &name,
            &error_handler_string, &Global_RTC::global_RTC.counter,
            &Global_RTC::global_RTC.second, &Global_RTC::global_RTC.minute,
            &Global_RTC::global_RTC.hour, &Global_RTC::global_RTC.day,
            &Global_RTC::global_RTC.month, &Global_RTC::global_RTC.year);
    }
    uint8_t padding{};
    Boundary(void*, Boundary<void, ERROR_HANDLER>) {
        boundary_type_id = Protector;
        error_handler_string.reserve(ERROR_HANDLER_MSG_MAX_LEN);
        fault_message = new HeapOrder(
            uint16_t{1555}, &padding, &boundary_type_id, &name,
            &error_handler_string, &Global_RTC::global_RTC.counter,
            &Global_RTC::global_RTC.second, &Global_RTC::global_RTC.minute,
            &Global_RTC::global_RTC.hour, &Global_RTC::global_RTC.day,
            &Global_RTC::global_RTC.month, &Global_RTC::global_RTC.year);
    }
    Boundary() = default;
    Protections::FaultType check_bounds() override {
        return not ErrorHandlerModel::error_triggered ? Protections::OK
                                                      : Protections::FAULT;
    }
    void update_error_handler_message(const char* err_message) override {
        error_handler_string = err_message;
        if (strlen(err_message) > ERROR_HANDLER_MSG_MAX_LEN) {
            ErrorHandler("Error Handler message is too long, max length is %d",
                         ERROR_HANDLER_MSG_MAX_LEN);
            return;
        }
        error_handler_string_len = error_handler_string.size();
    }

   private:
    string error_handler_string{};
    uint16_t error_handler_string_len{};
    static constexpr uint16_t ERROR_HANDLER_MSG_MAX_LEN = 255;
};

template <>
struct Boundary<void, INFO_WARNING> : public BoundaryInterface {
    static constexpr ProtectionType Protector = INFO_WARNING;
    Boundary(void*) {
        boundary_type_id = Protector - 2;
        warning_string.reserve(WARNING_HANDLER_MSG_MAX_LEN);
        warn_message = new HeapOrder(
            uint16_t{2555}, &padding, &boundary_type_id, &name, &warning_string,
            &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
            &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
            &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
            &Global_RTC::global_RTC.year);
    }
    uint8_t padding{};
    Boundary(void*, Boundary<void, INFO_WARNING>) {
        // SW are crybabies
        boundary_type_id = Protector - 2;
        warning_string.reserve(WARNING_HANDLER_MSG_MAX_LEN);
        warn_message = new HeapOrder(
            uint16_t{2555}, &padding, &boundary_type_id, &name, &warning_string,
            &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
            &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
            &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
            &Global_RTC::global_RTC.year);
    }
    Boundary() = default;
    Protections::FaultType check_bounds() override {
        return InfoWarning::warning_triggered ? Protections::WARNING
                                              : Protections::OK;
    }
    void update_warning_message(const char* warn_message) override {
        warning_string = warn_message;
        if (strlen(warn_message) > WARNING_HANDLER_MSG_MAX_LEN) {
            ErrorHandler("Error Handler message is too long, max length is %d",
                         WARNING_HANDLER_MSG_MAX_LEN);
            return;
        }
        warning_string_len = warning_string.size();
    }

   private:
    string warning_string{};
    uint16_t warning_string_len{};
    static constexpr uint16_t WARNING_HANDLER_MSG_MAX_LEN = 255;
};

template <typename Type>
    requires(std::is_floating_point_v<Type>)
struct Boundary<Type, TIME_ACCUMULATION> : public BoundaryInterface {
    static constexpr ProtectionType Protector = TIME_ACCUMULATION;
    Boundary(Type bound, float time_limit, float frequency,
             Boundary<Type, Protector>*& external_pointer)
        : real_still_good(new Protections::FaultType{Protections::OK}),
          bound(bound),
          time_limit(time_limit),
          frequency(frequency),
          moving_order(frequency * time_limit / 100),
          external_pointer(&external_pointer) {
        external_pointer = this;
    };
    Boundary(Type warning_threshold, Type bound, float time_limit,
             float frequency, Boundary<Type, Protector>*& external_pointer)
        : real_still_good(new Protections::FaultType{Protections::OK}),
          bound(bound),
          time_limit(time_limit),
          frequency(frequency),
          moving_order(frequency * time_limit / 100),
          external_pointer(&external_pointer) {
        external_pointer = this;
        has_warning_level = true;
        this->warning_threshold = warning_threshold;
    };
    Boundary(Type* src, Boundary<Type, Protector> boundary)
        : real_still_good(boundary.real_still_good),
          src(src),
          bound(boundary.bound),
          time_limit(boundary.time_limit),
          frequency(boundary.frequency),
          moving_order(frequency * time_limit / 100),
          external_pointer(boundary.external_pointer) {
        *external_pointer = this;
        boundary_type_id = Protector;
        format_id = BoundaryInterface::format_look_up.at(type_id<Type>);
        name.reserve(NAME_MAX_LEN);
        if (boundary.has_warning_level) {
            warning_threshold = boundary.warning_threshold;
            warn_message = new HeapOrder(
                uint16_t{2666}, &format_id, &boundary_type_id, &name,
                &this->warning_threshold, &this->bound, &this->frozen_value,
                &this->time_limit, &this->frequency,
                &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
                &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
                &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
                &Global_RTC::global_RTC.year);
            ok_message = new HeapOrder(
                uint16_t{3666}, &format_id, &boundary_type_id, &name,
                &this->warning_threshold, &this->bound, &this->frozen_value,
                &this->time_limit, &this->frequency,
                &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
                &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
                &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
                &Global_RTC::global_RTC.year);
        }
        fault_message = new HeapOrder(
            uint16_t{1666}, &format_id, &boundary_type_id, &name, &this->bound,
            &this->frozen_value, &this->time_limit, &this->frequency,
            &Global_RTC::global_RTC.counter, &Global_RTC::global_RTC.second,
            &Global_RTC::global_RTC.minute, &Global_RTC::global_RTC.hour,
            &Global_RTC::global_RTC.day, &Global_RTC::global_RTC.month,
            &Global_RTC::global_RTC.year);
    }
    Boundary(Type* src, Type bound, float time_limit, float frequency)
        : real_still_good(new Protections::FaultType{Protections::OK}),
          src(src),
          bound(bound),
          time_limit(time_limit),
          frequency(frequency),
          moving_order(frequency * time_limit / 100),
          external_pointer(nullptr) {}
    bool has_warning_level{false};
    Type warning_threshold;
    uint8_t format_id{};
    Type* src = nullptr;
    Type bound;
    float time_limit;
    float frequency;
    Protections::FaultType* real_still_good = nullptr;
    Protections::FaultType still_good = Protections::OK;
    Boundary<Type, Protector>** external_pointer;

    MeanCalculator<100> mean_calculator;
    vector<Type> mean_moving_average;
    uint16_t moving_order = 0;
    uint16_t moving_last = -1;
    uint16_t moving_first = 0;
    uint16_t moving_counter = 0;
    Type accumulator{};

    Protections::FaultType check_accumulation(Type value) {
        if (still_good == Protections::FAULT) return Protections::FAULT;
        mean_calculator.input(abs(value));
        mean_calculator.execute();
        if (mean_calculator.output_value == 0) {
            return Protections::OK;
        }
        mean_calculator.reset();
        if (moving_counter < moving_order) {
            moving_last++;
            mean_moving_average[moving_last] = mean_calculator.output_value;
            accumulator += mean_calculator.output_value / moving_order;
            moving_counter++;
            return Protections::OK;
        }
        accumulator -= mean_moving_average[moving_first] / moving_order;
        moving_first = (moving_first + 1) % moving_order;
        moving_last = (moving_last + 1) % moving_counter;
        mean_moving_average[moving_last] = mean_calculator.output_value;
        accumulator += mean_moving_average[moving_last] / moving_order;
        // we check by decreasing order.
        if (accumulator > bound) {
            *real_still_good = Protections::FAULT;
            still_good = Protections::FAULT;
            return Protections::FAULT;
        } else if (has_warning_level && accumulator > warning_threshold) {
            return Protections::WARNING;
        }
        return Protections::OK;
    }

    Protections::FaultType check_bounds() override {
        still_good = *real_still_good;
        return still_good;
    }
};
