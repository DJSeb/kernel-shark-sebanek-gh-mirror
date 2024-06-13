/*
* This file is for integration with KernelShark.
* Inner logic is in file 'Stacklook.cpp'
*/

// C
#include <stdc-predef.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plot.h"
#include "libkshark-plugin.h"

// Plugin header
#include "stacklook.h"

struct ksplot_font font;
char *font_file = NULL;
int ss_event_id;

/*Static hello world*/
static struct ksplot_font* get_font() {
    if (!ksplot_font_is_loaded(&font)) {
        ksplot_init_font(&font, FONT_SIZE, font_file);
    }

    return &font;
}

/** Callback **/
static void hello_world(struct kshark_cpp_argv* argv_c, int sd, int val, int draw_action) {
    struct ksplot_color text_col = {
        (uint8_t)128,
        (uint8_t)23,
        (uint8_t)168,
    };
    ksplot_print_text(get_font(), &text_col, 300, 100, "Hello, World!");
}

static void sl_free_ctx(struct plugin_stacklook_ctx* sl_ctx)
{
	if (!sl_ctx)
		return;

	kshark_free_data_container(sl_ctx->stacks_data);
}

KS_DEFINE_PLUGIN_CONTEXT(struct plugin_stacklook_ctx, sl_free_ctx);

/** Load the plugin **/
int KSHARK_PLOT_PLUGIN_INITIALIZER(struct kshark_data_stream* stream) {
    
    struct plugin_stacklook_ctx* sl_ctx = __init(stream->stream_id);
    if (!sl_ctx) {
		__close(stream->stream_id);
		return 0;
	}

    ss_event_id = kshark_find_event_id(stream, "sched/sched_switch");

    if (!font_file)
        font_file = ksplot_find_font_file("FreeSans", "FreeSans");
    if (!font_file)
        return 0;

    kshark_register_draw_handler(stream, hello_world);
    return 1;
}

/** Unload the plugin **/
int KSHARK_PLOT_PLUGIN_DEINITIALIZER(struct kshark_data_stream* stream) {

    struct plugin_stacklook_ctx* sl_ctx = __get_context(stream->stream_id);
    int retval = 0;

    if (sl_ctx) {
        kshark_unregister_draw_handler(stream, hello_world);
        retval = 1;
    }

    __close(stream->stream_id);

    return retval;
}