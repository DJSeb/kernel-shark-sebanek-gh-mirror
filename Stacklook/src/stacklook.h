#ifndef _KS_PLUGIN_STACKLOOK_H
#define _KS_PLUGIN_STACKLOOK_H

#include "libkshark.h"
#include "libkshark-plot.h"
#include "libkshark-plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FONT_SIZE 20

struct plugin_stacklook_ctx {
    /** Data of stack-related events. **/
    int ss_event_id;
    struct kshark_data_container* stacks_data;
};

KS_DECLARE_PLUGIN_CONTEXT_METHODS(struct plugin_stacklook_ctx)

// Global funcs
struct ksplot_font* get_font_ptr();
void draw_plot_buttons(struct kshark_cpp_argv* argv_c, int sd,
                           int val, int draw_action);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _KS_PLUGIN_STACKLOOK_H