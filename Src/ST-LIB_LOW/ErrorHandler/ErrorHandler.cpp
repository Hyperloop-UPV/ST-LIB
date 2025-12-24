/*
 * ErrorHandler.cpp
 *
 *  Created on: Dec 22, 2022
 *      Author: Pablo
 */

#include "ErrorHandler/ErrorHandler.hpp"

string ErrorHandlerModel::description = "Error-No-Description-Found";
string ErrorHandlerModel::line = "Error-No-Line-Found";
string ErrorHandlerModel::func = "Error-No-Func-Found";
string ErrorHandlerModel::file = "Error-No-File-Found";
double ErrorHandlerModel::error_triggered = 0;
bool ErrorHandlerModel::error_to_communicate = false;

void ErrorHandlerModel::SetMetaData(int line, const char * func, const char * file){
    // Minimal implementation to avoid std::string overhead
}

void ErrorHandlerModel::ErrorHandlerTrigger(string format, ... ){
    // Minimal implementation to avoid std::string and vsnprintf overhead
    if (ErrorHandlerModel::error_triggered) {
        return;
    }
    ErrorHandlerModel::error_triggered = 1.0;
    
    // Just loop forever or reset
    while(1) {}
}

void ErrorHandlerModel::ErrorHandlerUpdate(){
    // Minimal implementation
}

