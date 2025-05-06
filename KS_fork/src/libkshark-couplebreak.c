//NOTE: Changed here. (COUPLEBREAK) (2025-03-30)
/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    libkshark-couplebreak.c
 *  @brief   Interface for working with couplebreak events implementations.
*/

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// trace-cmd
#include <trace-cmd.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"
#include "libkshark-tepdata.h"
#include "libkshark-couplebreak.h"

/**
 * @brief Function serves as a mapping between the event Id of an
 * origin event (e.g. "sched/sched_switch") and a flag position
 * of a couplebreak event (e.g. "couplebreak/sched_switch[target]").
 * 
 * @param stream Input location for a Data stream pointer.
 * @param event_id The event Id of the origin event.
 * @return Position of the flag in the couplebreak_evts_flags bitmask or
 * -1 if the event Id is invalid (mapping doesn't exist).
 */
int couplebreak_origin_id_to_flag_pos(struct kshark_data_stream *stream,
	int event_id) {
	/*
	|0|0|0|0|0|0|0|0| ... |0|0|0|0|0|0|0|0| <- 32 bits
	Each bit is a flag. The position of the flag corresponds to the event Id,
	all according to the switch table below.
	*/

	int sched_switch_id = kshark_find_event_id(stream, "sched/sched_switch");
	int sched_waking_id = kshark_find_event_id(stream, "sched/sched_waking");

	if (event_id == sched_switch_id)
		return COUPLEBREAK_SSWITCH_FPOS;
	else if (event_id == sched_waking_id)
		return COUPLEBREAK_SWAKING_FPOS;
	else
		return -1;
}

/**
 * @brief Function serves as a mapping between the event Id of a
 * couplebreak vent and its flag position.
 * 
 * @param event_id The event Id of a couplebreak event.
 * @return Position of the flag in the couplebreak_evts_flags bitmask or
 * -1 if the event Id is invalid (mapping doesn't exist).
 */
int couplebreak_id_to_flag_pos(int event_id) {
	/*
	|0|0|0|0|0|0|0|0| ... |0|0|0|0|0|0|0|0| <- 32 bits
	Each bit is a flag. The position of the flag corresponds to the event Id,
	all according to the switch table below.
	*/

	switch (event_id) {
	case COUPLEBREAK_SST_ID:
		return COUPLEBREAK_SST_ID;
	case COUPLEBREAK_SWT_ID:
		return COUPLEBREAK_SWT_ID;
	default:
		return -1; 
	}
}

/**
 * @brief Function serves as a mapping between the flag position
 * of a couplebreak event (e.g. "couplebreak/sched_switch[target]")
 * and its event Id.
 * 
 * @param flag_pos The position of the flag in the couplebreak_evts_flags bitmask.
 * @return Event Id of the couplebreak event or -1 if the flag position
 * is invalid (mapping doesn't exist).
 */
int flag_pos_to_couplebreak_id(int flag_pos) {
    /*
	|0|0|0|0|0|0|0|0| ... |0|0|0|0|0|0|0|0| <- 32 bits
	Each bit is a flag. The position of the flag corresponds to the event Id,
	all according to the switch table below.
	*/

	switch (flag_pos) {
		case COUPLEBREAK_SSWITCH_FPOS:
			return COUPLEBREAK_SST_ID;
		case COUPLEBREAK_SWAKING_FPOS:
			return COUPLEBREAK_SWT_ID;
		default: // Represents a fault.
			return -1;
	}
}

/**
 * @brief Checks if the event Id is a couplebreak event.
 * 
 * @param event_id The event Id to check.
 * @return True if the event Id is a couplebreak event, false otherwise.
 */
bool is_couplebreak_event(int event_id) {
	const bool is_couplebreak_sst = (event_id == COUPLEBREAK_SST_ID);
	const bool is_couplebreak_swt = (event_id == COUPLEBREAK_SWT_ID);

	return (is_couplebreak_sst || is_couplebreak_swt);
}

/**
 * @brief Get the couplebreak event name as a string.
 * 
 * @param event_id The event Id of a couplebreak event.
 * @return Name of a couplebreak event or NULL if the event Id is invalid.
 */
char *get_couplebreak_event_name(int event_id) {
	const char *origin_evt_name = NULL;
	char *buffer;

	if (event_id == COUPLEBREAK_SST_ID)
		origin_evt_name = "sched_switch";
	else if (event_id == COUPLEBREAK_SWT_ID)
		origin_evt_name = "sched_waking";
	else
		return NULL; // Incorrect event Id, not a couplebreak event
	
	asprintf(&buffer, "couplebreak/%s[target]", origin_evt_name);

	return buffer;
}

/**
 * @brief Get the origin entry of a couplebreak entry.
 * 
 * @param entry Couplebreak entry to get origin from.
 * @return Pointer to the origin entry or NULL if the entry is invalid. 
 */
struct kshark_entry* couplebreak_get_origin(const struct kshark_entry *entry) {
	if (!entry)
		return NULL;

	struct kshark_entry *origin_entry = (struct kshark_entry *)entry->offset;
	if (!origin_entry)
		return NULL;

	return origin_entry;
}
// END of change
