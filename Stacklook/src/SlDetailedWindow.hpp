#ifndef _SL_DETAILED_WINDOW_HPP
#define _SL_DETAILED_WINDOW_HPP

// C++
#include <string>
#include <memory>

// Qt
#include <QtWidgets>

class SlDetailedView : public QTextEdit {
    Q_OBJECT
public:
    explicit SlDetailedView(const std::string& data,
                            QWidget* parent = nullptr);
private:
    explicit SlDetailedView(QWidget* parent = nullptr);
    std::shared_ptr<const std::string> event_info;
    std::shared_ptr<const std::string> stack_str;
// Qt stuff
private:
    QVBoxLayout  _layout;
    QRadioButton _choose_view;
    QLabel       _which_event;
    QStringList  _list_view;
    QTextEdit    _raw_view;
public:
    QPushButton _close_button;
};

#endif