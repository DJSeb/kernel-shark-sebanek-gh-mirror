/** TODO: Copyright? **/

/**
 * @file    SlConfig.cpp
 * @brief   This file has definitions of the config window class
 *          for the plugin as well as the config object.
*/

// C++
#include <stdint.h>
#include <limits>

// KernelShark
#include "KsPlotTools.hpp"
#include "libkshark.h"

// Plugin
#include "SlConfig.hpp"


// Static functions

static QFrame* _get_hline(QWidget* parent) {
    auto line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

static void _change_label_bg_color(QLabel* to_change,
                                   const QColor* new_col) {
    to_change->setStyleSheet(
        QString("background-color: %1").arg(new_col->name())
    );
}

static void _setup_colorchange(QWidget* holder,
                               const KsPlot::Color& curr_col,
                               QColor* changeling,
                               QPushButton* push_btn,
                               QLabel* preview,
                               QHBoxLayout* layout) {
    changeling->setRgb((int)(curr_col.r()),
                       (int)(curr_col.g()),
                       (int)(curr_col.b()));
    
    _change_label_bg_color(preview, changeling);

    preview->setFixedHeight(32);
    preview->setFixedWidth(32);
    preview->setFrameShape(QFrame::Panel);
    preview->setFrameShadow(QFrame::Sunken);
    preview->setLineWidth(2);

    layout->addWidget(push_btn);
    layout->addSpacing(100);
    layout->addWidget(preview);

    holder->connect(
        push_btn,
        &QPushButton::pressed,
        holder,
        [preview, changeling]() {
            QColor picked_color = QColorDialog::getColor();
            if (picked_color.isValid()) {
                *changeling = picked_color;
                _change_label_bg_color(preview, &picked_color);
            }
        }
    );
}

// Configuration

SlConfig& SlConfig::get_instance() {
    static SlConfig instance;
    return instance;
}

/**
 * @brief
 * 
 * @returns
*/
int32_t SlConfig::get_histo_limit() const
{ return _histo_entries_limit; }

/**
 * @brief
 * 
 * @returns
*/
uint16_t SlConfig::get_stack_offset(event_name_t evt_name) const {
    return (_events_meta.count(evt_name) == 0) ?
        0 : _events_meta.at(evt_name).second;
}

/**
 * @brief
 * 
 * @returns
*/
const KsPlot::Color SlConfig::get_default_btn_col() const
{ return _default_btn_col; }

/**
 * @brief
 * 
 * @returns
*/
const KsPlot::Color SlConfig::get_button_outline_col() const
{ return _button_outline_col; }

/**
 * @brief 
 * @return 
 */
const events_meta_t& SlConfig::get_events_meta() const {
    return _events_meta;
}

/**
 * @brief 
 * @param entry 
 * 
 * @return 
 */
bool SlConfig::is_event_allowed(kshark_entry* entry) const {
    const std::string evt_name{kshark_get_event_name(entry)};
    return (_events_meta.count(evt_name) == 0) ?
        false : _events_meta.at(evt_name).first;
}

// Window

SlConfig& SlConfigWindow::cfg{SlConfig::get_instance()};

SlConfigWindow::SlConfigWindow()
    : QWidget(SlConfig::main_w_ptr),
    _def_btn_col_btn("Choose default button color", this),
    _def_btn_col_preview(this),
    _btn_outline_btn("Choose button outline color", this),
    _btn_outline_preview(this),
    _histo_label("Entries on histogram until Stacklook buttons appear: "),
    _histo_limit(this),
    _close_button("Close", this),
    _apply_button("Apply", this)
{
    SlConfig& cfg = SlConfigWindow::cfg;

    setWindowTitle("Stacklook Plugin Configuration");
    // Set window flags to make header buttons
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint
                   | Qt::WindowMaximizeButtonHint
                   | Qt::WindowCloseButtonHint);
    // Change size to something reasonable


    // Setup colors
    const KsPlot::Color curr_def_btn_col = cfg._default_btn_col;
    const KsPlot::Color curr_btn_outline = cfg._button_outline_col;
    
    _setup_colorchange(this, curr_def_btn_col,
                       &_def_btn_col, &_def_btn_col_btn,
                       &_def_btn_col_preview,
                       &_def_btn_col_ctl_layout);
    _setup_colorchange(this, curr_btn_outline,
                       &_btn_outline, &_btn_outline_btn,
                       &_btn_outline_preview,
                       &_btn_outline_ctl_layout);
    
    setup_histo_section();

    // Connect endstage buttons to actions
    connect(&_close_button,	&QPushButton::pressed,
            this, &QWidget::close);
    connect(&_apply_button, &QPushButton::pressed,
            this, [this]() { this->update_controls(); });


    // Events meta
    setup_events_meta_widget();

    _endstage_btns_layout.addWidget(&_apply_button);
    _endstage_btns_layout.addWidget(&_close_button);

    // Create the layout
    _layout.addLayout(&_histo_layout);
    _layout.addLayout(&_def_btn_col_ctl_layout);
    _layout.addLayout(&_btn_outline_ctl_layout);
    _layout.addWidget(_get_hline(this));
    _layout.addLayout(&_events_meta_layout);
    _layout.addWidget(_get_hline(this));
    _layout.addLayout(&_endstage_btns_layout);

    // Set the layout to the prepared
	setLayout(&_layout);
}

