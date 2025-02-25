#pragma once

#include "C++Utilities/CppUtils.hpp"

template<class... Types> class stack_tuple;

template<>
class stack_tuple<> {
public:
    stack_tuple() = default;
    template<class FunctionType>
    void for_each(FunctionType function) {}
};

template<class Type, class... Types>
class stack_tuple<Type, Types...>: public stack_tuple<Types...> {
public:
    Type value;
    stack_tuple() = default;
    stack_tuple(Type value, Types... values): stack_tuple<Types...>(values...), value(value) {}
    template<class FunctionType>
    void for_each(FunctionType function) {
        function(value);
        stack_tuple<Types...>::for_each(function);
    }
};
