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

// Qt
#include <QtWidgets>

// KernelShark
#include "KsPlotTools.hpp"

// Plugin
#include "SlPrevState.hpp"

/**
 * @brief Singleton class for the config window.
*/
class SlConfig : public QWidget {
private: // Data members
    /// @brief Limit value of how many entries may be visible in a
    /// histogram for the plugin to take effect.
    int32_t _histo_entries_limit{200};
    
    /// @brief How many entries would be skipped when using the preview
    /// bar. By default, it's 3.
    int16_t _stack_offset{3};

    ///
    /// @brief Default color of Stacklook buttons, white.
    KsPlot::Color _default_btn_col{0xFF, 0xFF, 0xFF};

    ///
    /// @brief Default color of Stacklook buttons' outlines.
    KsPlot::Color _default_outline_col{0, 0, 0};

private: // Functions
private: // Qt data members
    ///
    /// @brief Layout for the widget's control elements.
    QVBoxLayout     _layout;
private: // Qt functions
public: // Functions
    static SlConfig& get_instance();
    int32_t get_histo_limit() const;
    int16_t get_stack_offset() const;    
    const KsPlot::Color get_default_btn_col() const; 
    const KsPlot::Color get_default_outline_col() const;
public: // Qt data members
    ///
    /// @brief Close button for the widget.
    QPushButton     _close_button;
};

#endif