/** TODO:
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
#include "KsPlugins.hpp"
#include "KsMainWindow.hpp"
#include "KsPlotTools.hpp"

// Plugin headers
#include "stacklook.h"
#include "Stacklook.hpp"
#include "SlDetailedWindow.hpp"
#include "SlButton.hpp"

// Static variables

constexpr static int HISTO_ENTRIES_LIMIT = 200;

const static KsPlot::Color DEFAULT_BUTTON_COL{0x60, 0x69, 0x90};

/**
 * @brief Color table for the tasks so that the stacklook buttons look
 * prettier.
*/
static KsPlot::ColorTable task_colors;

/**
 * @brief Indicates whether the colortable has already been loaded or not.
*/
static bool loaded_colors;

// Static functions

/**
 * @brief General function for checking whether to show Stacklook button
 * in the plot.
 * 
 * @param entry: KernelShark entry whose properties must be checked
 * @param ctx: Stacklook plugin context
*/
static bool _check_function_general(kshark_entry* entry,
                                    plugin_stacklook_ctx* ctx) {
    bool correct_event_id = (ctx->sswitch_event_id == entry->event_id)
                            || (ctx->swake_event_id == entry->event_id);

    bool is_visible_event = entry->visible
                            & kshark_filter_masks::KS_EVENT_VIEW_FILTER_MASK;
    bool is_visible_graph = entry->visible
                            & kshark_filter_masks::KS_GRAPH_VIEW_FILTER_MASK;
    
    return correct_event_id && is_visible_event && is_visible_graph;
}

/**
 * @brief Loads the current task colortable into `task_colors`.
*/
static void _load_current_colortable() {
    if (!loaded_colors) {
        task_colors = KsPlot::taskColorTable();
        loaded_colors = true;
    }
}

/**
 * @brief Returns either a default color or one present in the`task_colors`
 * color table.
 * 
 * @param task_pid: task PID to index `task_colors`.
 * @param default_color: color to be used in case we fail in finding a color
 * in the colortable.
*/
static KsPlot::Color _get_color(int32_t task_pid, KsPlot::Color default_color) {
    KsPlot::Color triangle_color = default_color;
    try {
        triangle_color = task_colors.at(task_pid);
    } catch (std::out_of_range&) {
        triangle_color = default_color;
    }

    return triangle_color;
}

/**
 * @brief Returns either black if the background color's intensity is too great,
 * otherwise returns white. Limit to intensity is `128.0`.
 * 
 * @param bg_color_intensity: computed intensity from an RGB color.
*/
static KsPlot::Color _black_or_white_text(float bg_color_intensity) {
    const KsPlot::Color WHITE {0xFF, 0xFF, 0xFF};
    const KsPlot::Color BLACK {0, 0, 0};
    constexpr float INTENSITY_LIMIT = 128.f;

    return (bg_color_intensity > INTENSITY_LIMIT) ? BLACK : WHITE;
}

/**
 * @brief Gets the color intensity using the formula
 * `(red * 0.299) + (green * 0.587) + (blue * 0.114)`.
 * 
 * @param c: RGB color value whose components will be checked.
*/
static float _get_color_intensity(const KsPlot::Color& c) {
    return (c.b() * 0.114f) + (c.g() * 0.587f) + (c.r() * 0.299f);
}

/**
 * @brief Creates a clickable Stacklook button to be displayed on the plot.
 * 
 * @param graph: KernelShark graphs
 * @param bin: KernelShark bins
 * @param data: entries with auxiliary data to draw buttons for
 * @param col: default color of the Stacklook button's insides
*/
static SlTriangleButton* make_sl_button(std::vector<const KsPlot::Graph*> graph,
                                        std::vector<int> bin,
                                        std::vector<kshark_data_field_int64*> data,
                                        KsPlot::Color col, float) {
    // Constants
    constexpr int32_t BUTTON_TEXT_OFFSET = 14;
    const std::string STACK_BUTTON_TEXT = "STACK";
    const KsPlot::Color OUTLINE_COLOR {0, 0, 0};

    // Marked entry (there's just one on the top)
    kshark_entry* event_entry = data[0]->entry;

    // Base point
    KsPlot::Point base_point = graph[0]->bin(bin[0])._val;

    // Coords
    int x = base_point.x();
    int y = base_point.y();
    
    // Triangle points
    /*
       0 ------ 1
        \     /
         \   /
          \ /
           2      
    */

    // Triangle points
    KsPlot::Point a {x - 24, y - 25};
    KsPlot::Point b {x + 24, y - 25};
    KsPlot::Point c {x, y - 5};

   // Outer triangle
    auto inner_triangle = KsPlot::Triangle();
    inner_triangle.setPoint(0, a);
    inner_triangle.setPoint(1, b);
    inner_triangle.setPoint(2, c);

    /** Colors are a bit wonky with sched_switch events. Using the function
     * makes it consistent across the board.
    */
    inner_triangle._color = _get_color(kshark_get_pid(event_entry), col);

    // Inner triangle
    auto back_triangle = KsPlot::Triangle(inner_triangle);
    back_triangle._color = OUTLINE_COLOR;
    back_triangle.setFill(false);

    int text_x = x - BUTTON_TEXT_OFFSET;
    int text_y = y - BUTTON_TEXT_OFFSET;

    float bg_intensity = _get_color_intensity(inner_triangle._color);
    auto text_color = _black_or_white_text(bg_intensity);

    auto text = KsPlot::TextBox(get_font_ptr(), STACK_BUTTON_TEXT, text_color,
                                KsPlot::Point{text_x, text_y});

    auto sl_button = new SlTriangleButton(event_entry, back_triangle,
                                          inner_triangle, text);

    return sl_button;
}

