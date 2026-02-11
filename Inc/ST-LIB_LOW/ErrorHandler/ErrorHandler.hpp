/*
 * ErrorHandler.hpp
 *
 *  Created on: Dec 22, 2022
 *      Author: Pablo
 */

#pragma once

#include "C++Utilities/CppUtils.hpp"

#ifndef SIM_ON
#include "HALAL/Services/Time/Time.hpp"
#include "HALAL/Services/Communication/UART/UART.hpp"
#endif // !defined(SIM_ON)

class ErrorHandlerModel {
private:
	static string description;
	static string line;
	static string func;
	static string file;

public:
	static double error_triggered;
	static bool error_to_communicate;

	 /**
	 * @brief Triggers ErrorHandler and format the error message. The format works
	 * 	      exactly like printf format.
	 *
	 * @param format String which will be formated.
	 * @param args   Arguments specifying data to print
	 * @return uint8_t Id of the service.
	 */
	static void ErrorHandlerTrigger(string format, ...);

	 /**
	 * @brief Get all metadata needed for the error message, including the line function and file.
	 *        The default parameters are not necessary but are there in case the compiler macros stop
	 *        working because a change of the compiler.
	 *
	 * @param line Line where the error occurred
	 * @param func Function where the error occurred
	 * @param file File where the file occurred
	 * @return uint8_t Id of the service.
	 */
	static void SetMetaData( int line = __builtin_LINE(), const  char * func = __builtin_FUNCTION(), const char * file = __builtin_FILE());

	/**
	 * @brief Transmit the error message.
	 */
	static void ErrorHandlerUpdate();

	friend class BoundaryInterface;

};

#define ErrorHandler(x, ...) do { ErrorHandlerModel::SetMetaData(__LINE__, __FUNCTION__, __FILE__); \
					           	   ErrorHandlerModel::ErrorHandlerTrigger(x, ##__VA_ARGS__);}while(0)
