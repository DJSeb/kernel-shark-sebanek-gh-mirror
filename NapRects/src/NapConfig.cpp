/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    NapConfig.cpp
 * @brief   This file has definitions of the config window class
 *          for the plugin as well as the config object.
*/

// C
#include <stdint.h>

// C++
#include <limits>
// KernelShark
#include "libkshark.h"
#include "KsPlotTools.hpp"

// Plugin
#include "NapConfig.hpp"

// Configuration object functions

/**
 * @brief Get the configuration object as a read-only reference.
 * Utilizes Meyers singleton creation (static local variable), which
 * also ensures the same address when taking a pointer.
 * 
 * @returns Const reference to the configuration object.
 */
const NapConfig& NapConfig::get_instance() {
    static NapConfig instance;
    return instance;
}

/**
 * @brief Gets the currently set limit of entries in the histogram.
 * 
 * @returns Limit of entries in the histogram.
 */
int32_t NapConfig::get_histo_limit() const
{ return _histo_entries_limit; }

/**
 * @brief Gets a boolean flag whether to draw rectangles for 'naps', i.e.
 * durations between sched_switch and sched_waking.
 * 
 * @returns True if we should draw naps, false otherwise.
 */
bool NapConfig::get_draw_naps() const
{ return _draw_naps; }

// Window
// Static functions

// Member functons

/**
 * @brief Constructor for the configuration window.
*/
NapConfigWindow::NapConfigWindow()
    : QWidget(NapConfig::main_w_ptr),
    _histo_label("Entries on histogram until nap rectangles appear: "),
    _histo_limit(this),
    _close_button("Close", this),
    _apply_button("Apply", this)
{
    setWindowTitle("Naps Plugin Configuration");
    // Set window flags to make header buttons
    setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint
                   | Qt::WindowCloseButtonHint);
    setMaximumHeight(300);

    setup_histo_section();
    setup_nap_rects();
    
    // Connect endstage buttons to actions
    setup_endstage();

    // Create the layout
    setup_layout();
}

/**
 * @brief Loads current configuration values into the configuration
 * window's control elements and inner values.
*/
void NapConfigWindow::load_cfg_values() {
    // Easier coding
    NapConfig& cfg = NapConfigWindow::cfg;

    _histo_limit.setValue(cfg._histo_entries_limit);
    _nap_rects_btn.setChecked(cfg._draw_naps);
}

/**
 * @brief Update the configuration object's values with the values
 * from the configuration window.
*/
void NapConfigWindow::update_cfg() {
    NapConfigWindow::cfg._histo_entries_limit = _histo_limit.value();
    NapConfigWindow::cfg._draw_naps = _nap_rects_btn.isChecked();

    // Display a successful change dialog
    // We'll see if unique ptr is of any use here
    auto succ_dialog = new QMessageBox{QMessageBox::Information,
                "Configuration change success",
                "All configuration changes have been applied.",
                QMessageBox::StandardButton::Ok, this};
    succ_dialog->show();
}

/**
 * @brief Sets up spinbox and explanation label.
 * Spinbox's limit values are also set.
 */
void NapConfigWindow::setup_histo_section() {
    _histo_limit.setMinimum(0);
    _histo_limit.setMaximum(std::numeric_limits<int>::max());
    _histo_limit.setValue(cfg._histo_entries_limit);

    _histo_label.setFixedHeight(32);
    _histo_layout.addWidget(&_histo_label);
    _histo_layout.addStretch();
    _histo_layout.addWidget(&_histo_limit);
}

/**
 * @brief Sets up explanation label and check box for controlling
 * display of nap rectangles.
 */
void NapConfigWindow::setup_nap_rects() {
    _nap_rects_label.setText("Display nap rectangles: ");
    _nap_rects_btn.setChecked(cfg._draw_naps);

    _nap_rects_layout.addWidget(&_nap_rects_label);
    _nap_rects_layout.addStretch();
    _nap_rects_layout.addWidget(&_nap_rects_btn);
}

/**
 * @brief Sets up the Apply and Close buttons by putting
 * them into a layout and assigning actions on pressing them.
 */
void NapConfigWindow::setup_endstage() {
    _endstage_btns_layout.addWidget(&_apply_button);
    _endstage_btns_layout.addWidget(&_close_button);

    connect(&_close_button,	&QPushButton::pressed,
            this, &QWidget::close);
    connect(&_apply_button, &QPushButton::pressed,
            this, [this]() { this->update_cfg(); });
}

/**
 * @brief Sets up the main layout of the configuration dialog.
*/
void NapConfigWindow::setup_layout() {
    // Don't allow resizing
    _layout.setSizeConstraint(QLayout::SetFixedSize);

    // Add all control elements
    _layout.addLayout(&_histo_layout);
    _layout.addLayout(&_nap_rects_layout);

    // Set the layout of the dialog
	setLayout(&_layout);
}