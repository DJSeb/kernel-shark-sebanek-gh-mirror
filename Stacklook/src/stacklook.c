/** TODO: Copyright? **/

/**
 * @file    stacklook.c
 * @brief   For integration with KernelShark. Contains definitions
 *          upon plugin loading and deloading, as well as handlers
 *          (un)registriations.
*/


// C
#include <stdc-predef.h>
#include <stdbool.h>
#include <stdio.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plot.h"
#include "libkshark-plugin.h"

// Plugin header
#include "stacklook.h"

// Static variables

///
/// @brief Font used by KernelShark when plotting.
static struct ksplot_font font;

///
/// @brief Path to font file.
static char* font_file = NULL;

///
/// @brief Integer ID of the `sched/sched_switch` event.
static int sched_switch_id;

///
/// @brief Integer ID of the `ftrace/kernel_stack` event.
static int kstack_id;

///
/// @brief Integer ID of the `sched/sched_wakeup` event.
static int sched_wake_id;

// Header file definitions

/**
 * @brief Get pointer to the font. Initializes the font
 * if this hasn't happened yet.
 * 
 * @returns Pointer to the font.
*/
struct ksplot_font* get_font_ptr() {
    if (!ksplot_font_is_loaded(&font)) {
        ksplot_init_font(&font, FONT_SIZE, font_file);
    }

    return &font;
}

// Context & plugin loading

/**
 * @brief Frees structures of the context and invalidates other number fields.
 * 
 * @param sl_ctx: pointer to plugin's context to be freed
*/ 
static void _sl_free_ctx(struct plugin_stacklook_ctx* sl_ctx)
{
	if (!sl_ctx) {
		return;
    }

    clean_opened_views(sl_ctx->cpp_views_container);

	kshark_free_data_container(sl_ctx->collected_events);

    sl_ctx->sswitch_event_id = -1;
    sl_ctx->kstack_event_id = -1;
    sl_ctx->swake_event_id = -1;
}

KS_DEFINE_PLUGIN_CONTEXT(struct plugin_stacklook_ctx, _sl_free_ctx);

/**
 * @brief Selects supported events from unsorted trace file data
 * during plugin loading.
 * 
 * @param stream: KernelShark's data stream
 * @param entry: KernelShark entry to be processed
 * 
 * @note Supported events are: `sched/sched_switch`,
 *                             `sched/sched_wakeup`.
*/
static void _select_events(struct kshark_data_stream* stream,
                           void*, struct kshark_entry* entry) {

    struct plugin_stacklook_ctx* sl_ctx = __get_context(stream->stream_id);
    struct kshark_data_container* sl_ctx_stack_data = sl_ctx->collected_events;

    if (entry->event_id == sched_switch_id ||
        entry->event_id == sched_wake_id) {
        /**
         * Add the event to the plugin's collected entries' container
         * -1 is a nonsensical value, but necessary so that the container
         * isn't considered empty.
        */
        kshark_data_container_append(sl_ctx_stack_data, entry, -1);
    }
}

/** 
 * @brief Initializes the plugin's context and registers handlers of the
 * plugin.
 * 
 * @param stream: KernelShark's data stream for which to initialize the
 * plugin.
 * 
 * @returns `0` if any error happened. `1` if initialization was successful.
*/
int KSHARK_PLOT_PLUGIN_INITIALIZER(struct kshark_data_stream* stream) {
    if (!font_file)
        font_file = ksplot_find_font_file("FreeSans", "FreeSans");
    if (!font_file)
        return 0;

    struct plugin_stacklook_ctx* sl_ctx = __init(stream->stream_id);

    if (!sl_ctx) {
		__close(stream->stream_id);
		return 0;
	}

    sl_ctx->collected_events = kshark_init_data_container();

    sched_switch_id = kshark_find_event_id(stream, "sched/sched_switch");
    sl_ctx->sswitch_event_id = sched_switch_id;

    kstack_id = kshark_find_event_id(stream, "ftrace/kernel_stack");
    sl_ctx->kstack_event_id = kstack_id;

    sched_wake_id = kshark_find_event_id(stream, "sched/sched_wakeup");
    sl_ctx->swake_event_id = sched_wake_id;

    sl_ctx->cpp_views_container = init_views();

    kshark_register_event_handler(stream, sched_switch_id, _select_events);
    kshark_register_event_handler(stream, sched_wake_id, _select_events);
    kshark_register_draw_handler(stream, draw_plot_buttons);

    return 1;
}

/**
 * @brief Deinitializes the plugin's context and unregisters handlers of the
 * plugin.
 * 
 * @param stream: KernelShark's data stream in which to deinitialize the
 * plugin.
 * 
 * @returns `0` if any error happened. `1` if deinitialization was successful.
*/
int KSHARK_PLOT_PLUGIN_DEINITIALIZER(struct kshark_data_stream* stream) {

    struct plugin_stacklook_ctx* sl_ctx = __get_context(stream->stream_id);

    int retval = 0;

    if (sl_ctx) {
        kshark_unregister_event_handler(stream, sched_switch_id, _select_events);
        kshark_unregister_event_handler(stream, sched_wake_id, _select_events);
        kshark_unregister_draw_handler(stream, draw_plot_buttons);
        retval = 1;
    }

    __close(stream->stream_id);

    return retval;
}

/**
 * @brief Initializes menu for the plugin and gives the plugin pointer
 * to KernelShark's main window.
 * 
 * @param gui_ptr: pointer to KernelShark's GUI, its main window.
 * 
 * @returns Pointer to KernelShark's main window.
*/
void* KSHARK_MENU_PLUGIN_INITIALIZER(void* gui_ptr) {
	return plugin_set_gui_ptr(gui_ptr);
}
