/** TODO: Copyright? **/

/**
 * @file    SlButton.cpp
 * @brief   This file defines the class functionaliteis of Stacklook
 *          buttons as well as plot object reactions to mouse events.
*/

// C++
#include <string>

// KernelShark
#include "libkshark.h"
#include "libkshark-plot.h"
#include "KsMainWindow.hpp"
#include "KsPlotTools.hpp"

// Plugin headers
#include "stacklook.h"
#include "SlDetailedView.hpp"
#include "SlButton.hpp"
#include "SlConfig.hpp"
#include "SlPrevState.hpp"

// Static functions

/**
 * @brief Adds text to a Stacklook button of a sched_switch event - text shows
 * "(PREVS_STATE)", where PREV_STATE is a letter representing what state the task
 * was in before it was switched. Text box with the new text is then also placed under
 * the always-present "STACK" text.
 * 
 * @param event_entry: entry whose prev_state we take if it is a sched_switch
 * @param orig_text: text box for consistent style and position coordinates
 * @param triangle_position: position of the triangle button containing the text
 * 
 * @note: `triangle_position` is necessary, as KernelShark's text boxes can't
 * return their own position and cannot be utilized as such.
*/
static void _add_sched_switch_prev_state_text(const kshark_entry* event_entry,
                                              const KsPlot::TextBox& orig_text,
                                              const ksplot_point triangle_position) {
    plugin_stacklook_ctx* ctx = __get_context(event_entry->stream_id);
    if (event_entry->event_id == ctx->sswitch_event_id) {
        // Get the state indicator
        const std::string prev_state_base = get_switch_prev_state(event_entry);
        const std::string prev_state = "(" + prev_state_base + ")";
        
        // Create a text box
        KsPlot::TextBox other_text(orig_text);
        other_text.setText(prev_state);
        ksplot_font* font_to_use = get_bold_font_ptr();
        
        if (!font_to_use) {
            other_text.setFont(get_font_ptr());
        } else {
            other_text.setFont(font_to_use);
        }

        /* We take position from the southmost point of the triangle in the button.
           Following the button placement formula from `Stacklook.cpp`, +5 is added
           to reset to original Y from which we can appropriately adjust the text.
        */ 
        KsPlot::Point other_pos{triangle_position.x - 9, // Center on the X axis
                                triangle_position.y + 5 - 11}; // Adjust Y position
        other_text.setPos(other_pos);

        // Draw additional information
        other_text.draw();
    }
}

/**
 * @brief Gets text (specific info) to be displayed in the detailed window.
 * Has a dummy value if there is no specific info.
 * 
 * @param entry: which entry's specific info to get
 * 
 * @returns Const standard string with specific info or message informing
 * of there being no specific info.
 */
static const std::string _get_specific_info(const kshark_entry* entry) {
    plugin_stacklook_ctx* ctx = __get_context(entry->stream_id);
    static const std::map<int32_t, const char*> SPECIFIC_INFO_MAP{{
        {ctx->sswitch_event_id, "Task was in state "},
        {ctx->swake_event_id,   "Task has woken up."}
    }};
    static const char* NO_MAP_VAL{"No specific info for event."};

    const int32_t entry_event_id = kshark_get_event_id(entry);
    const bool not_mapped = (SPECIFIC_INFO_MAP.count(entry_event_id) == 0);
    std::string spec_info{
        (not_mapped) ? NO_MAP_VAL : SPECIFIC_INFO_MAP.at(entry_event_id)};
    
    if (entry_event_id == ctx->sswitch_event_id) {
        spec_info = spec_info.append(get_longer_prev_state(entry) + ".");
    }

    return spec_info;
}

