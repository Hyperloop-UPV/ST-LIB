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
		ErrorHandlerModel::line = to_string(line);
		ErrorHandlerModel::func = string(func);
		ErrorHandlerModel::file = string(file);
}

void ErrorHandlerModel::ErrorHandlerTrigger(string format, ... ){
	if (ErrorHandlerModel::error_triggered) {
		return;
	}

	ErrorHandlerModel::error_triggered = 1.0;
	ErrorHandlerModel::error_to_communicate = true; //This flag is marked so the ProtectionManager can know if it already consumed the error in question.

	if (format.length() != 0) {
		description = "";
	}

	va_list arguments;
	va_start(arguments, format);
	va_list arg_copy;
	va_copy(arg_copy, arguments);

	const int32_t size = vsnprintf(nullptr, 0, format.c_str(), arguments) + 1;
	const unique_ptr<char[]> buffer = make_unique<char[]>(size);
	va_end(arguments);

	vsnprintf(buffer.get(), size, format.c_str(), arg_copy);
	va_end(arg_copy);

	description += string(buffer.get(), buffer.get() + size - 1) + " | Line: " + ErrorHandlerModel::line
							+ " Function: '" + ErrorHandlerModel::func + "' File: " + ErrorHandlerModel::file ;
	
	#ifdef SIM_ON
	LOG_FATAL(string(buffer.get(), buffer.get() + size - 1));
	#endif

#ifdef HAL_TIM_MODULE_ENABLED
	 description += " | TimeStamp: " + to_string(Time::get_global_tick());
#endif

}

void ErrorHandlerModel::ErrorHandlerUpdate(){
	if (!ErrorHandlerModel::error_triggered) {
		return;
	}

#ifdef HAL_UART_MODULE_ENABLED
	if (!UART::printf_ready) {
		return;
	}

	//printf("Error: %s%s", ErrorHandlerModel::description.c_str(), endl);
#endif

}

