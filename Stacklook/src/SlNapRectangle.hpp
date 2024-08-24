#ifndef _SL_NAP_RECTANGLE_HPP
#define _SL_NAP_RECTANGLE_HPP

#ifdef _VISUALIZE_NAPS
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
 

class SlNapRectangle: public KsPlot::PlotObject {
private:
    const kshark_entry* _start_entry = nullptr;
    const kshark_entry* _end_entry = nullptr;
    KsPlot::Rectangle _rect;

    KsPlot::Line _outline_up;
    KsPlot::Line _outline_down;
    KsPlot::TextBox _text;
    std::string _raw_text;
private:
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

#endif