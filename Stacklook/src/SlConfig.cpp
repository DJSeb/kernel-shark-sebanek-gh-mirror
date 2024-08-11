/** TODO: Copyright? **/

/**
 * @file    SlConfig.cpp
 * @brief   This file has definitions of the config window class
 *          for the plugin.
*/

#include "SlConfig.hpp"

SlConfig& SlConfig::get_instance() {
    static SlConfig instance;
    return instance;
}