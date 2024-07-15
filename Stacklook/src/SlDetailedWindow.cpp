/** TODO: */

#include <string>
#include <iostream>

#include "SlDetailedWindow.hpp"

void SlDetailedView::add_data_to_list_view(QListView& list_view, const QString& data) {
    QStringList list_data = data.split('\n');
    list_view.setModel(new QStringListModel{list_data, this});
}

SlDetailedView::SlDetailedView(const QString& data, QWidget* parent)
: QWidget(parent),
    _radio_btns(this),
    _raw_radio("Raw view", this),
    _list_radio("List view", this),
    _which_event("Some event", this),
    _stacked_widget(this),
    _raw_view(data, this),
    _list_view(this),
    _close_button("Close", this) {

    setWindowTitle("Stacklook - Detailed Stack View");
    // Set window flags to make it independent with header buttons
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint
                   | Qt::WindowMaximizeButtonHint
                   | Qt::WindowCloseButtonHint);
    resize(900, 450);

    _radio_btns.addButton(&_raw_radio);
    _radio_btns.addButton(&_list_radio);
    _raw_radio.setChecked(true);

    _stacked_widget.addWidget(&_raw_view);
    _stacked_widget.addWidget(&_list_view);

    add_data_to_list_view(_list_view, data);

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