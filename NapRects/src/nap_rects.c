/** Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    nap_rects.c
 * @brief   For integration with KernelShark. Contains definitions
 *          upon plugin loading and deloading, as well as handlers
 *          (un)registriations.
*/


// C
#include <stdbool.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plot.h"
#include "libkshark-plugin.h"
#include "libkshark-tepdata.h"

// Plugin header
#include "nap_rects.h"

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
/// @brief Integer ID of the `sched/sched_waking` event.
static int waking_event_id;

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
static void _nr_free_ctx(struct plugin_nap_rects_context* nr_ctx)
{
	if (!nr_ctx) {
		return;
    }

	kshark_free_data_container(nr_ctx->collected_events);

    nr_ctx->sswitch_event_id = nr_ctx->waking_event_id = -1;
}

/// @cond Doxygen_Suppress
// KernelShark-provided magic that will define the most basic
// plugin context functionality - init, freeing and getting context.
KS_DEFINE_PLUGIN_CONTEXT(struct plugin_nap_rects_context , _nr_free_ctx);
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
 * However, if couplebreak feature is present, waking events detected change to
 * `couplebreak/sched_waking[target]`.
*/
static void _select_events(struct kshark_data_stream* stream,
    [[maybe_unused]] void* rec, struct kshark_entry* entry) {

    struct plugin_nap_rects_context *nr_ctx = __get_context(stream->stream_id);
    if (!nr_ctx) return;
    struct kshark_data_container* nr_ctx_collected_events = nr_ctx->collected_events;
    if (!nr_ctx_collected_events) return;

    if (entry->event_id == sched_switch_id) {
        /**
        * Add the event to the plugin's collected entries' container.
        * 
        * -1 is a nonsensical value, but necessary so that the container
        * isn't considered empty.
        */
        kshark_data_container_append(nr_ctx_collected_events, entry, (int64_t)-1);
    } else if (entry->event_id == waking_event_id) {
        waking_evt_tep_processing(nr_ctx, stream, rec, entry);
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
    
    struct plugin_nap_rects_context* nr_ctx = __init(stream->stream_id);

    if (!nr_ctx) {
		__close(stream->stream_id);
		return 0;
	}

    if (!kshark_is_tep(stream)) {
        __close(stream->stream_id);
        return 0;
    }

    nr_ctx->tep = kshark_get_tep(stream);
    bool waking_found = define_wakeup_event(nr_ctx->tep, &nr_ctx->tep_waking);

    if (waking_found) {
        nr_ctx->sched_waking_pid_field = tep_find_any_field(nr_ctx->tep_waking, "pid");
    }

    nr_ctx->collected_events = kshark_init_data_container();

    sched_switch_id = kshark_find_event_id(stream, "sched/sched_switch");
    nr_ctx->sswitch_event_id = sched_switch_id;

    waking_event_id = (!stream->couplebreak_on) ?
        kshark_find_event_id(stream, "sched/sched_waking") :
        kshark_find_event_id(stream, "couplebreak/sched_waking[target]");

    nr_ctx->waking_event_id = waking_event_id;

    kshark_register_event_handler(stream, sched_switch_id, _select_events);
    kshark_register_event_handler(stream, waking_event_id, _select_events);
    kshark_register_draw_handler(stream, draw_nap_rectangles);

    return 1;
}