/** TODO: Copyright? **/

/**
 * @file    SlConfig.cpp
 * @brief   This file has definitions of the config window class
 *          for the plugin.
*/

#include "SlConfig.hpp"

// Classes

SlConfig& SlConfig::get_instance() {
    static SlConfig instance;
    return instance;
}

/**
 * @brief
 * 
 * @returns
*/
int32_t SlConfig::get_histo_limit() const
{ return _histo_entries_limit; }

/**
 * @brief
 * 
 * @returns
*/
int16_t SlConfig::get_stack_offset() const
{ return _stack_offset; }

/**
 * @brief
 * 
 * @returns
*/
const KsPlot::Color SlConfig::get_default_btn_col() const
{ return _default_btn_col; }

/**
 * @brief
 * 
 * @returns
*/
const KsPlot::Color SlConfig::get_default_outline_col() const
{ return _default_outline_col; }