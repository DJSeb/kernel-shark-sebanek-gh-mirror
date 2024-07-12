/** TODO: */

#include <string>

#include "SlDetailedWindow.hpp"

static int add_data_to_list_view(QStringList& list_view, const QString& data) {
    return 0;
}

SlDetailedView::SlDetailedView(const QString& data, QWidget* parent)
: QWidget(parent),
  _raw_view(data),
  _close_button("Close", this) {

    resize(900, 450);

    add_data_to_list_view(_list_view, data);

    _layout.addWidget(&_which_event);
    _layout.addWidget(&_choose_view);
    // Somehow make the above change the visibilities of the below.
    //_layout.addWidget(&_list_view);
    _layout.addWidget(&_raw_view);
    _layout.addWidget(&_close_button);

    connect(&_close_button,	&QPushButton::pressed,
            this, &QWidget::close);

	this->setLayout(&_layout);
}