void SlConfigWindow::update_controls() {
    bool event_meta_success = true;
    int r, g, b;
    _def_btn_col.getRgb(&r, &g, &b);
    SlConfigWindow::cfg._default_btn_col =
        {(uint8_t)r, (uint8_t)g, (uint8_t)b};

    _btn_outline.getRgb(&r, &g, &b);
    SlConfigWindow::cfg._button_outline_col = 
        {(uint8_t)r, (uint8_t)g, (uint8_t)b};

    SlConfigWindow::cfg._histo_entries_limit = _histo_limit.value();

    for (auto&& event_entry : _events_meta_layout.children()) {
        QHBoxLayout* typed_event_entry = (QHBoxLayout*)(event_entry);
        auto event_name = typed_event_entry->findChild<QLabel*>("evt_name");
        auto event_allowed = typed_event_entry->findChild<QCheckBox*>("evt_allowed");
        auto event_depth = typed_event_entry->findChild<QSpinBox*>("evt_depth");

        if (event_name != nullptr && event_allowed != nullptr && event_depth != nullptr) {
            std::string event_name_str = event_name->text().toStdString();
            event_meta_t& event_meta = SlConfigWindow::cfg._events_meta.at(event_name_str);
            event_meta.first = event_allowed->isChecked();
            event_meta.second = (uint16_t)event_depth->value();
        } else {
            event_meta_success = false;
        }      
    }

    if (!event_meta_success) {
        auto fail_dialog = new QMessageBox(QMessageBox::Information,
                    "Configuration change failed",
                    "Configuration changes to specific events could not be altered!",
                    QMessageBox::StandardButton::Ok, this);
        fail_dialog->show();
    } else {
        auto confirm_dialog = new QMessageBox(QMessageBox::Information,
                    "Configuration change",
                    "Configuration was successfully altered!",
                    QMessageBox::StandardButton::Ok, this);
        confirm_dialog->show();
    }
}

void SlConfigWindow::setup_histo_section() {
    _histo_limit.setMinimum(0);
    _histo_limit.setMaximum(std::numeric_limits<int>::max());
    _histo_limit.setValue(cfg._histo_entries_limit);

    _histo_label.setFixedHeight(32);
    _histo_layout.addWidget(&_histo_label);
    _histo_layout.addSpacing(100);
    _histo_layout.addWidget(&_histo_limit);
}

void SlConfigWindow::setup_events_meta_widget() {
    QHBoxLayout* header_row = new QHBoxLayout{nullptr};
    QLabel* header_evt_name = new QLabel{this};
    header_evt_name->setText("Event name");
    QLabel* header_evt_allowed = new QLabel{this};
    header_evt_allowed->setText("Allowed");
    QLabel* header_evt_depth = new QLabel{this};
    header_evt_depth->setText("Preview stack offset");
    
    header_row->addWidget(header_evt_name);
    header_row->addSpacing(50);
    header_row->addWidget(header_evt_allowed);
    header_row->addSpacing(16);
    header_row->addWidget(header_evt_depth);

    _events_meta_layout.addLayout(header_row);

    const events_meta_t& evts_meta = SlConfigWindow::cfg.get_events_meta();

    for (auto it = evts_meta.cbegin(); it != evts_meta.cend(); ++it) {
        QHBoxLayout* row = new QHBoxLayout{nullptr};
        QLabel* evt_name = new QLabel{this};
        QCheckBox* evt_allowed = new QCheckBox{this};
        QSpinBox* evt_depth = new QSpinBox{this};

        evt_name->setText(it->first.c_str());
        evt_allowed->setChecked(it->second.first);
        evt_depth->setValue(it->second.second);
        evt_depth->setMinimum(0);

        row->addWidget(evt_name);
        row->addSpacing(50);
        row->addWidget(evt_allowed);
        row->addSpacing(16);
        row->addWidget(evt_depth);

        _events_meta_layout.addLayout(row);
    }
}

void SlConfigWindow::load_cfg_values() {
    SlConfig& cfg = SlConfigWindow::cfg;
    _histo_limit.setValue(cfg._histo_entries_limit);
    _def_btn_col.setRgb(cfg._default_btn_col.r(),
                        cfg._default_btn_col.g(),
                        cfg._default_btn_col.b());
    _btn_outline.setRgb(cfg._button_outline_col.r(),
                        cfg._button_outline_col.g(),
                        cfg._button_outline_col.b());
    _change_label_bg_color(&_def_btn_col_preview,
                           &_def_btn_col);
    _change_label_bg_color(&_btn_outline_preview,
                           &_btn_outline);
}