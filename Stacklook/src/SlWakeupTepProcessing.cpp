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
#include "libkshark-tepdata.h"

// Plugin
#include "stacklook.h"

void wakeup_evt_tep_processing(struct plugin_stacklook_ctx* ctx, 
                               struct kshark_data_stream* stream,
                               void* rec,
                               struct kshark_entry* entry) {
    static tep_handle* tep = kshark_get_tep(stream);
    static tep_event* tep_wakeup;
    static tep_format_field* tep_waking_pid_field;

    static auto init_statics = [=](){
        bool wakeup_found = define_wakeup_event(tep, &tep_wakeup);

        if (wakeup_found) {
            tep_waking_pid_field = tep_find_any_field(tep_wakeup, "pid");
        }

        return true;
    }();

    tep_record* record = (struct tep_record*)rec;
    unsigned long long val; int ret;
    kshark_data_container* collected_events = ctx->collected_events;

    ret = tep_read_number_field(tep_waking_pid_field, record->data, &val);

    if (ret == 0) {
        entry->pid = (int32_t)val;
        kshark_data_container_append(collected_events, entry, val);
    } else {
        // Invalid
        kshark_data_container_append(collected_events, entry, (int64_t)-1);
    }
}