/**
 * For internal logic.
*/

// C++
#include <vector>
#include <string>
#include <iostream>
#include <map>

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
#include "SlDetailedWindow.hpp"

// Static variables

/**
 * @brief Pointer to the main window to allow manipulation and
 * window dependency (i.e. closing the main window closes the others).
*/
static KsMainWindow* main_w_ptr;

/**
 * @brief Container for opened Stacklook windows. Allows more independent lifetime
 * and is easier to control their destruction.
*/
static std::vector<SlDetailedView*> opened_views{};

// Runtime constants

/**
 * @brief Text to appear on the triangle buttons in the plot.
*/
const static std::string STACK_BUTTON_TEXT = "STACK";

/**
 * @brief Function which calculates (absolute value of) the area of a trigon using
 * the shoelace formula (https://en.wikipedia.org/wiki/Shoelace_formula).
 * Order of points is irrelevant, as the formula works generally.
 * 
 * @param a: point A of the triangle.
 * @param b: point B of the triangle.
 * @param c: point C of the triangle.
*/
static constexpr double trigon_area(const ksplot_point a,
                                const ksplot_point b,
                                const ksplot_point c) {
    return abs(((a.x * (b.y - c.y)) +
                (b.x * (c.y - a.y)) +
                (c.x * (a.y - b.y))) / 2.0);
}

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
 * next to the one in the argument.
 * @note Requires the entry in the argument to have its next field
 * initialized, otherwise it'd access nullptr.
 * @note The next entry must fulfill the predicate function given as an argument.
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

// Classes

/**
 * @brief Special button class just for the plugin.
 * It itself is a triangle that contains another inner triangle
 * and a textbox. This is so that everything is drawn together
 * and logical structuring is retained.
*/
class SlTriangleButton : public KsPlot::PlotObject {
private:
    /**
     * @brief What event the button points at and gets data from for
     * the window.
    */
    kshark_entry* _switch_event;
    // Graphical
    /**
     * @brief Triangle which creates the outline of the button.
     * Black coloured and filled.
    */
    KsPlot::Triangle _outline_triangle;
    /**
     * @brief Triangle which forms the innardsof 
    */
    KsPlot::Triangle _inner_triangle;
    KsPlot::TextBox _text;
public:
    SlTriangleButton() : KsPlot::PlotObject() {}
    explicit SlTriangleButton(kshark_entry* switch_entry,
                              KsPlot::Triangle& outer,
                              KsPlot::Triangle& inner,
                              KsPlot::TextBox& text)
        : _switch_event(switch_entry), _outline_triangle(outer),
          _inner_triangle(inner), _text(text) {}

    double distance(int x, int y) const override {
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

        double triangle_area = trigon_area(a, b, c);
        double pbc_area = trigon_area(p, b, c);
        double apc_area = trigon_area(a, p, c);
        double abp_area = trigon_area(a, b, p);

        double p_areas_sum = pbc_area + apc_area + abp_area;

        return (triangle_area == p_areas_sum) ? 0
                : std::numeric_limits<double>::max();
    }
private:
    void _doubleClick() const override {
        std::string window_text = "ERROR: No info on kernelstack!\n"
                                  "Try zooming in and out and come back here again :)";
        char* window_labeltext = kshark_get_task(_switch_event);

        /** NOTE: This could be a bit more rigorous: check the event ID, check the task and maybe even the
         * top of the stack for confirmation it is indeed a stacktrace of the kernel for the switch event.
         * However, since the stacktrace is done immediately after every event when using trace-cmd's
         * '-T' option, events are always sorted so that stacktraces appear right after their events,
         * so this checking would be redundant.
        */

        char* kstack_string_ptr = _get_info_of_next_event(_switch_event, _next_is_kstack);

        if (kstack_string_ptr != nullptr) {
            window_text = kstack_string_ptr;
        }

        char* window_ctext = const_cast<char*>(window_text.c_str());
        auto new_view = new SlDetailedView(window_labeltext, window_ctext, main_w_ptr);
        new_view->show();
        opened_views.push_back(new_view);
    }

    void _draw(const KsPlot::Color&, float) const override {
        _inner_triangle.draw();
        _outline_triangle.draw();
        _text.draw();
    }
};

// Statics
static SlTriangleButton* makeSlButton(std::vector<const KsPlot::Graph*> graph,
                                      std::vector<int> bin,
                                      std::vector<kshark_data_field_int64*> data,
                                      KsPlot::Color col, float) {
    constexpr int32_t BUTTON_TEXT_OFFSET = 14;
    const KsPlot::Color TEXT_COLOR {0xFF, 0xFF, 0xFF};
    const KsPlot::Color OUTLINE_COLOR {0, 0, 0};

    // Base point
    int x = graph[0]->bin(bin[0])._val.x();
    int y = graph[0]->bin(bin[0])._val.y();
    
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
    inner_triangle._color = col;

    // Inner triangle
    auto back_triangle = KsPlot::Triangle(inner_triangle);
    back_triangle._color = OUTLINE_COLOR;
    back_triangle.setFill(false);

    int text_x = x - BUTTON_TEXT_OFFSET;
    int text_y = y - BUTTON_TEXT_OFFSET;
    auto text = KsPlot::TextBox(get_font_ptr(), STACK_BUTTON_TEXT, TEXT_COLOR,
                                KsPlot::Point{text_x, text_y});

    auto sl_button = new SlTriangleButton(data[0]->entry, back_triangle,
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
                                   IsApplicableFunc checkFunc) {    
    eventFieldPlotMin(argv, dc, checkFunc, makeSlButton,
                      {0x60, 0x69, 0x90}, -1);
}

// Globals

/**
 * @brief Function for correct destroying of opened stacklook
 * windows.
*/
void clean_opened_views() {
    for(auto view : opened_views) {
        if (view != nullptr) {
            delete view;
        }
    }
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
    if (!(draw_action == KSHARK_CPU_DRAW))
        return;
    // Don't draw with too many bins (configurable zoom-in indicator actually)
    if (argVCpp->_histo->tot_count > 200)
        return;

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

    _draw_stacklook_button(argVCpp, plugin_data, checkFunc);
}

/**
 * @brief Give the plugin a pointer to KS's main window to allow
 * GUI manipulation.
*/
__hidden void* plugin_set_gui_ptr(void* gui_ptr) {
    main_w_ptr = static_cast<KsMainWindow*>(gui_ptr);
    return nullptr;
}