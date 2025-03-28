/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    SlConfig.hpp
 * @brief   This file has declaration of the config window class
 *          for the plugin as well as the config object.
 * 
 * @note    Definitions in `SlConfig.cpp`.
*/

#ifndef _SL_CONFIG_HPP
#define _SL_CONFIG_HPP

//C++
#include <stdint.h>
#include <map>

// Qt
#include <QtWidgets>

// KernelShark
#include "libkshark.h"
#include "KsPlotTools.hpp"
#include "KsMainWindow.hpp"

// Usings

/**
 * @brief Whether Stacklook buttons should show above event entries.
 */
using allowed_t = bool;

/**
 * @brief ID of the allowed events in string form.
 */
using event_name_t = std::string;

#ifndef _UNMODIFIED_KSHARK // Stack offset, mouse hover
/**
 * @brief From which depth in the kernel stack (top being 0)
 * the preview should start.
 */
using depth_t = uint32_t;

/**
 * @brief Object holding meta information about events in Stacklook's
 * context - that being whether buttons are allowed to show up for an
 * event and from which item in the kernel stack should the preview start. 
 */
using event_meta_t = std::pair<allowed_t, depth_t>;

/**
 * @brief Map of event meta objects keyed by each event's name.
 */
using events_meta_t = std::map<event_name_t, event_meta_t>;
#else
using events_meta_t = std::map<event_name_t, allowed_t>;
#endif

// For friending purposes :)
class SlConfigWindow;

// Class
/**
 * @brief Singleton class for the config object of the plugin.
 * Holds values of: histogram limit until Stacklook buttons activate,
 * default color of Stacklook buttons, color of Stacklook buttons' outline,
 * if task colors should be used for buttons or not (modified KernelShark
 * feature only), and meta information about supported events - whether it's
 * allowed to show Stacklook buttons for them and how much offset from the
 * top of the kernel stack will the preview take into account when being
 * displayed.
 * 
 * Uses sane defaults and is NOT persistent, i.e. settings won't be preserved
 * across different KernelShark sessions.
*/
class SlConfig {
// Necessary for the config window to manipulate values in the config object.
friend class SlConfigWindow;
public: // Class data members
    ///
    /// @brief Pointer to the main window used for window hierarchies.
    inline static KsMainWindow* main_w_ptr = nullptr;
private: // Data members
    /// @brief Limit value of how many entries may be visible in a
    /// histogram for the plugin to take effect.
    int32_t _histo_entries_limit{10000};

    ///
    /// @brief Default color of Stacklook buttons, white.
    KsPlot::Color _default_btn_col{0xFF, 0xFF, 0xFF};

    /// @brief Default color of Stacklook buttons' outlines.
    /// Used when the buttons couldn't get the color of their task.
    KsPlot::Color _button_outline_col{0, 0, 0};

#ifndef _UNMODIFIED_KSHARK // Task colors, stack offset
    ///
    /// @brief Whether to use task colors for buttons or not.
    bool _use_task_colors{false};

    /**
     * @brief Map of event names keyed by their names with values:
     * 
     * 1) If Stacklook is allowed to show a button above the event's entry.
     * 2) Offset when viewing the kernel stack items in the preview.
     * 
     * @note Hard-coded values imply a defined set of supported events.
     * 
     * @warning The preview offset value will not have any effect if
     * unmodified KernelShark is used.
    */
    events_meta_t _events_meta{
        {{"sched/sched_switch", {true, 3}},
         {"sched/sched_waking", {true, 3}}}};
#else
    /**
     * @brief Map of event names keyed by their names with values:
     * 
     * 1) If Stacklook is allowed to show a button above the event's entry.
     * 
     * @note Hard-coded values imply a defined set of supported events.
     * 
    */
    events_meta_t _events_meta{
        {{"sched/sched_switch", true},
         {"sched/sched_waking", false}}};
#endif
public: // Functions
    static SlConfig& get_instance();
    int32_t get_histo_limit() const;
#ifndef _UNMODIFIED_KSHARK // Task colors, stack offset
    bool get_use_task_colors() const;
    depth_t get_stack_offset(event_name_t evt_name) const;
#endif    
    const KsPlot::Color get_default_btn_col() const; 
    const KsPlot::Color get_button_outline_col() const;
    const events_meta_t& get_events_meta() const;
    bool is_event_allowed(const kshark_entry* entry) const;
};

