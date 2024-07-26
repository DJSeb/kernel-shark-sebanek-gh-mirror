#ifndef _SL_DETAILED_WINDOW_HPP
#define _SL_DETAILED_WINDOW_HPP

// C++
#include <string>
#include <memory>

// Qt
#include <QtWidgets>

// KernelShark
#include "libkshark.h"
#include "KsMainWindow.hpp"

class SlDetailedView : public QWidget {
public: // Data memebers
    /** Pointer to a container for opened Stacklook windows. **/
    inline static std::vector<SlDetailedView*>* opened_views = nullptr;
    /** Pointer to the main window used for window hierarchy. **/
    inline static KsMainWindow* main_w_ptr = nullptr;
public: // Functions
    static SlDetailedView* make_new_view(char* label, char* data);
private: // Data members
    std::shared_ptr<const std::string> event_info;
    std::shared_ptr<const std::string> stack_str;
private: // Qt data members
    QVBoxLayout     _layout;
    QButtonGroup    _radio_btns;
    QRadioButton    _raw_radio;
    QRadioButton    _list_radio;
    QLabel          _which_event;
    QStackedWidget  _stacked_widget;
    QListWidget     _list_view;
    QTextEdit       _raw_view;
public: // Qt data members
    QPushButton _close_button;
private: // Functions
    explicit SlDetailedView(QWidget* parent = nullptr);
    explicit SlDetailedView(char* label, char* data);
    void add_data_to_list_widget(QListWidget& list_widget, const QString& data);
    void toggle_view();
};

#endif