/**
 * @brief Predicate function to check that the KernelShark entry is indeed kernel's
 * stack trace entry.
 * 
 * @param entry: entry whose event ID is checked.
 * 
 * @returns True if the entry is a kernel stack entry, false otherwise.
*/
static bool _is_kstack(kshark_entry* entry) {
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

/**
 * @brief Simple function that returns the C-string of the info field of a KernelShark entry
 * next to the one in the argument or a nullptr on failing to fulfill the predicate in arguments.
 * @note Requires the entry in the argument to have its next field
 * initialized, otherwise it'd access nullptr.
 * 
 * @param entry: entry whose next neighbour's info we wish to get.
 * @param predicate: function which checks a KernelShark entry and returns true or false.
 * 
 * @returns Textual form of next entry's Info field as a C-string.
*/
static const char* _get_info_of_next_event(kshark_entry* entry,
                                           std::function<bool(kshark_entry*)> predicate) {
    const char* retval = nullptr;

    if (entry == nullptr) {
        return retval;
    }

    kshark_entry* next_entry = entry->next;

    if (predicate(next_entry)) {
        retval = kshark_get_info(next_entry);
    }

    return retval;
}

/**
 * @brief Function which calculates (absolute value of) the area of a trigon using
 * the [shoelace formula](https://en.wikipedia.org/wiki/Shoelace_formula).
 * Order of points is irrelevant, as the formula works generally.
 * 
 * @param a: point A of the triangle.
 * @param b: point B of the triangle.
 * @param c: point C of the triangle.
 * 
 * @returns Area of the trigon.
*/
static constexpr double _trigon_area(const ksplot_point a,
                                     const ksplot_point b,
                                     const ksplot_point c) {
    return abs(((a.x * (b.y - c.y)) +
                (b.x * (c.y - a.y)) +
                (c.x * (a.y - b.y))) / 2.0);
}

#ifndef _UNMODIFIED_KSHARK
/**
 * @brief Cuts off the address and the arrow of the textual stack item.
 * If the item is too long, it is truncated to its 44 starting characters
 * and joined with ellipsis.
 *  
 * @param to_prettify: stack item in the style of trace-cmd, i.e.
 * `=> STACK_ITEM_NAME (ADDRESS)`
 * 
 * @returns Prettier text representation of the stack item.
*/
static QString _prettify_stack_item(const std::string& to_prettify) {
    // Pretty arbitrary, but it does produce nice results that aren't too long
    constexpr int LABEL_LIMIT = 44;

    const std::size_t name_start = to_prettify.find("=> ", 0) + 3;
    const std::size_t name_end = to_prettify.find(" (", name_start);
    const std::size_t num_of_chars = name_end - name_start;

    QString prettified;
    prettified = prettified.replace('\n', "");

    if (num_of_chars > LABEL_LIMIT) {
        prettified = QString(to_prettify.substr(name_start, LABEL_LIMIT).append("...").c_str());
    } else {
        prettified = QString(to_prettify.substr(name_start, num_of_chars).c_str());
    }

    return prettified;
}

/**
 * @brief Puts the top three stack items from the raw C-string stack trace
 * into a QString array after making them prettier.
 * 
 * @param stacktrace: C-string of the stack trace to be processed
 * @param evt_name: for determining stack offset of an entry
 * @param out_array: output array, top traces are eventually stored there
*/
static void _get_top_three_stack_items(const char* stacktrace,
                                       const std::string& evt_name,
                                       QString out_array[]) {
    std::string trace_str{stacktrace};

    const int16_t stack_offset = SlConfig::get_instance().get_stack_offset(evt_name);
    std::size_t str_pos = 0;

    for (int64_t counter = 0; counter < stack_offset; ++counter) {
        str_pos = trace_str.find("=>", str_pos);
        
        if (str_pos == std::string::npos) {
            return;
        }

        ++str_pos;
    }

    for (int64_t counter = 0; counter < 3; ++counter) {
        std::size_t content_start = trace_str.find("=>", str_pos);

        // We don't have enough entries
        if (content_start == std::string::npos) {
            break;
        }

        std::size_t content_end = trace_str.find("=>", content_start + 1);

        // If we found the last entry, take the rest of the string
        // and block further iteration.
        if (content_end == std::string::npos) {
            content_end = trace_str.length() - 1;
            counter = 3;
        }

        // For future iteration
        str_pos = content_end;

        auto num_of_chars = content_end - content_start;
        const std::string item_as_str = trace_str.substr(content_start, num_of_chars);
        out_array[counter] = _prettify_stack_item(item_as_str);

    }
}
#endif

// Class functions

/**
 * @brief Checks if a point at the `x`, `y` coordinates is
 * inside the button (specifically it's triangles).
 * In other words, this checks if the mouse is above
 * the button in the KernelShark plot.
 * 
 * @param x: horizontal coordinate of the point
 * @param y: vertical coordinate of the point
 * 
 * @returns `0` if the point is inside the button, maximum value for
 * standard C++ `double` type otherwise.
*/
double SlTriangleButton::distance(int x, int y) const {
    /* How it is with point ordering:
        0 ------ 1
         \     /
          \   /
           \ /
            2    
    */
    ksplot_point p {x, y};
    const ksplot_point a = *(_outline_triangle.point(0));
    const ksplot_point b = *(_outline_triangle.point(1));
    const ksplot_point c = *(_outline_triangle.point(2));

    double triangle_area = _trigon_area(a, b, c);
    double pbc_area = _trigon_area(p, b, c);
    double apc_area = _trigon_area(a, p, c);
    double abp_area = _trigon_area(a, b, p);

    double p_areas_sum = pbc_area + apc_area + abp_area;

    return (triangle_area == p_areas_sum) ? 0
            : std::numeric_limits<double>::max();
}

/**
 * @brief Action on mouse double clicking on the plugin's plot object event.
 * Spawns a window with the info field of the next entry after the entry the
 * button is displayed above.
 * 
 * @note In the case of this plugin, the next entry always has to be an `ftrace/kernel_stack`
 * event entry, with the field being the kernel's stack trace. Otherwise, an error message
 * will be shown in the window instead.
*/
void SlTriangleButton::_doubleClick() const {
    constexpr const char error_msg[] = "ERROR: No info field found!";                          
    const char* window_labeltext = kshark_get_task(_event_entry);
    const char* kstack_string_ptr = _get_info_of_next_event(_event_entry, _is_kstack);
    
    const char* window_text = (kstack_string_ptr != nullptr) ? 
        kstack_string_ptr : error_msg;
    const std::string specific_entry_info{_get_specific_info(_event_entry)};

    auto new_view = new SlDetailedView(window_labeltext, specific_entry_info.c_str(), window_text);
    new_view->show();
    SlDetailedView::opened_views->push_back(new_view);
}

/**
 * @brief Draws all the basic plot objects the button is composed of.
 * In order of drawing: the inner colorful triangle, the outline triangle
 * and the textbox.
 * 
 * @note Despite the signature, the other parameters are ignored.
*/
void SlTriangleButton::_draw(const KsPlot::Color&, float) const {
    _inner_triangle.draw(); // Draw the triangle insides first
    _outline_triangle.draw(); // Make the outline by overlaying another triangle
    _text.draw();

    _add_sched_switch_prev_state_text(_event_entry, _text,
                                      *(_inner_triangle.point(2)));
}

#ifndef _UNMODIFIED_KSHARK
/**
 * @brief Action on mouse moving over the plugin's plot object event.
 * Shows the task name of the stack trace and three top items in the stack.
 *
 * @warning WILL NOT WORK WITHOUT MODIFIED KERNELSHARK WITH SUPPORT
 * FOR MOUSE MOVE OVER PLOTOBJECT REACTIONS
*/
void SlTriangleButton::_mouseHover() const {
    const char* kstack_string_ptr = _get_info_of_next_event(_event_entry, _is_kstack);
    
    if (kstack_string_ptr != nullptr) {
        QString top_three_items[3]{"-", "-", "-"};
        _get_top_three_stack_items(kstack_string_ptr,
                                   kshark_get_event_name(_event_entry),
                                   top_three_items); 
        const char* last_item = (top_three_items[2] == "-") ? "(End of stack)" : "..."; 

        SlConfig::main_w_ptr->graphPtr()->setPreviewLabels(
            kshark_get_task(_event_entry),
            top_three_items[0],
            top_three_items[1],
            top_three_items[2],
            last_item
        );
    } else {
        SlConfig::main_w_ptr->graphPtr()->setPreviewLabels(
            kshark_get_task(_event_entry),
            "NO KERNEL STACK ENTRY FOUND"
        );
    }
}
#endif