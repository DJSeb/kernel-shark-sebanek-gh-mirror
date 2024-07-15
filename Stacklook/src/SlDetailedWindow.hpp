#ifndef _SL_DETAILED_WINDOW_HPP
#define _SL_DETAILED_WINDOW_HPP

// C++
#include <string>
#include <memory>

// Qt
#include <QtWidgets>

class SlDetailedView : public QWidget {
    //Q_OBJECT
public:
    explicit SlDetailedView(const QString& data,
                            QWidget* parent = nullptr);
private:
    explicit SlDetailedView(QWidget* parent = nullptr);
    std::shared_ptr<const std::string> event_info;
    std::shared_ptr<const std::string> stack_str;
    void add_data_to_list_view(QListView& list_view, const QString& data);
// Qt stuff
private:
    QVBoxLayout     _layout;
    QButtonGroup    _radio_btns;
    QRadioButton    _raw_radio;
    QRadioButton    _list_radio;
    QLabel          _which_event;
    QStackedWidget  _stacked_widget;
    QListView       _list_view;
    QTextEdit       _raw_view;
public:
    QPushButton _close_button;
private:
    void toggle_view();
};

#endif