/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    SlDetailedView.hpp
 * @brief   This file declares a class for dialog windows with the full stack trace
 *          to be shown by the Stacklook plugin upon user's double click on a
 *          Stacklook button.
 * 
 * @note    Definitions in `SlDetailedView.cpp`.
*/

#ifndef _SL_DETAILED_VIEW_HPP
#define _SL_DETAILED_VIEW_HPP

// C++
#include <string>

// Qt
#include <QtWidgets>

// KernelShark
#include "KsMainWindow.hpp"

// For friending
class SlDetailedView;

/**
 * @brief This type represents the windows the user can spawn to view
 * the stack trace of an event in full. Every window of this type will be
 * dependent on the main KernelShark main window when it comes to program
 * termination (e.g. via clicking the X button).
 * 
 * It inherits from `QWidget`.
*/
class SlDetailedView : public QWidget {
private: // Qt data members
    ///
    /// @brief Layout for the widget's control elements.
    QVBoxLayout     _layout;

    ///
    /// @brief Group for the radio buttons so they exclude each other.
    QButtonGroup    _radio_btns;
    
    ///
    /// @brief Enables the raw view. Exclusive with `_list_radio`.
    QRadioButton    _raw_radio;
    
    ///
    /// @brief Enables the list view. Exclusive with `_raw_radio`.
    QRadioButton    _list_radio;
    
    ///
    /// @brief Name of the task whose stack trace we are viewing.
    QLabel          _which_task;
    
    /// @brief Information specific to a type of an event.
    /// For example, sched_switch events will show their prev state.
    QLabel          _specific_entry_info;

    ///
    /// @brief For toggling between views.
    QStackedWidget  _stacked_widget;
    
    ///
    /// @brief View if the stack trace where items are in a list.
    QListWidget     _list_view;
    
    ///
    /// @brief Purely textual view if the stack trace.
    QTextEdit       _raw_view;
public: // Qt data members
    ///
    /// @brief Close button for the widget.
    QPushButton     _close_button;
private: // Functions
    void _toggle_view();
public: // Functions
    explicit SlDetailedView(const char* task_name, const char* specific_info,
                            const char* data);
};

#endif