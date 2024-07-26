#ifndef _SL_BUTTON_HPP
#define _SL_BUTTON_HPP

// C++
#include <vector>
#include <string>
#include <iostream>

// Qt
#include <QtCore>
#include <QEvent>
#include <QHoverEvent>

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"
#include "libkshark-plot.h"
#include "libkshark-model.h"
#include "KsPlugins.hpp"
#include "KsMainWindow.hpp"
#include "KsPlotTools.hpp"

// Plugin headers
#include "stacklook.h"
#include "SlDetailedWindow.hpp"

/**
 * @brief Special button class just for the plugin.
 * It itself is a triangle that contains another inner triangle
 * and a textbox. This is so that everything is drawn together
 * and logical structuring is retained.
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
    SlTriangleButton() : KsPlot::PlotObject() {}
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
    void _mouseHover() const override;
};

#endif