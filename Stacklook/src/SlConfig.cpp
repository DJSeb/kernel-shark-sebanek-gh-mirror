/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    SlConfig.cpp
 * @brief   This file has definitions of the config window class
 *          for the plugin as well as the config object.
*/

// C
#include <stdint.h>

// KernelShark
#include "KsPlotTools.hpp"
#include "libkshark.h"

// Plugin
#include "SlConfig.hpp"

// Configuration object functions

/**
 * @brief Get the configuration object as a read-only reference.
 * Utilizes Meyers singleton creation (static local variable), which
 * also ensures the same address when taking a pointer.
 * 
 * @returns Const reference to the configuration object.
 */
SlConfig& SlConfig::get_instance() {
    static SlConfig instance;
    return instance;
}

/**
 * @brief Gets the currently set limit of entries in the histogram.
 * 
 * @returns Limit of entries in the histogram.
 */
int32_t SlConfig::get_histo_limit() const
{ return _histo_entries_limit; }

#ifndef _UNMODIFIED_KSHARK // Task colors, stack offset
/**
 * @brief Gets whether the task colors are used for Stacklook
 * buttons.
 * 
 * @return Boolean representing current configuration value. 
 */
bool SlConfig::get_use_task_colors() const
{ return _use_task_colors; }

/**
 * @brief Get offset from the top of the kernel stack used
 * when displaying preview of the kernel stack.
 * 
 * @param evt_name: name of the event, whose value we want
 * 
 * @returns Offset from the top of the kernel stack used
 * when displaying preview of the kernel stack.
 */
depth_t SlConfig::get_stack_offset(event_name_t evt_name) const {
    return (_events_meta.count(evt_name) == 0) ?
        0 : _events_meta.at(evt_name).second;
}
#endif

/**
 * @brief Gets the default color of Stacklook buttons.
 * Uses KernelShark's color type.
 * 
 * @returns Default color of Stacklook buttons. 
 */
const KsPlot::Color SlConfig::get_default_btn_col() const
{ return _default_btn_col; }

/**
 * @brief Gets the outline color of Stacklook buttons.
 * Uses KernelShark's color type.
 * 
 * @returns Outline color of Stacklook buttons. 
 */
const KsPlot::Color SlConfig::get_button_outline_col() const
{ return _button_outline_col; }

/**
 * @brief Gets const reference to the events meta of the configuration
 * object.
 *  
 * @returns Const reference to the events meta.
 */
const events_meta_t& SlConfig::get_events_meta() const
{ return _events_meta; }

/**
 * @brief Returns whether an event is allowed to have a Stacklook)
 * button above itself.
 * 
 * @param entry: entry whose event type we want to check
 * 
 * @returns True if event is allowed, false otherwise.
*/
bool SlConfig::is_event_allowed(const kshark_entry* entry) const {
    const std::string evt_name{kshark_get_event_name(entry)};
    return (_events_meta.count(evt_name) == 0) ?
        false
#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
        : _events_meta.at(evt_name).first;
#else
        : _events_meta.at(evt_name);
#endif
}

// Window
// Static functions

/**
 * @brief Creates a horizontal line to be used in a widget as
 * a dividing element.
 * 
 * @param parent: Qt object which will own the created line 
 * 
 * @returns Pointer to the line object.
 */
