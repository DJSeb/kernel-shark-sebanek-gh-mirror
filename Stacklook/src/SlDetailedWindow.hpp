#ifndef _SL_DETAILED_WINDOW_HPP
#define _SL_DETAILED_WINDOW_HPP

// C++
#include <string>
#include <memory>

// Qt
#include <QtWidgets>

class SlDetailedView : public QWidget {
public:
    explicit SlDetailedView(char* label,
                            char* data,
                            QWidget* = nullptr);
private:
    std::shared_ptr<const std::string> event_info;
    std::shared_ptr<const std::string> stack_str;
// Qt stuff
private:
    QVBoxLayout     _layout;
    QButtonGroup    _radio_btns;
    QRadioButton    _raw_radio;
    QRadioButton    _list_radio;
    QLabel          _which_event;
    QStackedWidget  _stacked_widget;
    QListWidget     _list_view;
    QTextEdit       _raw_view;
public:
    QPushButton _close_button;
private:
    explicit SlDetailedView(QWidget* parent = nullptr);
    void add_data_to_list_widget(QListWidget& list_widget, const QString& data);
    void toggle_view();
};

#endif