/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    SlDetailedView.cpp
 * @brief   This file defines the class functionaliteis of the dialog windows
 *          shown by the Stacklook plugin. Includes some string manipulations
 *          of stack trace data as well.
*/

// C++
#include <string>
#include <map>

// Plugin headers
#include "SlDetailedView.hpp"
#include "SlConfig.hpp"

// Static functions

/**
 * @brief Replaces the top of the stack trace's text with an indicator that
 * the top is there.
 * 
 * @param data: stack trace from trace-cmd in its textual form
 * 
 * @returns New QString with prettier text data.
*/
static QString _prettify_data(const char* data) {
    std::string base_string{data};

    // What we got is NOT a stack trace, but we'll display it anyway.
    // This is for error messages and the like
    if (base_string.find("<stack trace >") == std::string::npos) {
        return QString(data);
    }

    // Cut off '<stack trace >' text
    int first_newline = base_string.find_first_of('\n');
    // Mark what's the top (just to make it clearer)
    std::string new_string = "(top)" + base_string.substr(first_newline);

    // Space for any other possible data prettifications here...

    // Return new QString (needs a C string to be constructed)
    return QString(new_string.c_str());
}

// Class functions

/**
 * @brief Constructor for Stacklook's detailed stack trace view window.
 * 
 * @param task_name: name of the task whose stack trace is viewed
 * @param specific_info: specific info of a task
 * @param data: stack trace as text
 * 
 * @note It is dependent on the configuration 'SlConfig' singleton.
*/
SlDetailedView::SlDetailedView(const char* task_name, const char* specific_info,
                               const char* data)
  : QWidget(SlConfig::main_w_ptr), // Configuration access here
    _radio_btns(this),
    _raw_radio("Raw view", this),
    _list_radio("List view", this),
    _which_task("Kernel stack for task '" + QString(task_name) + "':", this),
    _specific_entry_info(specific_info, this),
    _stacked_widget(this),
    _list_view(this),
    _raw_view(this),
    _close_button("Close", this)
{
    // Delete on close
    setAttribute(Qt::WA_DeleteOnClose);

    // Make the data a bit nicer
    QString new_data = _prettify_data(data);

    setWindowTitle("Stacklook - Detailed Stack View");
    // Set window flags to make header buttons
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint
                   | Qt::WindowMaximizeButtonHint
                   | Qt::WindowCloseButtonHint);

    // Change size to something reasonable
    resize(900, 450);

    // Add control elements and set their defaults
    _radio_btns.addButton(&_raw_radio);
    _radio_btns.addButton(&_list_radio);
    _raw_radio.setChecked(true);

    _raw_view.setReadOnly(true);
    _raw_view.setAcceptRichText(true);
    _raw_view.setText(QString(new_data));

    _stacked_widget.addWidget(&_raw_view);
    _stacked_widget.addWidget(&_list_view);

    // Add stack trace to the list view as well
    _list_view.addItems(new_data.split('\n'));

    _layout.addWidget(&_which_task);
    _layout.addWidget(&_specific_entry_info);
    _layout.addWidget(&_raw_radio);
    _layout.addWidget(&_list_radio);

    _layout.addWidget(&_stacked_widget);
    _layout.addWidget(&_close_button);

    // Connections
    connect(&_raw_radio, &QRadioButton::toggled, this, &SlDetailedView::_toggle_view);
    connect(&_list_radio, &QRadioButton::toggled, this, &SlDetailedView::_toggle_view);

    connect(&_close_button,	&QPushButton::pressed, this, &QWidget::close);

    // Set the layout to the prepared one
	setLayout(&_layout);

    // Start with a view
    _toggle_view();
}

/**
 * @brief Toggles which view is currently active in the widget based on
 * the radio buttons' checked states.
*/
void SlDetailedView::_toggle_view() {
    if (_raw_radio.isChecked()) {
        _stacked_widget.setCurrentWidget(&_raw_view);
    } else if (_list_radio.isChecked()) {
        _stacked_widget.setCurrentWidget(&_list_view);
    }
}