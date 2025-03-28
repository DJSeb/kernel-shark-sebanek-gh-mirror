/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

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
#include "libkshark-tepdata.h"

// Plugin header
#include "stacklook.h"

// Static variables

///
/// @brief Font used by KernelShark when plotting.
static struct ksplot_font font;

/// @brief Font used by the plugin when highlighting
/// harder to see text.
static struct ksplot_font bold_font;

///
/// @brief Path to font file.
static char* font_file = NULL;

///
/// @brief Path to the bold font file.
static char* bold_font_path = NULL;

///
/// @brief Integer ID of the `sched/sched_switch` event.
static int sched_switch_id;

///
/// @brief Integer ID of the `ftrace/kernel_stack` event.
static int kstack_id;

///
/// @brief Integer ID of the `sched/sched_waking` event.
static int sched_wake_id;

// Header file definitions

/**
 * @brief Checks if the bold font is loaded. If it isn't loaded yet, it initializes it.
 * 
 * @note Font to be loaded is *FreeSansBold*. This shouldn't produce issues,
 * as KernelShark uses said font, but not bold. If it does produce an issue,
 * change `bold_font_path` to the font file you wish to use.
 * 
 * @returns True if font is loaded, false otherwise.
 */
struct ksplot_font* get_bold_font_ptr() {
    if (!ksplot_font_is_loaded(&bold_font)) {
        ksplot_init_font(&bold_font, FONT_SIZE + 2, bold_font_path);
    }
    
    return &bold_font;
}

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

	kshark_free_data_container(sl_ctx->collected_events);

    sl_ctx->sswitch_event_id = -1;
    sl_ctx->kstack_event_id = -1;
    sl_ctx->swaking_event_id = -1;
}

/// @cond Doxygen_Suppress
// KernelShark-provided magic that will define the most basic
// plugin context functionality - init, freeing and getting context.
KS_DEFINE_PLUGIN_CONTEXT(struct plugin_stacklook_ctx, _sl_free_ctx);
/// @endcond

/**
 * @brief Selects supported events from unsorted trace file data
 * during plugin and data loading.
 * 
 * @note Effective during KShark's get_records function.
 * 
 * @param stream: KernelShark's data stream
 * @param rec: Tep record structure holding data collected by trace-cmd
 * @param entry: KernelShark entry to be processed
 * 
 * @note Supported events are: `sched/sched_switch`,
 *                             `sched/sched_waking`.
*/
static void _select_events(struct kshark_data_stream* stream,
                           [[maybe_unused]] void* rec, struct kshark_entry* entry) {

    struct plugin_stacklook_ctx* sl_ctx = __get_context(stream->stream_id);
    if (!sl_ctx) return;
    struct kshark_data_container* sl_ctx_collected_events = sl_ctx->collected_events;
    if (!sl_ctx_collected_events) return;

    const bool is_supported_event = entry->event_id == sched_switch_id ||
        entry->event_id == sched_wake_id;

    if (is_supported_event) {
        // -1 is nonsensical, but ensures the container isn't empty
        // It will be later replaced by a pointer to the kernel stack entry
        // if it is found.
        kshark_data_container_append(sl_ctx_collected_events, entry, (int64_t)-1);
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
    if (!font_file || !bold_font_path) {
        font_file = ksplot_find_font_file("FreeSans", "FreeSans");
        bold_font_path = ksplot_find_font_file("FreeSans", "FreeSansBold");
    }
    if (!font_file || !bold_font_path) return 0;
    
    kstack_id = kshark_find_event_id(stream, "ftrace/kernel_stack");
    if (kstack_id < 0) { //This isn't totally reliable though.
        printf("No ftrace/kernel_stack entries found, returning...\n");
        return 0;
    }

    struct plugin_stacklook_ctx* sl_ctx = __init(stream->stream_id);

    if (!sl_ctx) { // Guard against faulty context double free (sessions)
		__close(stream->stream_id);
		return 0;
	}

    sl_ctx->collected_events = kshark_init_data_container();

    sl_ctx->kstacks_exist = false;
    sl_ctx->searched_for_kstacks = false;
    // Do not be fooled, the kstack events might not be real, but their
    // event ID might be present in the trace file.
    sl_ctx->kstack_event_id = kstack_id;

    sched_switch_id = kshark_find_event_id(stream, "sched/sched_switch");
    sl_ctx->sswitch_event_id = sched_switch_id;

    sl_ctx->swaking_event_id = kshark_find_event_id(stream, "sched/sched_waking");
    sched_wake_id = sl_ctx->swaking_event_id;


    kshark_register_event_handler(stream, sched_switch_id, _select_events);
    kshark_register_event_handler(stream, sched_wake_id, _select_events);
    kshark_register_draw_handler(stream, draw_stacklook_objects);

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
        kshark_unregister_draw_handler(stream, draw_stacklook_objects);
        retval = 1;
    }

    if (stream->stream_id >= 0)
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
