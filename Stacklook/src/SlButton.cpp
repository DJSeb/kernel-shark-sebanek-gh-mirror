
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
#include "SlDetailedView.hpp"
#include "SlButton.hpp"

/**
 * @brief Predicate function to check that the next KernelShark entry is indeed kernel's
 * stacktrace entry.
 * 
 * @param next_entry: entry whose event ID is checked.
*/
static bool _next_is_kstack(kshark_entry* next_entry) {
    plugin_stacklook_ctx* ctx = __get_context(next_entry->stream_id);
    return next_entry->event_id == ctx->kstack_event_id;
}

/**
 * @brief Simple function that returns the C-string of the info field of a KernelShark entry
 * next to the one in the argument or a nullptr on failing to fulfill the predicate in arguments.
 * @note Requires the entry in the argument to have its next field
 * initialized, otherwise it'd access nullptr.
 * 
 * @param entry: entry whose next neighbour's info we wish to get.
 * @param predicate: function which checks a KernelShark entry and returns true or false.
*/
static char* _get_info_of_next_event(kshark_entry* entry,
                                     std::function<bool(kshark_entry*)> predicate) {
    char* retval = nullptr;

    kshark_entry* next_entry = entry->next;

    if (predicate(next_entry)) {
        retval = kshark_get_info(next_entry);
    }

    return retval;
}

/**
 * @brief Function which calculates (absolute value of) the area of a trigon using
 * the shoelace formula (https://en.wikipedia.org/wiki/Shoelace_formula).
 * Order of points is irrelevant, as the formula works generally.
 * 
 * @param a: point A of the triangle.
 * @param b: point B of the triangle.
 * @param c: point C of the triangle.
*/
static constexpr double _trigon_area(const ksplot_point a,
                                     const ksplot_point b,
                                     const ksplot_point c) {
    return abs(((a.x * (b.y - c.y)) +
                (b.x * (c.y - a.y)) +
                (c.x * (a.y - b.y))) / 2.0);
}

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

void SlTriangleButton::_doubleClick() const {
    std::string window_text = "ERROR: No info on kernelstack!\n"
                              "Try zooming in and out and come back here again :)";
    char* window_labeltext = kshark_get_task(_event_entry);

    /** NOTE: This could be a bit more rigorous: check the event ID, check the task and maybe even the
     * top of the stack for confirmation it is indeed a stacktrace of the kernel for the switch or wakeup
     * event.
     * 
     * However, since the stacktrace is done immediately after every event when using trace-cmd's
     * '-T' option, events are always sorted so that stacktraces appear right after their events,
     * so this checking would be redundant.
    */

    char* kstack_string_ptr = _get_info_of_next_event(_event_entry, _next_is_kstack);

    if (kstack_string_ptr != nullptr) {
        window_text = kstack_string_ptr;
    }

    char* window_ctext = const_cast<char*>(window_text.c_str());
    auto new_view = SlDetailedView::make_new_view(window_labeltext, window_ctext);
    new_view->show();
    SlDetailedView::opened_views->push_back(new_view);
}

void SlTriangleButton::_draw(const KsPlot::Color&, float) const {
    _inner_triangle.draw(); // Draw the triangle insides first
    _outline_triangle.draw(); // Make the outline by overlaying another triangle
    _text.draw(); // Add text
}

static QString _prettify_stack_items(const std::string& to_prettify) {
    constexpr int LABEL_LIMIT = 44;
    std::size_t name_start = to_prettify.find("=> ", 0) + 3;
    std::size_t name_end = to_prettify.find(" (", name_start);
    auto num_of_chars = name_end - name_start;

    if (num_of_chars > LABEL_LIMIT) {
        return QString(to_prettify.substr(name_start, LABEL_LIMIT).append("...").c_str());
    }

    return QString(to_prettify.substr(name_start, num_of_chars).c_str()); 
}

static void _get_top_three_stack_items(char* stacktrace, QString out_array[]) {
    std::string trace_str{stacktrace};

    std::size_t str_pos = 0;
    for (uint8_t counter = 0; counter < 3; ++counter) {
        std::size_t content_start = trace_str.find('\n', str_pos) + 1;
        std::size_t content_end = trace_str.find('\n', content_start);
        auto num_of_chars = content_end - content_start;

        const std::string item_as_str = trace_str.substr(content_start, num_of_chars);

        out_array[counter] = _prettify_stack_items(item_as_str);

        str_pos = content_end + 1;
    }
}


/**
 * @brief Action on mouse move over the plugin's plot object event.
 * ... TODO:
*/
void SlTriangleButton::_mouseHover() const {
    char* kstack_string_ptr = _get_info_of_next_event(_event_entry, _next_is_kstack);
    QString top_three_items[3]{};
    _get_top_three_stack_items(kstack_string_ptr, top_three_items); 

    SlDetailedView::main_w_ptr->graphPtr()->setPreviewLabels(
        kshark_get_task(_event_entry),
        top_three_items[0],
        top_three_items[1],
        top_three_items[2],
        "..."
    );
}