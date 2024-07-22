/** TODO: Copyroght **/

#ifndef _KS_PLUGIN_STACKLOOK_H
#define _KS_PLUGIN_STACKLOOK_H

// C
#include <stdbool.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plot.h"
#include "libkshark-plugin.h"
#include "libkshark-model.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FONT_SIZE 8

typedef void* sorted_events_container_ptr;

struct plugin_stacklook_ctx {
    /** Numerical id of sched_switch event. **/
    int ss_event_id;
    /** Numerical id of kernel_stack event. **/
    int kstack_event_id;
    /** Data of stack-related events. **/
    struct kshark_data_container* stacks_data;

    /**
     * Sort schedswitch events by CPU & timestamp -> std::map
     * Sort ftrace_kstack by CPU & timestamp (there's many) -> std::map
     * Find next neighbour of sched_switch events in kstacks -> ???
     * Map these into stacks data. _________________________/
     * 
     * Problem is that I don't have a specific endpoint after the registered 
     * handlers to do this sorting.
     * To be honest, if I had, I might not even need to do the sorting at all.
     * I could just ask the sorted histogram.
    */

    // Sort containers (to be recasted in C++ code into sorted maps)
    sorted_events_container_ptr ss_sort;
    sorted_events_container_ptr kstack_sort;
};

KS_DECLARE_PLUGIN_CONTEXT_METHODS(struct plugin_stacklook_ctx)

// Global funcs
struct ksplot_font* get_font_ptr();
void draw_plot_buttons(struct kshark_cpp_argv* argv_c, int sd,
                       int val, int draw_action);

// Defined in C++
void* plugin_set_gui_ptr(void* gui_ptr);

void clean_opened_views();

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _KS_PLUGIN_STACKLOOK_H