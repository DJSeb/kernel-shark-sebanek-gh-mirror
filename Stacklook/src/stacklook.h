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

/**
 * @brief Context for the plugin, basically structured
 * globally shared data.
*/
struct plugin_stacklook_ctx {
    /** Numerical id of sched_switch event. **/
    int sswitch_event_id;
    /** Numerical id of kernel_stack event. **/
    int kstack_event_id;
    /** Numerical id of sched_wakeup event. */
    int swake_event_id;

    /** Collected switch or wakeup events. **/
    struct kshark_data_container* collected_events;
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