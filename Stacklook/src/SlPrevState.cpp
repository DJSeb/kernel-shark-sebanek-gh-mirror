/** TODO: Copyright? **/

/**
 * @file    SlPrevState.cpp
 * @brief   
*/

// C++
#include <map>
#include <string>

// KernelShark
#include "libkshark.h"

// Plugin
#include "SlPrevState.hpp"

// TODO:

// Global functions
/**
 * @brief 
 * 
 * @param entry
 * 
 * @returns 
*/
const std::string get_switch_prev_state(const kshark_entry* entry) {
    auto info_as_str = std::string(kshark_get_info(entry));
    auto start = info_as_str.find(" ==>");
    auto prev_state = info_as_str.substr(start - 1, 1);
    return prev_state;
}

/**
 * @brief 
 * @param entry 
 * @returns
 * 
 * @note Process states taken from [here](https://man7.org/linux/man-pages/man5/proc_pid_stat.5.html).
 */
const std::string get_longer_prev_state(const kshark_entry* entry) {
    static const std::map<char, const char*> LETTER_TO_NAME {{
        {'S', "sleeping"}, {'D', "uninterruptible (disk) sleep"},
        {'R', "running"}, {'I', "idle"}, {'T', "stopped"},
        {'t', "tracing stop"}, {'X', "dead"}, {'Z', "zombie"},
        {'P', "parked"}
    }};
    
    auto ps_base = get_switch_prev_state(entry);
    std::string final_string = (LETTER_TO_NAME.count(ps_base[0])) ?
        LETTER_TO_NAME.at(ps_base[0]) : "unknown";
    return {ps_base + " - " + final_string};
}