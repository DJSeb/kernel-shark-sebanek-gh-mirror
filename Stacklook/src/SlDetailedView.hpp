#ifndef _SL_DETAILED_VIEW_HPP
#define _SL_DETAILED_VIEW_HPP

// C++
#include <string>

// Qt
#include <QtWidgets>

// KernelShark
#include "KsMainWindow.hpp"

/**
 * @brief This type represents the windows the user can spawn to view
 * the stack trace of an event in full. Every window of this type will be
 * dependent on the main KernelShark main window when it comes to program
 * termination (e.g. via clicking the X button).
*/
class SlDetailedView : public QWidget {
public: // Class data members
    /**
     * @brief Pointer to a vector of opened Stacklook windows.
    */
    inline static std::vector<SlDetailedView*>* opened_views = nullptr;
    /**
     * @brief Pointer to the main window used for window hierarchy. 
    */
    inline static KsMainWindow* main_w_ptr = nullptr;
private: // Qt data members
    /**
     * @brief Layout for the widget's control elements.
    */
    QVBoxLayout     _layout;
    /**
     * @brief Group for the radio buttons so they exclude each other.
    */
    QButtonGroup    _radio_btns;
    /**
     * @brief Enables the raw view. Exclusive with `_list_radio`.
    */
    QRadioButton    _raw_radio;
    /**
     * @brief Enables the list view. Exclusive with `_raw_radio`.
    */
    QRadioButton    _list_radio;
    /**
     * @brief Name of the task whose stack trace we are viewing.
    */
    QLabel          _which_task;
    /**
     * @brief For toggling between views.
    */
    QStackedWidget  _stacked_widget;
    /**
     * @brief View if the stack trace where items are in a list.
    */
    QListWidget     _list_view;
    /**
     * @brief Purely textual view if the stack trace.
    */
    QTextEdit       _raw_view;
public: // Qt data members
    /**
     * @brief Close button for the widget.
    */
    QPushButton     _close_button;
private: // Functions
    explicit SlDetailedView(char* task_name, char* data);
    void _toggle_view();
public: // Functions
    static SlDetailedView* make_new_view(char* task_name, char* data);
};

#endif