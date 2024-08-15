/** TODO: Copyright? **/

/**
 * @file    SlConfig.hpp
 * @brief   This file has declaration of the config window class
 *          for the plugin.
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

// Plugin

// Usings
using depth_t = uint16_t;
using allowed_t = bool;
using event_meta_t = std::pair<allowed_t, depth_t>;
using event_name_t = std::string;
using events_meta_t = std::map<event_name_t, event_meta_t>;

class SlConfigWindow;

// Class
/**
 * @brief Singleton class for the config of the plugin.
*/
class SlConfig {
friend class SlConfigWindow;
public: // Class data members
    ///
    /// @brief Pointer to the main window used for window hierarchies.
    inline static KsMainWindow* main_w_ptr = nullptr;
private: // Data members
    /// @brief Limit value of how many entries may be visible in a
    /// histogram for the plugin to take effect.
    int32_t _histo_entries_limit{200};

    ///
    /// @brief Default color of Stacklook buttons, white.
    KsPlot::Color _default_btn_col{0xFF, 0xFF, 0xFF};

    ///
    /// @brief Default color of Stacklook buttons' outlines.
    KsPlot::Color _button_outline_col{0, 0, 0};

    /**
     * @brief Collection of event names with data about:
     * 
     * 1) Being allowed to be shown.
     * 2) Offset when viewing the stack items in the preview.
    */
    events_meta_t _events_meta{
        {{"sched/sched_switch", {true, 3}},
         {"sched/sched_wakeup", {false, 3}}}};
public: // Functions
    static SlConfig& get_instance();
    int32_t get_histo_limit() const;
    uint16_t get_stack_offset(event_name_t evt_name) const;    
    const KsPlot::Color get_default_btn_col() const; 
    const KsPlot::Color get_button_outline_col() const;
    const events_meta_t& get_events_meta() const;
    bool is_event_allowed(kshark_entry* entry) const;
};

class SlConfigWindow : public QWidget {
private: // Class data members
    static SlConfig& cfg;
private: // Qt data members
    ///
    /// @brief Layout for the widget's control elements.
    QVBoxLayout     _layout;

    QHBoxLayout     _endstage_btns_layout;

    // Triangle button inner fill
    QColor          _def_btn_col;

    QHBoxLayout     _def_btn_col_ctl_layout;
    
    QPushButton     _def_btn_col_btn;

    QLabel          _def_btn_col_preview;

    // Triangle button outline
    QColor          _btn_outline;

    QHBoxLayout     _btn_outline_ctl_layout;
    
    QPushButton     _btn_outline_btn;

    QLabel          _btn_outline_preview;

    // Histo limit
    QHBoxLayout     _histo_layout;

    QLabel          _histo_label;

    QSpinBox        _histo_limit;

    // Events meta

    QVBoxLayout     _events_meta_layout;
public: // Qt data members
    ///
    /// @brief Close button for the widget.
    QPushButton     _close_button;

    QPushButton     _apply_button;
private: // Qt functions
    void update_controls();
    void setup_histo_section();
    void setup_events_meta_widget();
public: // Functions
    SlConfigWindow();
    void load_cfg_values();
};

#endif