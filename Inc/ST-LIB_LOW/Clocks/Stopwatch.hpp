/*
 * Time.hpp
 *
 *  Created on: 11 nov. 2022
 *      Author: Dani
 */

#pragma once
#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

class Stopwatch {

private:
    map<const string, uint64_t> start_times;

public:
    void start(const string);
    uint64_t stop(const string);
};
