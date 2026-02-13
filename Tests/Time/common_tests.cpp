#include <gtest/gtest.h>
#include "ErrorHandler/ErrorHandler.hpp"

std::string ErrorHandlerModel::line;
std::string ErrorHandlerModel::func;
std::string ErrorHandlerModel::file;

namespace ST_LIB::TestErrorHandler {
bool fail_on_error = true;
int call_count = 0;

void reset() {
    fail_on_error = true;
    call_count = 0;
}

void set_fail_on_error(bool enabled) {
    fail_on_error = enabled;
}
} // namespace ST_LIB::TestErrorHandler

void ErrorHandlerModel::SetMetaData(int line, const char * func, const char * file){
		ErrorHandlerModel::line = to_string(line);
		ErrorHandlerModel::func = string(func);
		ErrorHandlerModel::file = string(file);
}

void ErrorHandlerModel::ErrorHandlerTrigger(string format, ... ){
    (void)format;
    ST_LIB::TestErrorHandler::call_count++;
    if(ST_LIB::TestErrorHandler::fail_on_error){
        EXPECT_EQ(1, 0);
    }
}

void ErrorHandlerModel::ErrorHandlerUpdate(){}