static QFrame* _get_hline(QWidget* parent) {
    // Apparently, lines are just special QFrames.

    auto line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

/**
 * @brief Changes the background color of a QLabel to one specifed in
 * the arguments.
 * 
 * @param to_change: label which will be changed
 * @param new_col: color to be used as the background
 */
static void _change_label_bg_color(QLabel* to_change,
                                   const QColor* new_col) {
    to_change->setStyleSheet(
        QString("background-color: %1").arg(new_col->name())
    );
}

/**
 * @brief Sets up the layout for a button to change a color and a
 * preview of the color gotten from the color dialog displayed by
 * pushing the button.
 * 
 * @param parent: owner of the Qt objects
 * @param curr_col: color currently present in the configuration
 * object and shown in the preview
 * @param changeling: color which may be changed by the actions of
 * the user and may be saved into the configuration object
 * @param push_btn: button which will invoke the color dialog for
 * choosing a new color
 * @param preview: label whose background will serve as a preview of
 * the chosen color or the one currently in the configuration object
 * @param layout: layout into which the button and push button will go
 */
static void _setup_colorchange(QWidget* parent,
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
    layout->addStretch();
    layout->addWidget(preview);

    parent->connect(
        push_btn,
        &QPushButton::pressed,
        parent,
        [preview, changeling]() {
            QColor picked_color = QColorDialog::getColor();
            if (picked_color.isValid()) {
                *changeling = picked_color;
                _change_label_bg_color(preview, &picked_color);
            }
        }
    );
}

// Class functions

/**
 * @brief Constructor for the configuration window.
 * 
 * @note It is dependent on the configuration 'SlConfig' singleton.
 */
SlConfigWindow::SlConfigWindow()
    : QWidget(SlConfig::main_w_ptr), // Configuration access here
    _def_btn_col_btn("Choose default button color", this),
    _def_btn_col_preview(this),
    _btn_outline_btn("Choose button outline color", this),
    _btn_outline_preview(this),
    _histo_label("Entries on histogram until Stacklook buttons appear: "),
    _histo_limit(this),
#ifndef _UNMODIFIED_KSHARK // Task colors
    _task_col_label("Use task colors for Stacklook buttons: "),
    _task_col_btn(this),
#endif
    _close_button("Close", this),
    _apply_button("Apply", this)
{
    setWindowTitle("Stacklook Plugin Configuration");
    // Set window flags to make header buttons
    setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint
                   | Qt::WindowCloseButtonHint);
    setMaximumHeight(300);

    setup_histo_section();
#ifndef _UNMODIFIED_KSHARK // Task colors
    setup_use_task_coloring();
#endif
    // Configuration access here
    const SlConfig& cfg = SlConfig::get_instance();

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
    
    // Connect endstage buttons to actions
    setup_endstage();

    // Events meta
    setup_events_meta_widget();

    // Create the layout
    setup_layout();
}

/**
 * @brief Update the configuration object's values with the values
 * from the configuration window.
 * 
 * @note It is dependent on the configuration 'SlConfig' singleton.
 */
void SlConfigWindow::update_cfg() {
    // If changing the events meta was a success
    bool events_meta_change = true;

    // For BOTH color changes
    int r, g, b;
    
    // Configuration access here
    SlConfig& cfg = SlConfig::get_instance();

    _def_btn_col.getRgb(&r, &g, &b);
    cfg._default_btn_col = {(uint8_t)r, (uint8_t)g, (uint8_t)b};

    _btn_outline.getRgb(&r, &g, &b);
    cfg._button_outline_col = {(uint8_t)r, (uint8_t)g, (uint8_t)b};

    cfg._histo_entries_limit = _histo_limit.value();

#ifndef _UNMODIFIED_KSHARK // Task colors
    cfg._use_task_colors = _task_col_btn.isChecked();
#endif

    // Dynamically added members need special handling 
    const int SUPPORTED_EVENTS_COUNT = static_cast<int>(cfg.get_events_meta().size());

    for (int i = 0; i < SUPPORTED_EVENTS_COUNT; ++i) {
        auto index_str = std::to_string(i);
        auto event_name = this->findChild<QLabel*>("evt_name_" + index_str);
        auto event_allowed = this->findChild<QCheckBox*>("evt_allowed_" + index_str);
#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
        auto event_depth = this->findChild<QSpinBox*>("evt_depth_" + index_str);
#endif
        // On successful finds, change values in the configuration object
        if (event_name != nullptr
            && event_allowed != nullptr
#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
            && event_depth != nullptr
#endif
        ) {
            std::string event_name_str = event_name->text().toStdString();
#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
            event_meta_t& event_meta = cfg._events_meta.at(event_name_str);
            event_meta.first = event_allowed->isChecked();
            event_meta.second = (depth_t)event_depth->value();
#else
            allowed_t& is_allowed = cfg._events_meta.at(event_name_str);
            is_allowed = event_allowed->isChecked();
            
#endif
        } else { // Otherwise indicate that there was a failure
            events_meta_change = false;
        }
    }

    // Display a dialog based on the success of the update process
    const char* change_status = events_meta_change ?
        "Configuration change success" :
        "Configuration change fail";
    const char* detailed_message = events_meta_change ?
        "Configuration was successfully altered!" :
        "Configuration alteration wasn't fully successful.\n"
        "Changes to specific events weren't applied.\n"
        "Other configuration changes were successfully changed.";
        
    auto info_dialog = new QMessageBox(QMessageBox::Information,
                change_status, detailed_message,
                QMessageBox::StandardButton::Ok, this);
    info_dialog->show();
}

