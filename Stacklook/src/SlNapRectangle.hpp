/** TODO: To discuss with supervisor
 * Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com>
*/

/**
 * @file    SlNapRectangle.hpp
 * @brief   Declarations & definitions of plugin's nap rectangle class.
 * 
 *          Nap := space in the histogram between a sched_switch and
 *          the closest next sched_wakeup events.
*/

#ifndef _SL_NAP_RECTANGLE_HPP
#define _SL_NAP_RECTANGLE_HPP

// C++
#include <cstring>
#include <string>

// KernelShark
#include "libkshark.h"
#include "KsPlotTools.hpp"

// Plugin
#include "stacklook.h"
#include "SlConfig.hpp"
#include "SlPrevState.hpp"
 
/**
 * @brief Represents a rectangle that is displayed in the "nap"
 * of a task - the sched_switch and its closest next sched_wakeup.
 * 
 * It cannot be interacted with, only drawn and looked at.
 * 
 * It will show a rectangle
 * with a background (in plugin, this background is assigned a color based
 * on the prev_state of the sched_switch event the nap rectangle uses),
 * upper and lower higlights of the outline (in plugin, their color is
 * the color of the task assigned to it by KernelShark) and text with
 * the full name of the prev_state of the sched_switch event of the nap
 * rectangle.
 */
class SlNapRectangle: public KsPlot::PlotObject {
private:
    ///
    /// @brief Pointer to the entry where the nap rectangle starts.
    const kshark_entry* _start_entry = nullptr;
    ///
    /// @brief Pointer to the entry where the nap rectangle ends.
    const kshark_entry* _end_entry = nullptr;
    ///
    /// @brief Main shape of the nap rectangle
    KsPlot::Rectangle _rect;
    ///
    /// @brief Upper line for the outline highlight.
    KsPlot::Line _outline_up;
    ///
    /// @brief Lower line for the outline highlight.
    KsPlot::Line _outline_down;
    /// @brief KernelShark plot objects for display of prev_state
    /// information of the sched_switch on the nap rectangle.
    KsPlot::TextBox _text;
    /// @brief Full name of a prev_state the sched_switch of the nap
    /// rectangle was in.
    std::string _raw_text;
private:
    /**
     * @brief Draws all the basic plot objects the nap rectangle is composed of.
     * In order of drawing: the rectangle, the outlines & if wide enough to display
     * text, the text box.
     * 
     * @note Despite the signature, the other parameters are ignored.
    */
    void _draw(const KsPlot::Color&, float) const override {
        // Don't draw into other plots basically.
        if (_rect.pointY(0) == _rect.pointY(3)) {
            _rect.draw();
            _outline_up.draw();
            _outline_down.draw();
            
            int nap_rect_width = (_rect.pointX(3) - _rect.pointX(0));
            int minimal_width = _raw_text.size() * FONT_SIZE;
            if (nap_rect_width > minimal_width) {
                _text.draw();
            }
        }
    }
public:
    /**
     * @brief Constructor of the nap_rectangle. Constructs the text box
     * and outlines from the data in arguments.
     * 
     * @param start: pointer to the event entry from which to start the
     * nap rectangle
     * @param end: pointer to the event entry at which to end the nap
     * rectangle
     * @param rect: KernelShark rectangle to display as basis for the
     * nap rectangle
     * @param outline_col: color of the outlines of the nap rectangle
     * @param text_col: color of the text to be displayed on the nap rectangle
    */
    explicit SlNapRectangle(const kshark_entry* start,
                            const kshark_entry* end,
                            const KsPlot::Rectangle& rect,
                            const KsPlot::Color& outline_col,
                            const KsPlot::Color& text_col)
        : _start_entry(start), _end_entry(end), _rect(rect)
        {
            // Outline
            const ksplot_point upper_point_a = *rect.point(0);
            const ksplot_point upper_point_b = *rect.point(3);
            _outline_up._color = outline_col;
            _outline_up.setA(upper_point_a.x, upper_point_a.y);
            _outline_up.setB(upper_point_b.x, upper_point_b.y);

            const ksplot_point lower_point_a = *rect.point(1);
            const ksplot_point lower_point_b = *rect.point(2);
            _outline_down._color = outline_col;
            _outline_down.setA(lower_point_a.x, lower_point_a.y);
            _outline_down.setB(lower_point_b.x, lower_point_b.y);

            // Text
            char start_ps = get_switch_prev_state(start)[0];
            std::string raw_text{LETTER_TO_NAME.at(start_ps)};
            // Capitalize to be more readable (and cooler)
            for(auto& character : raw_text) {
                character = std::toupper(character);
            }
            _raw_text = raw_text;

            int base_x_1 = _rect.pointX(0);
            int base_x_2 = _rect.pointX(3);
            int base_y = _rect.pointY(1);
            int actual_x = base_x_1 
                        + ((base_x_2 - base_x_1) / 2)
                        - (_raw_text.size() * FONT_SIZE / 3);
            KsPlot::Point final_point{actual_x, base_y - 1};


            _text = KsPlot::TextBox(get_bold_font_ptr(), _raw_text,
                                    text_col, final_point);
        }
};

#endif