/**
 * @brief Function wrapper for drawing on the plot.
 * 
 * @param argv: The C++ arguments of the drawing function of the plugin.
 * @param dc: Input location for the container of the event's data.
 * @param checkFunc: check function used to select events from data container.
*/
static void _draw_stacklook_button(KsCppArgV* argv, 
                                   kshark_data_container* dc,
                                   IsApplicableFunc checkFunc,
                                   pluginShapeFunc makeButton) {    
    eventFieldPlotMin(argv, dc, checkFunc, makeButton,
                      DEFAULT_BUTTON_COL, -1);
}

// Global functions

/**
 * @brief Function for correct destroying of opened stacklook
 * windows.
*/
void clean_opened_views(void* view_container) {
    auto views = *(std::vector<SlDetailedView*>*)(view_container);
    for(auto view : views) {
        if (view != nullptr) {
            delete view;
        }
    }
    delete (std::vector<SlDetailedView*>*)(view_container);
}

/**  **/
void* init_views() {
    auto views = new std::vector<SlDetailedView*>{};
    SlDetailedView::opened_views = views;
    return (void*)(views);
}

/**
 * @brief Plugin's draw function.
 *
 * @param argv_c: a C pointer to be converted to KsCppArgV
 * @param sd: data stream identifier
 * @param val: process or CPU ID value
 * @param draw_action: draw action identifier
*/
void draw_plot_buttons(struct kshark_cpp_argv* argv_c, int sd,
                       int val, int draw_action) {
    KsCppArgV* argVCpp KS_ARGV_TO_CPP(argv_c);
    plugin_stacklook_ctx* ctx = __get_context(sd);
    kshark_data_container* plugin_data;
    
    // Don't draw if not drawing tasks or CPUs.
    if (!(draw_action == KSHARK_CPU_DRAW 
          || draw_action == KSHARK_TASK_DRAW)) {
        return;
    }
    
    // Don't draw with too many bins (configurable zoom-in indicator)
    if (argVCpp->_histo->tot_count > HISTO_ENTRIES_LIMIT) {
        if (loaded_colors) {
            loaded_colors = false;
        }
        return;
    }

    plugin_data = __get_context(sd)->collected_events;
    // Couldn't get the context container (any reason)
    if (!plugin_data) {
        return;
    }

    _load_current_colortable();
    
    IsApplicableFunc check_func;
    
    if (draw_action == KSHARK_TASK_DRAW) {
        check_func = [=] (kshark_data_container* data_c, ssize_t t) {
            kshark_entry* entry = data_c->data[t]->entry;
            bool correct_pid = (entry->pid == val);
            return _check_function_general(entry, ctx) && correct_pid;
        };
    } else if (draw_action == KSHARK_CPU_DRAW) {
        check_func = [=] (kshark_data_container* data_c, ssize_t t) {
            kshark_entry* entry = data_c->data[t]->entry;
            bool correct_cpu = (data_c->data[t]->entry->cpu == val);
            return _check_function_general(entry, ctx) && correct_cpu;
        };
    }

    _draw_stacklook_button(argVCpp, plugin_data, check_func, make_sl_button);
}

/**
 * @brief Give the plugin a pointer to KS's main window to allow
 * GUI manipulation.
*/
__hidden void* plugin_set_gui_ptr(void* gui_ptr) {
    SlDetailedView::main_w_ptr = static_cast<KsMainWindow*>(gui_ptr);
    return nullptr;
}