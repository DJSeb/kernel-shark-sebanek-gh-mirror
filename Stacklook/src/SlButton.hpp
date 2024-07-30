/** TODO: Copyright? **/

/**
 * @file    SlButton.hpp
 * @brief   Declares a special plot object class holding components
 *          of the button and its mouse interaction reactions.
 * 
 *          This class NECESSITATES that a modified KernelShark is used.
 *          To remove this dependency, remove/modify the code with warnings
 *          in its documentation.
 * 
 * @note    Definitions in `SlButton.cpp`.
*/

#ifndef _SL_BUTTON_HPP
#define _SL_BUTTON_HPP

// KernelShark
#include "libkshark.h"
#include "KsPlotTools.hpp"

/**
 * @brief Special button class for the Stacklook plugin, child of
 * KernelShark's PlotObject.
 * 
 * Graphically, it is an outline triangle that contains another
 * inner triangle and a textbox. This is so that everything is
 * drawn together by KernelShark.
*/
class SlTriangleButton : public KsPlot::PlotObject {
private: // Data members
    /**
     * @brief What event the button points at and gets data from for
     * the window.
    */
    kshark_entry* _event_entry;
    // Graphical
    /**
     * @brief Triangle which creates the outline of the button.
     * Black coloured and filled.
    */
    KsPlot::Triangle _outline_triangle;
    /**
     * @brief Triangle which forms the innards of the button. Color
     * depends on the entry's PID. 
    */
    KsPlot::Triangle _inner_triangle;
    /**
     * @brief Textbox to specify what the button will show.
    */
    KsPlot::TextBox _text;
public: // Functions
    /** 
     * @brief Explicit constructor for the button (you should use
     * only this).
     * 
     * @param event_entry - entry the button gets data from
     * @param outer - triangle used for the black outline
     * @param inner - triangle used as the filling
     * @param text - text on the button
    */ 
    explicit SlTriangleButton(kshark_entry* event_entry,
                              KsPlot::Triangle& outer,
                              KsPlot::Triangle& inner,
                              KsPlot::TextBox& text)
        : KsPlot::PlotObject(),
          _event_entry(event_entry),
          _outline_triangle(outer),
          _inner_triangle(inner),
          _text(text) {}

    double distance(int x, int y) const override;
private: // Functions
    void _doubleClick() const override;
    void _draw(const KsPlot::Color&, float) const override;

    /// WARN:
    /// WILL NOT WORK WITHOUT MODIFIED KERNELSHARK WITH SUPPORT
    /// FOR MOUSE MOVE OVER PLOTOBJECT REACTIONS
    
    void _mouseHover() const override;
};

#endif