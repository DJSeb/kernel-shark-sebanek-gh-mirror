/**
 * For internal logic.
*/

// C++
#include <vector>
#include <string>
#include <iostream>

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
#include "Stacklook.hpp"

// Utilities
#include "_utilities.hpp"

// Statics
static KsMainWindow* main_w_ptr;

/**
 * @brief Give the plugin a pointer to KS's main window to allow
 * GUI manipulation.
*/
__hidden void* plugin_set_gui_ptr(void* gui_ptr) {
    main_w_ptr = static_cast<KsMainWindow*>(gui_ptr);
    return nullptr;
}

// Classes
class SlTriangleButton : public KsPlot::Triangle {
private:
    void _doubleClick() const override {
        log("Opening dialog from triangle...");
    }
};

class SlTextBox : public KsPlot::TextBox {
public:
    SlTextBox(ksplot_font* f, const std::string& text,
              const KsPlot::Color& col, const KsPlot::Point& pos)
    : KsPlot::TextBox(f, text, col, pos) {}
private:
    void _doubleClick() const override {
        log("Opening dialog from text...");
    }
};

// Runtime constants
const static std::string STACK_BUTTON_TEXT = "STACK";
const static std::string SEARCHED_EVENT = "sched/sched_switch";

// Statics
static KsPlot::PlotObject* makeText(std::vector<const KsPlot::Graph*> graph,
                                     std::vector<int> bin,
                                     std::vector<kshark_data_field_int64*> data,
                                     KsPlot::Color c, float size) {
    // Adjust values
    int x, y;
    x = graph[0]->bin(bin[0])._val.x() - 14;
    y = graph[0]->bin(bin[0])._val.y() - 14;
    SlTextBox* button_text = 
        new SlTextBox(get_font_ptr(),
                      STACK_BUTTON_TEXT,
                      c, KsPlot::Point{x, y});
    
    return button_text;
}

static KsPlot::PlotObject* makeTriangle(std::vector<const KsPlot::Graph*> graph,
                                        std::vector<int> bin,
                                        std::vector<kshark_data_field_int64*> data,
                                        KsPlot::Color col, float size) {
    // Change below to some actual values
    // Base point
    int x, y;
    int x_start = graph[0]->bin(bin[0])._val.x();
    int y_start = graph[0]->bin(bin[0])._val.y();
    x = x_start - 24;
    y = y_start - 25;
    
    // Triangle points
    KsPlot::Point a {x, y};
    KsPlot::Point b {x + 48, y};
    KsPlot::Point c {x + 24, y + 20};
    
    // Triangle
    SlTriangleButton* backTriangle = new SlTriangleButton();
    backTriangle->setFill(true);
    backTriangle->setPoint(0, a);
    backTriangle->setPoint(1, b);
    backTriangle->setPoint(2, c);

    return backTriangle;
}

static void _draw_triangle_w_text(KsCppArgV* argv, 
                                  kshark_data_container* dc,
                                  IsApplicableFunc checkFunc,
                                  int sd, int val,
                                  int draw_action) {
    // First drawn will be the triangles actually
    eventFieldPlotMin(argv,
                      dc,
                      checkFunc,
                      makeText,
                      {0xFF, 0xFF, 0xFF},
                      10);
    
    eventFieldPlotMin(argv,
                      dc,
                      checkFunc,
                      makeTriangle,
                      {0x25, 0x69, 0x90},
                      10);
}

// Globals

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
