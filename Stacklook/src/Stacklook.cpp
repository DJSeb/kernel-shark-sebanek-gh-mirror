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

// Plugin header
#include "stacklook.h"

// Runtime constants
const std::string STACK_BUTTON_TEXT = "STACK";
const std::string SEARCHED_EVENT = "sched/sched_switch";

static KsPlot::PlotObject* makeText(std::vector<const KsPlot::Graph*> graph,
                                     std::vector<int> bin,
                                     std::vector<kshark_data_field_int64*> data,
                                     KsPlot::Color c, float size) {
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
    KsPlot::Triangle* backTriangle = new KsPlot::Triangle();
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

    // If I am to draw and on a CPU, don't do that.
    if (!(draw_action && KSHARK_CPU_DRAW)) return;
    // Don't draw with too many bins (configurable zoom-in indicator actually)
    if (argVCpp->_histo->tot_count > 200) return;

    std::cout << "(SKIP) Checking plugin context state." << std::endl;

    plugin_data = __get_context(sd)->stacks_data;
    // Couldn't get the context container (any reason)
    if (!plugin_data) return;

    std::cout << "Making check function." << std::endl;

    auto checkFunc = [&] (kshark_data_container* data_c, ssize_t t) {
        auto event_name = std::string{kshark_get_event_name(data_c->data[t]->entry)};
        bool is_correct_event = (SEARCHED_EVENT == event_name);
        return is_correct_event;
    };

    std::cout << "Trying to plot triangles." << std::endl;

    /*eventFieldPlotMin(argVCpp,
                      plugin_data,
                      checkFunc,
                      makeTriangle,
                      {0x80, 0x17, 0xA8},
                      10);*/
    
    std::cout << "Trying to plot text." << std::endl;

    eventFieldPlotMin(argVCpp,
                      plugin_data,
                      checkFunc,
                      makeText,
                      {0x0F, 0x0F, 0x0F},
                      10);
}