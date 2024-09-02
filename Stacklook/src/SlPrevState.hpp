/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    SlPrevState.hpp
 * @brief   This file has declarations of the prev_state get-functions.
 *          and also a global constant map of abbreviated prev_state names
 *          to their full names.
*/

#ifndef _SL_PREV_STATE_HPP
#define _SL_PREV_STATE_HPP

// C++
#include <string>
#include <map>

// KernelShark
#include "libkshark.h"

// Static variables

/**
 * @brief Map of abbreviations of prev_states to their full names.
 */
static const std::map<char, const char*> LETTER_TO_NAME {{
    {'S', "sleeping"},
    {'D', "uninterruptible (disk) sleep"},
    {'R', "running"},
    {'I', "idle"},
    {'T', "stopped"},
    {'t', "tracing stop"},
    {'X', "dead"},
    {'Z', "zombie"},
    {'P', "parked"}
}};

// Global functions
const std::string get_switch_prev_state(const kshark_entry* entry);
const std::string get_longer_prev_state(const kshark_entry* entry);

#endif