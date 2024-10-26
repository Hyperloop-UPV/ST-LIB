/*
 * InfoWarning.cpp
 *
 *  Created on: Jun 12, 2024
 *      Author: gonzalo
 */

#include "HALALMock/Services/InfoWarning/InfoWarning.hpp"

string InfoWarning::description = "Error-No-Description-Found";
string InfoWarning::line = "Error-No-Line-Found";
string InfoWarning::func = "Error-No-Func-Found";
string InfoWarning::file = "Error-No-File-Found";
bool InfoWarning::warning_triggered = false;

void InfoWarning::SetMetaData(int line, const char * func, const char * file){
		InfoWarning::line = to_string(line);
		InfoWarning::func = string(func);
		InfoWarning::file = string(file);
}

void InfoWarning::InfoWarningTrigger(string format, ... ){
	if (InfoWarning::warning_triggered) {
		return;
	}

	InfoWarning::warning_triggered = true;

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

	description += string(buffer.get(), buffer.get() + size - 1) + " | Line: " + InfoWarning::line
							+ " Function: '" + InfoWarning::func + "' File: " + InfoWarning::file ;

#ifdef HAL_TIM_MODULE_ENABLED
	 description += " | TimeStamp: " + to_string(Time::get_global_tick());
#endif

}

void InfoWarning::InfoWarningUpdate(){
	if (!InfoWarning::warning_triggered) {
		return;
	}
	printf("Warning: %s%s", InfoWarning::description.c_str(), endl);

}