/**
 * @brief Sets up spinbox and explanation label.
 * Spinbox's limit values are also set. Also creates
 * aesthetic spacing. 
 * 
 * @note It is dependent on the configuration 'SlConfig' singleton.
 */
void SlConfigWindow::setup_histo_section() {
    // Configuration access here
    const SlConfig& cfg = SlConfig::get_instance();

    _histo_limit.setMinimum(0);
    _histo_limit.setMaximum(1'000'000'000);
    _histo_limit.setValue(cfg._histo_entries_limit);

    _histo_label.setFixedHeight(32);
    _histo_layout.addWidget(&_histo_label);
    _histo_layout.addStretch();
    _histo_layout.addWidget(&_histo_limit);
}

#ifndef _UNMODIFIED_KSHARK // Task colors
/**
 * @brief Sets up the layout, checkbox and explanation label for the
 * task coloring configuration.
 * 
 * @note It is dependent on the configuration 'SlConfig' singleton.
 */
void SlConfigWindow::setup_use_task_coloring() {
    // Configuration access here
    const SlConfig& cfg = SlConfig::get_instance();

    _task_col_btn.setChecked(cfg._use_task_colors);
    _task_col_layout.addWidget(&_task_col_label);
    _task_col_layout.addStretch();
    _task_col_layout.addWidget(&_task_col_btn);
}
#endif

/**
 * @brief Setup control elements for events meta. These control
 * elements are added dynamically and require special handling,
 * e.g. setting object names to find them afterwards when getting their
 * values.
 * 
* @note It is dependent on the configuration 'SlConfig' singleton.
 */
void SlConfigWindow::setup_events_meta_widget() {
    // Configuration access here
    const SlConfig& cfg = SlConfig::get_instance();
    
    // Create a header row, so that the user knows what is what
    QHBoxLayout* header_row = new QHBoxLayout{nullptr};
    QLabel* header_evt_name = new QLabel{this};
    header_evt_name->setText("Event name");
    QLabel* header_evt_allowed = new QLabel{this};
    header_evt_allowed->setText("Allowed");

    header_row->addWidget(header_evt_name);
    header_row->addStretch();
    header_row->addWidget(header_evt_allowed);
#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
    QLabel* header_evt_depth = new QLabel{this};
    header_evt_depth->setText("Preview stack offset");

    header_row->addStretch();
    header_row->addWidget(header_evt_depth);
#endif
    _events_meta_layout.addLayout(header_row);

    // Create controls for the events meta
    const events_meta_t& evts_meta = cfg.get_events_meta();
    // Supported entry index to differentiate object names
    int i = 0;
    
    for (auto it = evts_meta.cbegin(); it != evts_meta.cend(); ++it) {
        QHBoxLayout* row = new QHBoxLayout{nullptr};
        QLabel* evt_name = new QLabel{this};
        QCheckBox* evt_allowed = new QCheckBox{this};

        evt_name->setText(it->first.c_str());
        // Necessary for finding these later
        evt_name->setObjectName("evt_name_" + std::to_string(i));

#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
        evt_allowed->setChecked(it->second.first);
#else
        evt_allowed->setChecked(it->second);
#endif
        evt_allowed->setObjectName("evt_allowed_" + std::to_string(i));

        row->addWidget(evt_name);
        row->addStretch();
        row->addWidget(evt_allowed);

#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
        QSpinBox* evt_depth = new QSpinBox{this};
        
        evt_depth->setValue(it->second.second);
        evt_depth->setObjectName("evt_depth_" + std::to_string(i));
        evt_depth->setMinimum(0);
        evt_depth->setMaximum(100'000'000);

        row->addStretch();
        row->addWidget(evt_depth);
#endif

        _events_meta_layout.addLayout(row);
        ++i;
    }
}

/**
 * @brief Sets up the main layout of the configuration dialog.
 */
void SlConfigWindow::setup_layout() {
    // Don't allow resizing
    _layout.setSizeConstraint(QLayout::SetFixedSize);

    // Add all control elements
    _layout.addLayout(&_histo_layout);
    _layout.addWidget(_get_hline(this));
    _layout.addStretch();
#ifndef _UNMODIFIED_KSHARK // Task colors
    _layout.addLayout(&_task_col_layout);
#endif
    _layout.addLayout(&_def_btn_col_ctl_layout);
    _layout.addLayout(&_btn_outline_ctl_layout);
    _layout.addWidget(_get_hline(this));
    _layout.addStretch();
    _layout.addLayout(&_events_meta_layout);
    _layout.addWidget(_get_hline(this));
    _layout.addStretch();
    _layout.addLayout(&_endstage_btns_layout);

    // Set the layout of the dialog
	setLayout(&_layout);
}

/**
 * @brief Sets up the Apply and Close buttons by putting
 * them into a layout and assigning actions on pressing them.
 */
void SlConfigWindow::setup_endstage() {
    _endstage_btns_layout.addWidget(&_apply_button);
    _endstage_btns_layout.addWidget(&_close_button);

    connect(&_close_button,	&QPushButton::pressed,
            this, &QWidget::close);
    connect(&_apply_button, &QPushButton::pressed,
            this, [this]() { this->update_cfg(); this->close(); });
}

/**
 * @brief Loads current configuration values into the configuration
 * window's control elements and inner values.
 * 
 * @note It is dependent on the configuration 'SlConfig' singleton.
 */
void SlConfigWindow::load_cfg_values() {
    // Configuration access here
    const SlConfig& cfg = SlConfig::get_instance();

    // Setting of always-present members
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

#ifndef _UNMODIFIED_KSHARK // Task colors
    _task_col_btn.setChecked(cfg._use_task_colors);
#endif

    // Setting of dynamically added members - events meta
    const int SUPPORTED_EVENTS_COUNT = static_cast<int>(cfg.get_events_meta().size());
    const events_meta_t& cfg_evts_meta = cfg.get_events_meta();
    // For loop to get all the event-specific objects
    for (int i = 0; i < SUPPORTED_EVENTS_COUNT; ++i) {
        auto index_str = std::to_string(i);
        auto event_name = this->findChild<QLabel*>("evt_name_" + index_str);
        auto event_allowed = this->findChild<QCheckBox*>("evt_allowed_" + index_str);
#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
        auto event_depth = this->findChild<QSpinBox*>("evt_depth_" + index_str);
#endif
        // If all went well, events meta elements were found and can be changed
        if (event_name != nullptr
            && event_allowed != nullptr
#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
            && event_depth != nullptr
#endif
        ) {
            std::string event_name_str = event_name->text().toStdString();
#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
            const event_meta_t& specific_evt_meta = cfg_evts_meta.at(event_name_str);
            
            const allowed_t is_allowed = specific_evt_meta.first;
            
            depth_t stack_depth = specific_evt_meta.second;
            event_depth->setValue(static_cast<int>(stack_depth));
#else
            const allowed_t is_allowed = cfg_evts_meta.at(event_name_str);
#endif
            event_allowed->setChecked(is_allowed);
        } else { // Otherwise, notify the user - this most likely won't happen though
            auto info_dialog = new QMessageBox(QMessageBox::Warning,
                "Events meta load failed",
                "Events meta couldn't be loaded from the configuration.",
                QMessageBox::StandardButton::Ok, this);
            info_dialog->show();
        }
    }
}