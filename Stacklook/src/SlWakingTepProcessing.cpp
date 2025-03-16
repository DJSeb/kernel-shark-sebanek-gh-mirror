/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    SlWakingTepProcessing.cpp
 * @brief   Contains definition for processing event entries as tep
 *          records.
*/

// C++

// trace-cmd
#include <trace-cmd.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"

// Plugin
#include "stacklook.h"

/**
 * @brief Process sched_waking events as tep records during plugin loads,
 * adds PID of who is being awoken into the sched_waking entry's auxiliary
 * field - this pid will also become the owner of the event.
 * Else, it adds in invalid -1 (which isn't a valid PID).
 * 
 * @param ctx: pointer to plugin context
 * @param stream: pointer to the KernelShark stream with data
 * @param rec: pointer to the tep record of the entry
 * @param entry: pointer KernelShark event entry
 */
void waking_evt_tep_processing(struct plugin_stacklook_ctx* ctx, 
                               [[maybe_unused]] struct kshark_data_stream* stream,
                               void* rec,
                               struct kshark_entry* entry) {
    tep_record* record = (struct tep_record*)rec;
    unsigned long long val; int ret;
    kshark_data_container* collected_events = ctx->collected_events;

    ret = tep_read_number_field(ctx->sched_waking_pid_field, record->data, &val);

    if (ret == 0) {
        entry->pid = (int32_t)val;
        entry->visible &= ~KS_PLUGIN_UNTOUCHED_MASK;
        kshark_data_container_append(collected_events, entry, val);
    } else {
        // Invalid
        kshark_data_container_append(collected_events, entry, (int64_t)-1);
    }
}