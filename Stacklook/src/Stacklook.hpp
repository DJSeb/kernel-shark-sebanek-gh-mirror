/** TODO:
 * For global variables of internal logic.
*/

#ifndef _STACKLOOK_HPP
#define _STACKLOOK_HPP

// C++
#include <vector>
#include <string>
#include <iostream>

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"
#include "libkshark-plot.h"
#include "KsPlugins.hpp"
#include "KsMainWindow.hpp"
#include "KsPlotTools.hpp"

// Plugin headers
#include "stacklook.h"
#include "SlDetailedWindow.hpp"

/**
 * @brief Pointer to the main window to allow manipulation and
 * window dependency (i.e. closing the main window closes the others).
*/
static KsMainWindow* main_w_ptr;

/**
 * @brief Container for opened Stacklook windows. Allows more independent lifetime
 * and is easier to control their destruction.
*/
static std::vector<SlDetailedView*> opened_views{};

#endif