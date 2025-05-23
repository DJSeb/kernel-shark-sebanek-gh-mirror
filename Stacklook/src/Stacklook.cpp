/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    Stacklook.cpp
 * @brief   Central for most internal logic of the plugin, bridge between the
 *          C code and the C++ and the Qt code.
*/

// Inclusions

// C
#include <stdint.h>

// C++
#include <vector>
#include <string>
#include <map>
#include <unordered_set>

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"
#include "KsPlugins.hpp"
#include "KsMainWindow.hpp"
#include "KsPlotTools.hpp"

// Plugin headers
#include "stacklook.h"
#include "SlButton.hpp"
#include "SlConfig.hpp"

// #########################################################################
// Static variables
/**
 * @brief Static pointer to the configuration window.
 */
static SlConfigWindow* cfg_window;

// #########################################################################
// Static functions

/**
 * @brief General function for checking whether to show Stacklook button
 * in the plot.
 * 
 * @param entry: KernelShark entry whose properties must be checked
 * @param kstack_entry: KernelShark entry containing the correct kernel stack
 * of the event in `entry`.
 * @param ctx: Stacklook plugin context
 * 
 * @returns True if the entry fulfills all of function's requirements,
 *          false otherwise.
 *
 * @note It is dependent on the configuration 'SlConfig' singleton.
*/
static bool _check_function_general(const kshark_entry* entry,
                                    const kshark_entry* kstack_entry,
                                    const plugin_stacklook_ctx* ctx) {
    if (!entry || !kstack_entry)
        return false;
    
    bool correct_event_id = (ctx->sswitch_event_id == entry->event_id)
                            || (ctx->swaking_event_id == entry->event_id);
    
    // Configuration access here.
    bool is_config_allowed = SlConfig::get_instance().is_event_allowed(entry);

    bool is_visible_event = entry->visible
                            & kshark_filter_masks::KS_EVENT_VIEW_FILTER_MASK;
    bool is_visible_graph = entry->visible
                            & kshark_filter_masks::KS_GRAPH_VIEW_FILTER_MASK;
    
    return correct_event_id && is_config_allowed
           && is_visible_event && is_visible_graph;
}

#ifndef _UNMODIFIED_KSHARK // Task colors
/**
 * @brief Returns either a default color or one present in the
 * color table of the main window's GL Widget based on Process ID of a task.
 * 
 * @param task_pid: task PID to index the task color table.
 * @param default_color: color to be used in case we fail in finding a color
 * in the colortable.
 * 
 * @returns Default color or one present in the main window's GL Widget
 * color table.
 *
 * @note It is dependent on the configuration 'SlConfig' singleton.
*/
static KsPlot::Color _get_task_color(int32_t task_pid, KsPlot::Color default_color) {
    // Configuration access here.
    const KsPlot::ColorTable& task_colors =
        SlConfig::get_instance().main_w_ptr->graphPtr()->glPtr()->getPidColors();
    bool task_color_exists = static_cast<bool>(task_colors.count(task_pid));

    KsPlot::Color triangle_color = (task_color_exists) ?
        task_colors.at(task_pid) : default_color;

    return triangle_color;
}
#endif

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
 * 
 * @note It is dependent on the configuration 'SlConfig' singleton.