/**
 * @brief Widget class for modifying the configuration via GUI.
 * It is a fixed size dialog window that allows modification
 * of all that is in the config object by applying changes via
 * the Apply button. Changes won't be saved unless this is done.
 */
class SlConfigWindow : public QWidget {
private: // Qt data members
    ///
    /// @brief Layout for the widget's control elements.
    QVBoxLayout     _layout;
    
    /// @brief Layout for the Apply and Close buttons.
    QHBoxLayout     _endstage_btns_layout;

    // Triangle button inner fill

    /// @brief Color displayed in the preview label for default color
    /// of Stacklook buttons.
    /// This value changes the config object's value and vice-versa.
    QColor          _def_btn_col;
    
    /// @brief Layout for the preview label and color change
    /// buttons for the default color of Stacklook buttons.
    QHBoxLayout     _def_btn_col_ctl_layout;
    
    /// @brief Button that invokes color dialog for the user
    /// to easily change the default color of Stacklook buttons.
    QPushButton     _def_btn_col_btn;

    /// @brief Label without text, used to display what color the
    /// user either currently uses or has chosen from the color
    /// as default for Stacklook buttons.
    QLabel          _def_btn_col_preview;

    // Triangle button outline

    /// @brief Color displayed in the preview label for outline color
    /// of Stacklook buttons.
    /// This value changes the config object's value and vice-versa.
    QColor          _btn_outline;

    /// @brief Layout for the preview label and color change
    /// buttons for the outline color of Stacklook buttons.
    QHBoxLayout     _btn_outline_ctl_layout;
    
    /// @brief Button that invokes color dialog for the user
    /// to easily change the outline color of Stacklook buttons.
    QPushButton     _btn_outline_btn;

    /// @brief Label without text, used to display what color the
    /// user either currently uses or has chosen from the color
    /// for the outline of Stacklook buttons.
    QLabel          _btn_outline_preview;

    // Histo limit

    /// @brief Layout used for the spinbox and explanation
    /// of what it does in the label.
    QHBoxLayout     _histo_layout;

    ///
    /// @brief Explanation of what the spinbox next to it does.
    QLabel          _histo_label;

    /// @brief Spinbox used to change the limit of entries visible
    /// before Stacklook buttons show up.
    QSpinBox        _histo_limit;
#ifndef _UNMODIFIED_KSHARK // Task colors
    // Task-like coloring

    /// @brief Layout used for the button and explanation of
    /// what it does.
    QHBoxLayout     _task_col_layout;

    ///
    /// @brief Explanation of what the checkbox next to it does.
    QLabel          _task_col_label;

    /// @brief Toggles whether to use task colors for buttons or not.
    QCheckBox       _task_col_btn;
#endif
    // Events meta

    /// @brief Layout used for the section of the config window
    /// which changes meta information of events in Stacklook's context.
    QVBoxLayout     _events_meta_layout;
public: // Qt data members
    ///
    /// @brief Close button for the widget.
    QPushButton     _close_button;

    /// @brief Button applies changes to the
    /// configuration object and shows an info dialog.
    QPushButton     _apply_button;
private: // Qt functions
    void update_cfg();
    void setup_histo_section();
#ifndef _UNMODIFIED_KSHARK // Task colors
    void setup_use_task_coloring();
#endif
    void setup_events_meta_widget();
    void setup_layout();
    void setup_endstage();
public: // Functions
    SlConfigWindow();
    void load_cfg_values();
};

#endif