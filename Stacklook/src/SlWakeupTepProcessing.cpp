/** TODO: Copyright? */

/**
 * @file    SlWakeupTepProcessing.cpp
 * @brief
*/

// C++

// trace-cmd
#include <trace-cmd.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"

// Plugin
#include "stacklook.h"

void wakeup_evt_tep_processing(struct plugin_stacklook_ctx* ctx, 
                               struct kshark_data_stream* stream,
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