// DO NOT SHIP WITH THIS!!!
#ifndef _UTILITIES_HPP_
#define _UTILITIES_HPP_

#include <cstdint>
#include <string>
#include <iostream>

/** @brief
 * A basic counter for the logs, incremented on each log output.
 */
static uint32_t log_counter = 0;

/**
 * @brief Logging function for simple strings.
 * 
 * @param to_log: string to write on std::cout
 * @param with_newline: whether to end the log with a newline
*/
void log(const std::string& to_log, bool with_newline = true) {
    std::cout << "  [LOG #" << log_counter << "] " << to_log;
    if (with_newline)
        std::cout << std::endl;
    ++log_counter;
}

/**
 * @brief Logging function for simple strings, which supports
 * other value inputs as well.
 * 
 * @param to_log: string to write on std::cout
 * @param other: any type that supports being written into a
 * std::ostream
 * @param with_newline: whether to end the log with a newline
*/
template<typename OtherInfoT>
void log(const std::string& to_log, OtherInfoT other, bool with_newline = true) {
    std::cout << "  [LOG #" << log_counter << "] " << to_log;
    std::cout << other;
    if (with_newline)
        std::cout << std::endl;
    ++log_counter;
}

#endif