*/
static SlTriangleButton* _make_sl_button(std::vector<const KsPlot::Graph*> graph,
                                         std::vector<int> bin,
                                         std::vector<kshark_data_field_int64*> data,
                                         KsPlot::Color col, float) {
    // Constants
    constexpr int32_t BUTTON_TEXT_OFFSET = 14;
    const std::string STACK_BUTTON_TEXT = "STACK";

    // Configuration access here.
    const SlConfig& cfg = SlConfig::get_instance();

    kshark_entry* event_entry = data[0]->entry;
    const kshark_entry* kstack_entry = (const kshark_entry*)(data[0]->field);

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

   constexpr int32_t TRIANGLE_HALFWIDTH = 24;
   constexpr int32_t TRIANGLE_HEIGHT = 27;
   
   // Triangle points (0-a, 1-b, 2-c)
    KsPlot::Point a {x - TRIANGLE_HALFWIDTH, y - TRIANGLE_HEIGHT};
    KsPlot::Point b {x + TRIANGLE_HALFWIDTH, y - TRIANGLE_HEIGHT};
    KsPlot::Point c {x, y - 2};

    // Outer triangle
    auto inner_triangle = KsPlot::Triangle();
    inner_triangle.setPoint(0, a);
    inner_triangle.setPoint(1, b);
    inner_triangle.setPoint(2, c);

    inner_triangle._color = col;
#ifndef _UNMODIFIED_KSHARK // Task colors
    // Colors are a bit wonky with sched_switch events. Using the function
    // makes it consistent across the board.
    if (cfg.get_use_task_colors()) {
        int entry_pid = (event_entry->visible & KS_PLUGIN_UNTOUCHED_MASK) ?
            event_entry->pid :
            // "Emergency get" if some plugins messed around with the entry before
            kshark_get_pid(event_entry);

        inner_triangle._color = _get_task_color(entry_pid, col);
    }
#endif

    // Inner triangle
    auto back_triangle = KsPlot::Triangle(inner_triangle);
    // Configuration access here.
    back_triangle._color = cfg.get_button_outline_col();
    back_triangle.setFill(false);

    // Text coords
    int text_x = x - BUTTON_TEXT_OFFSET;
    int text_y = y - BUTTON_TEXT_OFFSET - 2;

    // Colors
    float bg_intensity = _get_color_intensity(inner_triangle._color);
    auto text_color = _black_or_white_text(bg_intensity);

    // Final object initialitations
    auto text = KsPlot::TextBox(get_font_ptr(), STACK_BUTTON_TEXT, text_color,
                                KsPlot::Point{text_x, text_y});

    auto sl_button = new SlTriangleButton(event_entry, kstack_entry, back_triangle,
                                          inner_triangle, text);

    return sl_button;
}

/**
 * @brief Function wrapper for drawing Stacklook objects on the plot.
 * 
 * @param argv: The C++ arguments of the drawing function of the plugin
 * @param dc: Input location for the container of the event's data
 * @param check_func: Check function used to select events from data container
 * @param make_button: Function which specifies what will be drawn and how
 *
 * @note It is dependent on the configuration 'SlConfig' singleton.
*/
static void _draw_stacklook_buttons(KsCppArgV* argv, 
                                    kshark_data_container* dc,
                                    IsApplicableFunc check_func,
                                    pluginShapeFunc make_button) {
    // -1 means default size
    // The default color of buttons will hopefully be overriden when
    // the button's entry's task PID is found.
    
    // Configuration access here.
    eventFieldPlotMin(argv, dc, check_func, make_button,
                      SlConfig::get_instance().get_default_btn_col(),
                      -1);
}

/**
 * @brief Loads values into the configuration windows from
 * the configuration object and shows the window afterwards.
 */
static void config_show([[maybe_unused]] KsMainWindow*) {
    cfg_window->load_cfg_values();
    cfg_window->show();
}

/**
 * @brief To be called only once per stream load. Stores kernel
 * stack entry pointers to the field of Stacklook-relevant
 * entries in the container in the argument.
 * 
 * @param dct Data container of Stacklook-relevant entries
 * @return True if any kernel stack entry was found, false
 * otherwise.
 */
static bool search_for_kstacks(const kshark_data_container* dct) {
    if (dct == nullptr || dct->size == 0)
        return false;
    
    bool found_at_least_one = false;

    for (ssize_t i = 0; i < dct->size; ++i) {
        kshark_data_field_int64* sl_relevant = dct->data[i];
        const kshark_entry* kstack_entry = get_kstack_entry(sl_relevant->entry);
        if (kstack_entry != nullptr) {
            sl_relevant->field = (int64_t)(kstack_entry);
            found_at_least_one = true;
        }
    }

    return found_at_least_one;
}

// #########################################################################

// Functions defined in the C header

/**
 * @brief Finds the `ftrace/kernel_stack` event entry if it was
 * recorded in the trace and is directly after the event entry on
 * the same CPU and belongs to the same task.
 * 
 * @param kstack_owner Entry, whose kernel stack trace we want to find.
 * @return Pointer to the `ftrace/kernel_stack` event entry if it was
 * found, nullptr otherwise (or if there was an error in data access).
 */
