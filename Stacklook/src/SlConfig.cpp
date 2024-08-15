/** TODO: Copyright? **/

/**
 * @file    SlConfig.cpp
 * @brief   This file has definitions of the config window class
 *          for the plugin.
*/

// C++
#include <stdint.h>

// KernelShark
#include "KsPlotTools.hpp"
#include "libkshark.h"

// Plugin
#include "SlConfig.hpp"

// Configuration

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
uint16_t SlConfig::get_stack_offset(event_name_t evt_name) const {
    return (_events_meta.count(evt_name) == 0) ?
        0 : _events_meta.at(evt_name).second;
}

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

/**
 * @brief 
 * @return 
 */
//const event_depths_t SlConfig::get_events_meta() const {
  //  return _events_meta;
//}

/**
 * @brief 
 * @param entry 
 * @return 
 */
bool SlConfig::is_event_allowed(kshark_entry* entry) const {
    const std::string evt_name{kshark_get_event_name(entry)};
    return (_events_meta.count(evt_name) == 0) ?
        false : _events_meta.at(evt_name).first;
}

// Window

SlConfig& SlConfigWindow::cfg{SlConfig::get_instance()};

SlConfigWindow::SlConfigWindow()
    : QWidget(SlConfig::main_w_ptr),
    _close_button("Close", this)
{
    setWindowTitle("Stacklook Plugin Configuration");
    // Set window flags to make header buttons
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint
                   | Qt::WindowMaximizeButtonHint
                   | Qt::WindowCloseButtonHint);
    // Change size to something reasonable
    resize(600, 450);

    _layout.addWidget(&_close_button);

    connect(&_close_button,	&QPushButton::pressed, this, &QWidget::close);

    // Set the layout to the prepared one
	setLayout(&_layout);

}

void SlConfigWindow::update_controls() {}

void SlConfigWindow::upshow() {
    update_controls();
    show();
}

