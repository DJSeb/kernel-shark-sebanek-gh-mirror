/*
* This file is for integration with KernelShark.
* Inner logic is in file 'Stacklook.cpp'
*/

// C
#include <stdc-predef.h>
#include <stdio.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plot.h"
#include "libkshark-plugin.h"

// Plugin header
#include "stacklook.h"

struct ksplot_font font;
char *font_file = NULL;
int sched_switch_id;

/** Get pointer to the font **/
struct ksplot_font* get_font_ptr() {
    if (!ksplot_font_is_loaded(&font)) {
        ksplot_init_font(&font, FONT_SIZE, font_file);
    }

    return &font;
}

static void plugin_process(struct kshark_data_stream *stream,
                           void *rec, struct kshark_entry *entry) {
    
    //TODO: Probably gonna link ftrace/kernel_stacktraces here to the data...
    
    struct kshark_data_container* sl_ctx_stack_data = 
            __get_context(stream->stream_id)->stacks_data;

    if (entry->event_id == sched_switch_id) {
        kshark_data_container_append(sl_ctx_stack_data, entry, 1);
    }
}

/** Context & plugin loading **/

static void sl_free_ctx(struct plugin_stacklook_ctx* sl_ctx)
{
	if (!sl_ctx)
		return;

	kshark_free_data_container(sl_ctx->stacks_data);
    sl_ctx->ss_event_id = -1;
}

KS_DEFINE_PLUGIN_CONTEXT(struct plugin_stacklook_ctx, sl_free_ctx);

/** Load the plugin **/
int KSHARK_PLOT_PLUGIN_INITIALIZER(struct kshark_data_stream* stream) {
    
    struct plugin_stacklook_ctx* sl_ctx = __init(stream->stream_id);
    sl_ctx->stacks_data = kshark_init_data_container();

    if (!sl_ctx) {
		__close(stream->stream_id);
		return 0;
	}

    sched_switch_id = kshark_find_event_id(stream, "sched/sched_switch");
    sl_ctx->ss_event_id = sched_switch_id;

    if (!font_file)
        font_file = ksplot_find_font_file("FreeSans", "FreeSans");
    if (!font_file)
        return 0;

    kshark_register_event_handler(stream, sched_switch_id, plugin_process);
    kshark_register_draw_handler(stream, draw_plot_buttons);
    return 1;
}

/** Unload the plugin **/
int KSHARK_PLOT_PLUGIN_DEINITIALIZER(struct kshark_data_stream* stream) {

    struct plugin_stacklook_ctx* sl_ctx = __get_context(stream->stream_id);

    int retval = 0;

    if (sl_ctx) {
        kshark_unregister_event_handler(stream, sched_switch_id, plugin_process);
        kshark_unregister_draw_handler(stream, draw_plot_buttons);
        retval = 1;
    }

    __close(stream->stream_id);

    return retval;
}