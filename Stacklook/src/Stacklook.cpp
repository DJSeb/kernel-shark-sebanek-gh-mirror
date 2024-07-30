
/** TODO: To discuss with supervisor
 * Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com>
*/

/**
 * @file    Stacklook.cpp
 * @brief   Central for most internal logic of the plugin, bridge between the
 *          C code and the C++ and the Qt code.
*/

// Inclusions

// C++
#include <vector>
#include <string>
#include <stdint.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"
#include "KsPlugins.hpp"
#include "KsMainWindow.hpp"
#include "KsPlotTools.hpp"

// Plugin headers
#include "stacklook.h"
#include "SlDetailedView.hpp"
#include "SlButton.hpp"

// Constants

/// @brief Limit value of how many entries may be visible in a
/// histogram for the plugin to take effect.
constexpr static int HISTO_ENTRIES_LIMIT = 200;

///
/// @brief Default color of Stacklook buttons, white.
const static KsPlot::Color DEFAULT_BUTTON_COL{0xFF, 0xFF, 0xFF};

// Static variables

/**
 * @brief Color table of the tasks so that the Stacklook buttons look
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
 * 
 * @returns True if the entry fulfills all of function's requirements,
 *          false otherwise.
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
 * color table based on Process ID of a task.
 * 
 * @param task_pid: task PID to index `task_colors`.
 * @param default_color: color to be used in case we fail in finding a color
 * in the colortable.
 * 
 * @returns Default color or one present in the`task_colors` color table.
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
 * 
 * @returns Black on high intensity, white otheriwse.
*/
static KsPlot::Color _black_or_white_text(float bg_color_intensity) {
    const static KsPlot::Color WHITE {0xFF, 0xFF, 0xFF};
    const static KsPlot::Color BLACK {0, 0, 0};
    constexpr float INTENSITY_LIMIT = 128.f;

    return (bg_color_intensity > INTENSITY_LIMIT) ? BLACK : WHITE;
}

/**
 * @brief Gets the color intensity using the formula
 * `(red * 0.299) + (green * 0.587) + (blue * 0.114)`.
 * 
 * @param c: RGB color value whose components will be checked.
 * 
 * @returns Color intensity as floating-point value.
*/
static float _get_color_intensity(const KsPlot::Color& c) {
    // Color multipliers are chosen the way they are based on human
    // eye's receptiveness to each color (green being the color human
    // eyes focus on the most).
    return (c.b() * 0.114f) + (c.g() * 0.587f) + (c.r() * 0.299f);
}

/**
 * @brief Creates a clickable Stacklook button to be displayed on the plot.
 * 
 * @param graph: KernelShark graphs
 * @param bin: KernelShark bins
 * @param data: entries with auxiliary data to draw buttons for
 * @param col: default color of the Stacklook button's insides
 * 
 * @returns Pointer to the created button.
*/
static SlTriangleButton* make_sl_button(std::vector<const KsPlot::Graph*> graph,
                                        std::vector<int> bin,
                                        std::vector<kshark_data_field_int64*> data,
                                        KsPlot::Color col, float) {
    // Constants
    constexpr int32_t BUTTON_TEXT_OFFSET = 14;
    const std::string STACK_BUTTON_TEXT = "STACK";
    const KsPlot::Color OUTLINE_COLOR {0, 0, 0};

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

    // Colors are a bit wonky with sched_switch events. Using the function
    // makes it consistent across the board.
    inner_triangle._color = _get_color(kshark_get_pid(event_entry), col);

    // Inner triangle
    auto back_triangle = KsPlot::Triangle(inner_triangle);
    back_triangle._color = OUTLINE_COLOR;
    back_triangle.setFill(false);

    // Text coords
    int text_x = x - BUTTON_TEXT_OFFSET;
    int text_y = y - BUTTON_TEXT_OFFSET;

    // Colors
    float bg_intensity = _get_color_intensity(inner_triangle._color);
    auto text_color = _black_or_white_text(bg_intensity);

    // Final object initialitations
    auto text = KsPlot::TextBox(get_font_ptr(), STACK_BUTTON_TEXT, text_color,
                                KsPlot::Point{text_x, text_y});

    auto sl_button = new SlTriangleButton(event_entry, back_triangle,
                                          inner_triangle, text);

    return sl_button;
}

/**
 * @brief Function wrapper for drawing Stacklook buttons on the plot.
 * 
 * @param argv: The C++ arguments of the drawing function of the plugin.
 * @param dc: Input location for the container of the event's data.
 * @param checkFunc: check function used to select events from data container.
 * @param makeButton: function which specifies what will be drawn and how
*/
static void _draw_stacklook_button(KsCppArgV* argv, 
                                   kshark_data_container* dc,
                                   IsApplicableFunc checkFunc,
                                   pluginShapeFunc makeButton) {    
    // -1 means default size
    // The default color of buttons will hopefully be overriden when
    // the button's entry's task PID is found.
    
    eventFieldPlotMin(argv, dc, checkFunc, makeButton,
                      DEFAULT_BUTTON_COL, -1);
}

// Functions used in C code

/**
 * @brief Ensures correct destruction of opened Stacklook
 * view windows and the vector which holds them.
 * 
 * @param view_container: vector containing opened Stacklook view
 * windows.
*/
void clean_opened_views(void* view_container) {
    auto views_ptr = (std::vector<SlDetailedView*>*)(view_container);
    
    // To prevent nullptr access
    if (views_ptr == nullptr) {
        return;
    }

    auto views = *views_ptr;
    
    // Cycle & destroy
    for(auto view : views) {
        if (view != nullptr) {
            delete view;
        }
    }

    // Finally, destroy the vector itself
    delete (std::vector<SlDetailedView*>*)(view_container);
}

/**
 * @brief Initializes the vector managing the view windows. Also puts
 * the typed pointer into the windows' class member.
 * 
 * @returns Vector's heap address as a void pointer.
*/
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
 * @brief Give the plugin a pointer to KernalShark's main window to allow
 * GUI manipulation.
 * 
 * This is where plugin menu can be made and initialized.
 * 
 * @returns Nullptr as there is no menu control created.
*/
__hidden void* plugin_set_gui_ptr(void* gui_ptr) {
    KsMainWindow* main_w = static_cast<KsMainWindow*>(gui_ptr);
    SlDetailedView::main_w_ptr = main_w;

    return nullptr;
}