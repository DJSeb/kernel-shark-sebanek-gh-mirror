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
#include "SlDetailedView.hpp"
#include "SlButton.hpp"
#include "SlConfig.hpp"
#include "SlPrevState.hpp"

#ifndef _NO_NAPS
#include "SlNapRectangle.hpp"
#endif

// #########################################################################
// Usings

#ifndef _NO_NAPS
/**
 * @brief Type to be used by the PREV_STATE_TO_COLOR constant.
 */
using prev_state_colors_t = std::map<char, KsPlot::Color>;
#endif

// #########################################################################
// Constants
#ifndef _NO_NAPS
/**
 * @brief Constant map of assigned colors to prev_state abbreviations.
 */
static const prev_state_colors_t PREV_STATE_TO_COLOR {
    {'S', {0, 0, 255}}, // Blue
    {'D', {255, 0, 0}}, // Red
    {'R', {0, 255, 0}}, // Green
    {'I', {255, 255, 0}}, // Yellow
    {'T', {0, 255, 255}}, // Cyan
    {'t', {139, 69, 19}}, // Brown
    {'X', {255, 0, 255}}, // Magenta
    {'Z', {128, 0, 128}}, // Purple
    {'P', {255, 165, 0}} // Orange
};
#endif

// #########################################################################
// Static variables
/**
 * @brief Static pointer to the configuration window.
 */
static SlConfigWindow* cfg_window;

/**
 * @brief Color table of the tasks so that the Stacklook buttons look
 * prettier.
*/
static KsPlot::ColorTable task_colors{};

/**
 * @brief Indicates whether the colortable has already been loaded or not.
*/
static bool loaded_colors;

