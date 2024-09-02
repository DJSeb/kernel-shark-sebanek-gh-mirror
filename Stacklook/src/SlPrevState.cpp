/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    SlPrevState.cpp
 * @brief   This file has definitions of the prev_state get-functions.
*/

// C++
#include <string>

// KernelShark
#include "libkshark.h"

// Plugin
#include "SlPrevState.hpp"

// Global functions

/**
 * @brief Gets the abbreviated name of a prev_state from the info field of a
 * KernelShark entry using the specific format of the entries in KernelShark.
 * 
 * @param entry: `sched/sched_switch` event entry whose prev_state we wish to get
 *  
 * @returns Const C++ string with only one member - the name abbreviation.
 * 
 * @note Returning the string is more useful as the value is used a lot
 * in string concatenations.
 */
const std::string get_switch_prev_state(const kshark_entry* entry) {
    auto info_as_str = std::string(kshark_get_info(entry));
    std::size_t start = info_as_str.find(" ==>");
    auto prev_state = info_as_str.substr(start - 1, 1);
    return prev_state;
}

/**
 * @brief Gets the full name of a prev_state from the info field of a
 * KernelShark entry using the specific format of the entries in kernelshark.
 * 
 * @param entry: `sched/sched_switch` event entry whose prev_state we wish to get
 * 
 * @returns Const C++ string with the full name of the identified prev_state.
 * 
 * @note Process states taken from [here](https://man7.org/linux/man-pages/man5/proc_pid_stat.5.html).
 */
const std::string get_longer_prev_state(const kshark_entry* entry) {    
    const std::string& ps_base = get_switch_prev_state(entry);
    std::string final_string = (LETTER_TO_NAME.count(ps_base[0])) ?
        LETTER_TO_NAME.at(ps_base[0]) : "unknown";
    return {ps_base + " - " + final_string};
}