/** Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    noboxes.c
 * @brief   Plugin for KernelShark to disable drawing of taskboxes from
 *          certain events' bins or ending at certain bins.
 * 
 * @note    Declarations in `noboxes.c`.
*/

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"
#include "libkshark-couplebreak.h"

// Plugin header
#include "noboxes.h"

/**
 * @brief Frees structures of the context and invalidates other number fields.
 * 
 * @param nb_ctx: Pointer to plugin's context to be freed
*/ 
static void nb_free_ctx(struct plugin_noboxes_context* nb_ctx)
{
    if (!nb_ctx) {
        return;
    }

    nb_ctx->kstack_event_id = -1;
}

/**
 * @brief Adjusts the visiblity of each chosen entry to disable
 * drawing of taskboxes beginning or ending at their bins. At the very least,
 * this will work most of the time.
 * 
 * @param stream KernelShark's data stream
 * @param rec Tep record structure holding data collected by trace-cmd - unnecessary here
 * @param entry KernelShark entry to be processed
 * @note Supported events are: `ftrace/kernel_stack`,
 *                             `couplebreak/sched_waking[target]`.
 * Couplebreak events are modified only if a modified KernelShark is used, which
 * has this modification.
 */
static void adjust_visiblity([[maybe_unused]] struct kshark_data_stream* stream,
    [[maybe_unused]] void* rec, struct kshark_entry* entry)
{
    entry->visible &= ~KS_DRAW_TASKBOX_MASK;
}

/// @cond Doxygen_Suppress
// KernelShark-provided magic that will define the most basic
// plugin context functionality - init, freeing and getting context.
KS_DEFINE_PLUGIN_CONTEXT(struct plugin_noboxes_context , nb_free_ctx);
/// @endcond

/** 
 * @brief Initializes the plugin's context and registers handlers of the
 * plugin. Currently supported events are all couplebreak events and
 * ftrace/kernel_stack events.
 * 
 * @param stream: KernelShark's data stream for which to initialize the
 * plugin
 * 
 * @returns `0` if any error happened. `1` if initialization was successful.
*/
int KSHARK_PLOT_PLUGIN_INITIALIZER(struct kshark_data_stream* stream) {
    struct plugin_noboxes_context* nb_ctx = __init(stream->stream_id);

    if (!nb_ctx) {
		__close(stream->stream_id);
		return 0;
	}

    nb_ctx->kstack_event_id = kshark_find_event_id(stream, "ftrace/kernel_stack");

    kshark_register_event_handler(stream, COUPLEBREAK_SWT_ID, adjust_visiblity);
    kshark_register_event_handler(stream, nb_ctx->kstack_event_id, adjust_visiblity);

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
    struct plugin_noboxes_context* nb_ctx = __get_context(stream->stream_id);

    int retval = 0;

    if (nb_ctx) {
        kshark_unregister_event_handler(stream, COUPLEBREAK_SWT_ID, adjust_visiblity);
        kshark_unregister_event_handler(stream, nb_ctx->kstack_event_id, adjust_visiblity);

        retval = 1;
    }

    if (stream->stream_id >= 0) {
        __close(stream->stream_id);
    }

    return retval;
}
