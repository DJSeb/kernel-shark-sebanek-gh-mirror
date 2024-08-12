/** TODO: Copyright? **/

/**
 * @file    SlPrevState.hpp
 * @brief   
*/

#ifndef _SL_PREV_STATE_HPP
#define _SL_PREV_STATE_HPP

// C++
#include <string>

// KernelShark
#include "libkshark.h"

// Global functions
const std::string get_switch_prev_state(const kshark_entry* entry);
const std::string get_longer_prev_state(const kshark_entry* entry);

#endif