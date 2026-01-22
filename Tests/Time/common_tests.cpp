#include <gtest/gtest.h>
#include "ErrorHandler/ErrorHandler.hpp"

std::string ErrorHandlerModel::line;
std::string ErrorHandlerModel::func;
std::string ErrorHandlerModel::file;

void ErrorHandlerModel::SetMetaData(int line, const char * func, const char * file){
		ErrorHandlerModel::line = to_string(line);
		ErrorHandlerModel::func = string(func);
		ErrorHandlerModel::file = string(file);
}

void ErrorHandlerModel::ErrorHandlerTrigger(string format, ... ){
    EXPECT_EQ(1, 0);
}

void ErrorHandlerModel::ErrorHandlerUpdate(){}