// #########################################################################
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
static bool _check_function_general(const kshark_entry* entry,
                                    const plugin_stacklook_ctx* ctx) {
    if (!entry) return false;
    bool correct_event_id = (ctx->sswitch_event_id == entry->event_id)
                            || (ctx->swake_event_id == entry->event_id);
    
    bool is_config_allowed = SlConfig::get_instance().is_event_allowed(entry);
    
    const kshark_entry* kstack_entry = _get_kstack_entry(entry);
    // We hate segfaults in this house - so they are banned!
    if (!kstack_entry) return false;
    int entry_evt_id = kshark_get_event_id(kstack_entry);
    bool has_next_kernelstack = (ctx->kstack_event_id == entry_evt_id);

    bool is_visible_event = entry->visible
                            & kshark_filter_masks::KS_EVENT_VIEW_FILTER_MASK;
    bool is_visible_graph = entry->visible
                            & kshark_filter_masks::KS_GRAPH_VIEW_FILTER_MASK;
    
    return correct_event_id && is_config_allowed && has_next_kernelstack
           && is_visible_event && is_visible_graph;
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
    bool task_color_exists = static_cast<bool>(task_colors.count(task_pid));

    KsPlot::Color triangle_color = (task_color_exists) ?
        task_colors.at(task_pid) : default_color;

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

#ifndef _NO_NAPS
/**
 * @brief General function for checking whether to show a nap rectangle
 * in the plot.
 * 
 * @param entry: pointer to an event entry which could be a part of the
 * nap rectangle 
 * 
 * @returns True if entry is visible (and pointer wasn't null).
 */
static bool _nap_rect_check_function_general(const kshark_entry* entry) {
    if (!entry) return false;
    bool is_visible_event = entry->visible
                            & kshark_filter_masks::KS_EVENT_VIEW_FILTER_MASK;
    bool is_visible_graph = entry->visible
                            & kshark_filter_masks::KS_GRAPH_VIEW_FILTER_MASK;
    return is_visible_event && is_visible_graph;
}

/**
 * @brief Creates a nap rectangle to be displayed on the plot.
 * 
 * @param graph KernelShark graphs
 * @param bin KernelShark bins
 * @param data: container of two entries between which to draw the nap
 * rectangle
 * 
 * @returns Pointer to the heap-created nap rectangle.
 */
static SlNapRectangle* _make_sl_nap_rect(std::vector<const KsPlot::Graph*> graph,
                                             std::vector<int> bin,
                                             std::vector<kshark_data_field_int64*> data,
                                             KsPlot::Color, float) {
    kshark_entry* switch_entry = data[0]->entry;
    kshark_entry* wakeup_entry = data[1]->entry;

    // Base points and such
    KsPlot::Point start_base_point = graph[0]->bin(bin[0])._val;
    KsPlot::Point end_base_point = graph[0]->bin(bin[1])._val;
    int height = 12;
    int height_offset = 15;

    auto point_0 = KsPlot::Point{start_base_point.x(),
                                 start_base_point.y() - height_offset - height};
    auto point_1 = KsPlot::Point{start_base_point.x(),
                                 start_base_point.y() - height_offset};
    auto point_3 = KsPlot::Point{end_base_point.x(),
                                 end_base_point.y() - height_offset - height};
    auto point_2 = KsPlot::Point{end_base_point.x(),
                                 end_base_point.y() - height_offset};

    // Create the rectangle and color it    
    KsPlot::Rectangle rect;
    rect.setFill(true);
    rect._color = PREV_STATE_TO_COLOR.at(get_switch_prev_state(switch_entry)[0]);

    rect.setPoint(0, point_0);
    rect.setPoint(1, point_1);
    rect.setPoint(2, point_2);
    rect.setPoint(3, point_3);

    // Prepare outline color
    const KsPlot::Color outline_col = _get_color(switch_entry->pid,
                                      SlConfig::get_instance().get_default_btn_col());
    
    // Prepare text color
    float bg_intensity = _get_color_intensity(rect._color);
    const KsPlot::Color text_color = _black_or_white_text(bg_intensity);

    // Create the final nap rectangle and return it
    SlNapRectangle* nap_rect = new SlNapRectangle{switch_entry, wakeup_entry, rect,
                                                  outline_col, text_color};
    return nap_rect;
}

/**
 * @brief Initializes check functions for selecting nap rectangle
 * entries and draws nap rectangles via interval plotting.
 * 
 * @param argVCpp: the C++ arguments of the drawing function of the plugin
 * @param plugin_data: input location for the container of the event's data
 * @param ctx: pointer to the plugin's context
 * @param val: process or CPU ID value
 */
static void _draw_stacklook_nap_rectangles(KsCppArgV* argVCpp,
                                           kshark_data_container* plugin_data,
                                           const plugin_stacklook_ctx* ctx,
                                           int val) {
    IsApplicableFunc nap_rect_check_func_switch;
    IsApplicableFunc nap_rect_check_func_wakeup;

    nap_rect_check_func_switch = [=] (kshark_data_container* data_c, ssize_t t) {
        kshark_entry* entry = data_c->data[t]->entry;

        bool is_switch = (entry->event_id == ctx->sswitch_event_id);
        bool correct_pid = (entry->pid == val);
        return _nap_rect_check_function_general(entry) && is_switch && correct_pid;
    };

    nap_rect_check_func_wakeup = [=] (kshark_data_container* data_container, ssize_t i) {
        kshark_entry* entry = data_container->data[i]->entry;
        int64_t d_field = data_container->data[i]->field;

        bool is_wakeup = (entry->event_id == ctx->swake_event_id);
        bool correct_waking_pid = (d_field == val);
        return _nap_rect_check_function_general(entry) && is_wakeup && correct_waking_pid;
    };

    // Noteworthy thing here is that KernelShark will automatically
    // select a pair and NOT use members of it again - there won't be multiple
    // nap rectangles from one entry.
    // Hopefully this will never change.
    eventFieldIntervalPlot(argVCpp, plugin_data, nap_rect_check_func_switch,
                            plugin_data, nap_rect_check_func_wakeup,
                            _make_sl_nap_rect, {0, 0, 0}, -1);
}
#endif

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
static SlTriangleButton* _make_sl_button(std::vector<const KsPlot::Graph*> graph,
                                         std::vector<int> bin,
                                         std::vector<kshark_data_field_int64*> data,
                                         KsPlot::Color col, float) {
    // Constants
    constexpr int32_t BUTTON_TEXT_OFFSET = 14;
    const std::string STACK_BUTTON_TEXT = "STACK";

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

    // Colors are a bit wonky with sched_switch events. Using the function
    // makes it consistent across the board.
    inner_triangle._color = _get_color(kshark_get_pid(event_entry), col);

    // Inner triangle
    auto back_triangle = KsPlot::Triangle(inner_triangle);
    back_triangle._color = SlConfig::get_instance().get_button_outline_col();
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

    auto sl_button = new SlTriangleButton(event_entry, back_triangle,
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
*/
static void _draw_stacklook_buttons(KsCppArgV* argv, 
                                    kshark_data_container* dc,
                                    IsApplicableFunc check_func,
                                    pluginShapeFunc make_button) {
    // -1 means default size
    // The default color of buttons will hopefully be overriden when
    // the button's entry's task PID is found.
    
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

// Functions defined in the C header

/**
 * @brief Predicate function to check that the KernelShark entry is indeed kernel's
 * stack trace entry.
 * 
 * @param entry: entry whose event ID is checked.
 * 
 * @returns True if the entry is a kernel stack entry, false otherwise.
*/
bool is_kstack(const struct kshark_entry* entry) {
    if (entry == nullptr) {
        return false;
    }
    
    plugin_stacklook_ctx* ctx = __get_context(entry->stream_id);

    /** NOTE: This could be a bit more rigorous: check the event ID, check the task and maybe even the
     * top of the stack for confirmation it is indeed a stacktrace of the kernel for the switch or wakeup
     * event.
     * 
     * However, since the stacktrace is done immediately after every event when using trace-cmd's
     * '-T' option, events are always sorted so that stacktraces appear right after their events,
     * so this checking would be redundant.
     * 
     * Of course, if data are manipulated so that they aren't sorted by time, this approach is insufficent.
    */

    return entry->event_id == ctx->kstack_event_id;
}

//SL_TODO: documentation
const struct kshark_entry* get_kstack_entry(
    const struct kshark_entry* kstack_owner) {
    const kshark_entry* kstack_entry = kstack_owner;
    while (!(is_kstack(kstack_entry) &&
        kstack_owner->pid == kstack_entry->pid))
    {
        kstack_entry = kstack_entry->next;
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
*/
void draw_stacklook_objects(struct kshark_cpp_argv* argv_c, int sd,
                            int val, int draw_action) {
    KsCppArgV* argVCpp KS_ARGV_TO_CPP(argv_c);
    plugin_stacklook_ctx* ctx = __get_context(sd);
    kshark_data_container* plugin_data;

    // Config data
    const SlConfig& config = SlConfig::get_instance();
    const int32_t HISTO_ENTRIES_LIMIT = config.get_histo_limit();
    
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

    plugin_data = ctx->collected_events;
    // Couldn't get the context container (any reason)
    if (!plugin_data) {
        return;
    }

    _load_current_colortable();

    IsApplicableFunc check_func;
    
    if (draw_action == KSHARK_TASK_DRAW) {
        check_func = [=] (kshark_data_container* data_c, ssize_t t) {
            kshark_entry* entry = data_c->data[t]->entry;
            if (!entry) return false;
            bool correct_pid = (entry->pid == val);
            return _check_function_general(entry, ctx) && correct_pid;
        };
        
    } else if (draw_action == KSHARK_CPU_DRAW) {
        check_func = [=] (kshark_data_container* data_c, ssize_t t) {
            kshark_entry* entry = data_c->data[t]->entry;
            if (!entry) return false;
            bool correct_cpu = (data_c->data[t]->entry->cpu == val);
            return _check_function_general(entry, ctx) && correct_cpu;
        };
    }

    _draw_stacklook_buttons(argVCpp, plugin_data, check_func, _make_sl_button);

#ifndef _NO_NAPS
    // If the user wants to draw nap rectangles, do so
    if (SlConfig::get_instance().get_draw_naps()
        && draw_action == KSHARK_TASK_DRAW) {
        _draw_stacklook_nap_rectangles(argVCpp, plugin_data, ctx, val);
    }
#endif
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
*/
__hidden void* plugin_set_gui_ptr(void* gui_ptr) {
    KsMainWindow* main_w = static_cast<KsMainWindow*>(gui_ptr);
    SlConfig::main_w_ptr = main_w;

    if (cfg_window == nullptr) {
        cfg_window = new SlConfigWindow();
    }

    QString menu("Tools/Stacklook Configuration");
    main_w->addPluginMenu(menu, config_show);

    return cfg_window;
}

/**
 * @brief Deinitializes the task colors colortable, making sure it'll
 * be reloaded on new trace file load.
*/
void deinit_task_colors() {
    task_colors.clear();
    loaded_colors = false;
}