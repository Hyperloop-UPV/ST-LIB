/*
 * InfoWarning.hpp
 *
 *  Created on: Jun 12, 2024
 *      Author: gonzalo
 */

#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "HALAL/Services/Time/Time.hpp"

class InfoWarning {
private:
    static string description;
    static string line;
    static string func;
    static string file;

public:
    static bool warning_triggered;

    /**
     * @brief Triggers WarningHandler and format the warning message. The format works
     * 	      exactly like printf format.
     *
     * @param format String which will be formated.
     * @param args   Arguments specifying data to print
     */
    static void InfoWarningTrigger(string format, ...);

    /**
     * @brief Get all metadata needed for the warning message, including the line function and file.
     *        The default parameters are not necessary but are there in case the compiler macros
     * stop working because a change of the compiler.
     *
     * @param line Line where the warning occurred
     * @param func Function where the warning occurred
     * @param file File where the warning occurred
     */
    static void SetMetaData(
        int line = __builtin_LINE(),
        const char* func = __builtin_FUNCTION(),
        const char* file = __builtin_FILE()
    );

    /**
     * @brief Transmit the warning message.
     */
    static void InfoWarningUpdate();

    friend class BoundaryInterface;
};

#define WARNING(x, ...)                                                                            \
    do {                                                                                           \
        InfoWarning::SetMetaData(__LINE__, __FUNCTION__, __FILE__);                                \
        InfoWarning::InfoWarningTrigger(x, ##__VA_ARGS__);                                         \
    } while (0)
