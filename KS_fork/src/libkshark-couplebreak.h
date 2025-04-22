//NOTE: Changed here. (COUPLEBREAK) (2025-03-30)
/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    libkshark-couplebreak.h
 *  @brief   Interface for working with couplebreak events.
 */

#ifndef _KSHARK_COUPLEBRAK_H
#define _KSHARK_COUPLEBRAK_H

// KernelShark
#include "libkshark.h"

#ifdef __cplusplus
extern "C" {
#endif

//NOTE: Changed here. (COUPLEBREAK) (2025-03-30)
// Listed below are event Ids supported by couplebreak.
///@brief Event Id for "couplebreak/sched_switch[target]" event.
#define COUPLEBREAK_SST_ID -10000
///@brief Event Id for "couplebreak/sched_waking[target]" event.
#define COUPLEBREAK_SWT_ID -10001
// END of change

/* Below are constants used to index bits/flags in the
 * couplebreak_evts_flags bitmask of a stream.
*/
///@brief Bitmask flag position for the couplebreak/sched_switch[target] event.
#define COUPLEBREAK_SSWITCH_FPOS 0
///@brief Bitmask flag position for the couplebreak/sched_waking[target] event.
#define COUPLEBREAK_SWAKING_FPOS 1


struct kshark_entry* couplebreak_get_origin(const struct kshark_entry *entry);
int couplebreak_origin_id_to_flag_pos(struct kshark_data_stream *stream,
    int event_id);
int couplebreak_id_to_flag_pos(int event_id);
int flag_pos_to_couplebreak_id(int flag_pos);
bool is_couplebreak_event(int event_id);
char *get_couplebreak_event_name(int event_id);

#ifdef __cplusplus
}
#endif

#endif
// END of change