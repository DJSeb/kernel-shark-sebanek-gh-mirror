/**
 * For internal logic.
*/

// C++
#include <vector>
#include <string>
#include <iostream>

// KernelShark
#include "KsPlugins.hpp"
#include "KsPlotTools.hpp"

// Plugin headers
#include "stacklook.h"
#include "Stacklook.hpp"

// Runtime constants
const static std::string STACK_BUTTON_TEXT = "STACK";
const static std::string SEARCHED_EVENT = "sched/sched_switch";

static KsPlot::PlotObject* makeText(std::vector<const KsPlot::Graph*> graph,
                                     std::vector<int> bin,
                                     std::vector<kshark_data_field_int64*> data,
                                     KsPlot::Color c, float size) {
    // Adjust values
    int x, y;
    x = graph[0]->bin(bin[0])._val.x() - FONT_SIZE * 0.75;
    y = graph[0]->bin(bin[0])._val.y() - FONT_SIZE / 3;
    KsPlot::TextBox* textBox =
        new KsPlot::TextBox(get_font_ptr(),
                            STACK_BUTTON_TEXT,
                            c, KsPlot::Point{x, y});
    
    return textBox;
}

static KsPlot::PlotObject* makeTriangle(std::vector<const KsPlot::Graph*> graph,
                                        std::vector<int> bin,
                                        std::vector<kshark_data_field_int64*> data,
                                        KsPlot::Color col, float size) {
    // Change below to some actual values
    // Base point
    int x, y;
    x = graph[0]->bin(bin[0])._val.x() - FONT_SIZE * 0.75;
    y = graph[0]->bin(bin[0])._val.y() - FONT_SIZE / 3;
    
    // Triangle points
    KsPlot::Point a {x, y};
    KsPlot::Point b {x + 50, y};
    KsPlot::Point c {x + 25, y + 20};
    
    // Triangle
    StackTriangle* backTriangle = new StackTriangle();
    backTriangle->setFill(true);
    backTriangle->setPoint(0, a);
    backTriangle->setPoint(1, b);
    backTriangle->setPoint(2, c);

    return backTriangle;
}

/**
 * @brief Plugin's draw function.
 *
 * @param argv_c: A C pointer to be converted to KsCppArgV (C++ struct).
 * @param sd: Data stream identifier.
 * @param pid: Process Id.
 * @param draw_action: Draw action identifier.
 */
void draw_plot_buttons(struct kshark_cpp_argv* argv_c, int sd,
                           int val, int draw_action) {
    KsCppArgV* argVCpp KS_ARGV_TO_CPP(argv_c);
    kshark_data_container* plugin_data;

    // If I am to draw and on a CPU, don't do that (I think).
    if (!(draw_action && KSHARK_CPU_DRAW)) return;
    // Don't draw with too many bins (configurable zoom-in indicator actually)
    if (argVCpp->_histo->tot_count > 200) return;

    plugin_data = __get_context(sd)->stacks_data;
    // Couldn't get the context container (any reason)
    if (!plugin_data) {
        return;
    }

    auto checkFunc = [=] (kshark_data_container* data_c, ssize_t t) {
        bool correct_event_id = (__get_context(sd)->ss_event_id 
                    == data_c->data[t]->entry->event_id);
        bool correct_cpu = (data_c->data[t]->entry->cpu == val);
        return correct_cpu && correct_event_id;
    };

    _draw_triangle_w_text(argVCpp, plugin_data, checkFunc, sd, val, draw_action);
}

static void _draw_triangle_w_text(KsCppArgV* argv, 
                                  kshark_data_container* dc,
                                  IsApplicableFunc checkFunc,
                                  int sd, int val,
                                  int draw_action) {
    eventFieldPlotMin(argv,
                      dc,
                      checkFunc,
                      makeTriangle,
                      {0x25, 0x69, 0x90},
                      10);

    eventFieldPlotMin(argv,
                      dc,
                      checkFunc,
                      makeText,
                      {0x0F, 0x0F, 0x0F},
                      10);
}