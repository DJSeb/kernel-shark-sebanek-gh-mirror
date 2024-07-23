/** TODO: */

#include <string>
#include <iostream>

#include "SlDetailedWindow.hpp"

/** @brief
 * 
*/
static QString prettify_data(char* data) {
    std::string empty_str = "";
    std::string base_string{data};

    // Cut off '<stack trace >' text
    int first_newline = base_string.find_first_of('\n');
    // Mark what's the top (just to make it clearer)
    std::string new_string = "(top)" + base_string.substr(first_newline);

    // Return new QString
    return QString(new_string.c_str());
}

void SlDetailedView::add_data_to_list_widget(QListWidget& list_widget, const QString& data) {
    QStringList list_data = data.split('\n');
    list_widget.addItems(list_data);
}

SlDetailedView::SlDetailedView(char* label, char* data, QWidget* parent)
: QWidget(parent),
    _radio_btns(this),
    _raw_radio("Raw view", this),
    _list_radio("List view", this),
    _which_event("Kernlestack for task '" + QString(label) + "':", this),
    _stacked_widget(this),
    _list_view(this),
    _raw_view(this),
    _close_button("Close", this) {

    QString new_data = prettify_data(data);

    setWindowTitle("Stacklook - Detailed Stack View");
    // Set window flags to make it independent with header buttons
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint
                   | Qt::WindowMaximizeButtonHint
                   | Qt::WindowCloseButtonHint);
    resize(900, 450);

    _radio_btns.addButton(&_raw_radio);
    _radio_btns.addButton(&_list_radio);
    _raw_radio.setChecked(true);

    _raw_view.setReadOnly(true);
    _raw_view.setAcceptRichText(true);
    _raw_view.setText(QString(new_data));

    _stacked_widget.addWidget(&_raw_view);
    _stacked_widget.addWidget(&_list_view);

    add_data_to_list_widget(_list_view, new_data);

    _layout.addWidget(&_which_event);
    _layout.addWidget(&_raw_radio);
    _layout.addWidget(&_list_radio);

    _layout.addWidget(&_stacked_widget);
    _layout.addWidget(&_close_button);

    // Connections
    connect(&_raw_radio, &QRadioButton::toggled, this, &SlDetailedView::toggle_view);
    connect(&_list_radio, &QRadioButton::toggled, this, &SlDetailedView::toggle_view);

    connect(&_close_button,	&QPushButton::pressed, this, &QWidget::close);

	setLayout(&_layout);

    toggle_view();
}

void SlDetailedView::toggle_view() {
    if (_raw_radio.isChecked()) {
        _stacked_widget.setCurrentWidget(&_raw_view);
    } else if (_list_radio.isChecked()) {
        _stacked_widget.setCurrentWidget(&_list_view);
    }
}