const struct kshark_entry* get_kstack_entry(const struct kshark_entry* kstack_owner) {
    const kshark_entry* kstack_entry = kstack_owner;
    
    if (kstack_entry == nullptr)
        return nullptr;

    plugin_stacklook_ctx* ctx = __get_context(kstack_owner->stream_id);
    
    if (ctx == nullptr)
        return nullptr;

    int relevant_owner_pid = (kstack_owner->visible & KS_PLUGIN_UNTOUCHED_MASK) ?
        kstack_owner->pid :
        // "Emergency get" if some plugins messed around with the entry before
        kshark_get_pid(kstack_owner);
    bool is_kstack = (kstack_entry->event_id == ctx->kstack_event_id);
    bool is_correct_task = (relevant_owner_pid == kstack_entry->pid);
    
    // This loop will usually stop either after one or two iterations.
    // This will be the case unless some plugin aggressively reorders
    // and changes innards of entries.
    while (!(is_kstack && is_correct_task)) {
        // Move onto next entry on the same CPU
        // Kernelstack trace will be on the same CPU as the event
        // directly after which it is made.
        kstack_entry = kstack_entry->next;
        if (kstack_entry == nullptr)
            return nullptr;
        // Update conditions
        is_kstack = (kstack_entry->event_id == ctx->kstack_event_id);
        is_correct_task = (relevant_owner_pid == kstack_entry->pid);
    }

    return kstack_entry;
}

/**
 * @brief Plugin's draw function.
 *
 * @param argv_c: a C pointer to be converted to KsCppArgV
 * @param sd: data stream identifier
 * @param val: process or CPU ID value
 * @param draw_action: draw action identifier
 * 
 * @note It is dependent on the configuration 'SlConfig' singleton.
*/
void draw_stacklook_objects(struct kshark_cpp_argv* argv_c, int sd,
                            int val, int draw_action) {
    KsCppArgV* argVCpp KS_ARGV_TO_CPP(argv_c);
    plugin_stacklook_ctx* ctx = __get_context(sd);
    kshark_data_container* plugin_data;

    // Configuration access here.
    const SlConfig& config = SlConfig::get_instance();
    const int32_t HISTO_ENTRIES_LIMIT = config.get_histo_limit();
    
    // Don't draw if not drawing tasks or CPUs.
    if (!(draw_action == KSHARK_CPU_DRAW 
        || draw_action == KSHARK_TASK_DRAW)) {
        return;
    }
    
    // Don't draw with too many bins (configurable zoom-in indicator).
    if (argVCpp->_histo->tot_count > HISTO_ENTRIES_LIMIT) {
        return;
    }

    plugin_data = ctx->collected_events;
    if (!plugin_data) {
        // Couldn't get the context container (any reason)
        return;
    }

    // Search for kernelstack events once per stream on load.
    if (!ctx->searched_for_kstacks) {
        // Update context variable to indicate whether any
        // kernel stack entry exists.
        ctx->kstacks_exist = search_for_kstacks(plugin_data);
        ctx->searched_for_kstacks = true;
    }

    if (!ctx->kstacks_exist) {
        // No reason to draw anything, if no kernelstacks are present in
        // the trace.
        return;
    }

    IsApplicableFunc check_func;
    
    if (draw_action == KSHARK_TASK_DRAW) {
        check_func = [=] (kshark_data_container* data_c, ssize_t t) {
            kshark_entry* entry = data_c->data[t]->entry;
            const kshark_entry* kstack_ptr = (kshark_entry*)(data_c->data[t]->field);
            if (!entry)
                return false;
            bool correct_pid = (entry->pid == val);
            return _check_function_general(entry, kstack_ptr, ctx) && correct_pid;
        };
        
    } else if (draw_action == KSHARK_CPU_DRAW) {
        check_func = [=] (kshark_data_container* data_c, ssize_t t) {
            kshark_entry* entry = data_c->data[t]->entry;
            const kshark_entry* kstack_ptr = (kshark_entry*)(data_c->data[t]->field);
            if (!entry)
                return false;
            bool correct_cpu = (entry->cpu == val);
            return _check_function_general(entry, kstack_ptr, ctx) && correct_cpu;
        };
    }

    _draw_stacklook_buttons(argVCpp, plugin_data, check_func, _make_sl_button);
}

/**
 * @brief Give the plugin a pointer to KernalShark's main window to allow
 * GUI manipulation and menu creation.
 * 
 * This is where plugin menu is made and initialized first. It's lifetime
 * is managed by KernelShark afterward.
 * 
 * @param gui_ptr: Pointer to the main KernelShark window.
 * 
 * @returns Pointer to the configuration menu instance.
 * 
 * @note It is dependent on the configuration 'SlConfig' singleton.
*/
__hidden void* plugin_set_gui_ptr(void* gui_ptr) {
    KsMainWindow* main_w = static_cast<KsMainWindow*>(gui_ptr);
    // Configuration access here.
    SlConfig::main_w_ptr = main_w;

    if (cfg_window == nullptr) {
        cfg_window = new SlConfigWindow();
    }

    QString menu("Tools/Stacklook Configuration");
    main_w->addPluginMenu(menu, config_show);

    return cfg_window;
}