// C
#include <stdlib.h>
#include <string.h>

// trace-cmd
#include <trace-cmd.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"
#include "libkshark-tepdata.h"

static struct tep_handle* tep;
static int sched_switch_id;

static void process_sched_switch(struct kshark_data_stream* stream,
                                 void* rec, struct kshark_entry* entry)
{
    /** Capture and analyze tep_record data. **/
    struct tep_record* record = (struct tep_record*)rec;
    struct tep_event* event = tep_find_event_by_record(tep, record);
    
    /** Fail if event couldn't be loaded. **/
    if (!event) {
        printf("\n\n Failed to find event by record in Couplebreaker. \n\n");
        return;
    }

    /* Extract original event fields */
    struct tep_format_field* prev_pid_field = tep_find_field(event, "prev_pid");
    struct tep_format_field* next_pid_field = tep_find_field(event, "next_pid");

    unsigned long long prev_pid_val;
    unsigned long long next_pid_val;
    
    int success_prev = tep_read_number_field(prev_pid_field, record->data, &prev_pid_val);
    int success_next = tep_read_number_field(next_pid_field, record->data, &next_pid_val);


    /* Create synthetic event for sched_switch[target] */
    

    /* Change the PID ownership for target */
    
    

    /* Add synthetic event to visualization */
    
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
    if (!kshark_is_tep(stream)) {
        __close(stream->stream_id);
        return 0;
    }

    tep = kshark_get_tep(stream);
    sched_switch_id = kshark_find_event_id(stream, "sched/sched_switch");

    kshark_register_event_handler(stream, sched_switch_id, process_sched_switch);
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
    kshark_unregister_event_handler(stream, sched_switch_id, process_sched_switch);
    
    sched_switch_id = -1;
    tep = NULL;

    __close(stream->stream_id);

    return 1;
}