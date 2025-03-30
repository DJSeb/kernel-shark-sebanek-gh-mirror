// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2019 VMware Inc, Yordan Karadzhov (VMware) <y.karadz@gmail.com>
 */

/**
 *  @file    libkshark-tepdata.c
 *  @brief   Interface for processing of FTRACE (trace-cmd) data.
 */


// C
#ifndef _GNU_SOURCE
/** Use GNU C Library. */
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// trace-cmd
#include <trace-cmd.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"
#include "libkshark-tepdata.h"

static __thread struct trace_seq seq;

static bool init_thread_seq(void)
{
	if (!seq.buffer)
		trace_seq_init(&seq);

	return seq.buffer != NULL;
}

/** Structure for handling all unique attributes of the FTRACE data. */
struct tepdata_handle {
	/** Page event used to parse the page. */
	struct tep_handle	*tep; /* MUST BE FIRST ENTRY */

	/** Input handle for the trace data file. */
	struct tracecmd_input	*input;

	/**
	 * Filter allowing sophisticated filtering based on the content of
	 * the event.
	 */
	struct tep_event_filter	*advanced_event_filter;

	/** The unique Id of the sched_switch_event event. */
	int sched_switch_event_id;

	/** Pointer to the sched_switch_next_field format descriptor. */
	struct tep_format_field	*sched_switch_next_field;

	/** Pointer to the sched_switch_comm_field format descriptor. */
	struct tep_format_field	*sched_switch_comm_field;
};

static inline int get_tepdate_handle(struct kshark_data_stream *stream,
				     struct tepdata_handle **handle)
{
	struct kshark_generic_stream_interface *interface;

	interface = stream->interface;
	if (!interface)
		return -EFAULT;

	*handle = interface->handle;

	return 0;
}

/** Get the Page event object used to parse the page. */
struct tep_handle *kshark_get_tep(struct kshark_data_stream *stream)
{
	struct tepdata_handle *tep_handle;
	int ret;

	ret = get_tepdate_handle(stream, &tep_handle);
	if (ret < 0)
		return NULL;

	return tep_handle->tep;
}

/** Get the input handle for the trace data file */
struct tracecmd_input *kshark_get_tep_input(struct kshark_data_stream *stream)
{
	struct tepdata_handle *tep_handle;
	int ret;

	ret = get_tepdate_handle(stream, &tep_handle);
	if (ret < 0)
		return NULL;

	return tep_handle->input;
}

static inline struct tep_event_filter *
get_adv_filter(struct kshark_data_stream *stream)
{
	struct tepdata_handle *tep_handle;
	int ret;

	ret = get_tepdate_handle(stream, &tep_handle);
	if (ret < 0)
		return NULL;

	return tep_handle->advanced_event_filter;
}

static int get_sched_switch_id(struct kshark_data_stream *stream)
{
	struct tepdata_handle *tep_handle;
	int ret;

	ret = get_tepdate_handle(stream, &tep_handle);
	if (ret < 0)
		return ret;

	return tep_handle->sched_switch_event_id;
}

static struct tep_format_field *get_sched_next(struct kshark_data_stream *stream)
{
	struct tepdata_handle *tep_handle;
	int ret;

	ret = get_tepdate_handle(stream, &tep_handle);
	if (ret < 0)
		return NULL;

	return tep_handle->sched_switch_next_field;
}

static struct tep_format_field *get_sched_comm(struct kshark_data_stream *stream)
{
	struct tepdata_handle *tep_handle;
	int ret;

	ret = get_tepdate_handle(stream, &tep_handle);
	if (ret < 0)
		return NULL;

	return tep_handle->sched_switch_comm_field;
}

static void set_entry_values(struct kshark_data_stream *stream,
			     struct tep_record *record,
			     struct kshark_entry *entry)
{
	struct tep_handle *tep = kshark_get_tep(stream);

	if (!tep)
		return;

	/* Offset of the record */
	entry->offset = record->offset;

	/* CPU Id of the record */
	entry->cpu = record->cpu;

	/* Time stamp of the record */
	entry->ts = record->ts;

	/* Event Id of the record */
	entry->event_id = tep_data_type(tep, record);

	/*
	 * Is visible mask. This default value means that the entry
	 * is visible everywhere.
	 */
	entry->visible = 0xFF;

	/* Process Id of the record */
	entry->pid = tep_data_pid(tep, record);
}

/** Prior time offset of the "missed_events" entry. */
#define ME_ENTRY_TIME_SHIFT	10

static void missed_events_action(struct kshark_data_stream *stream,
				 struct tep_record *record,
				 struct kshark_entry *entry)
{
	/*
	 * Use the offset field of the entry to store the number of missed
	 * events.
	 */
	entry->offset = record->missed_events;

	entry->cpu = record->cpu;

	/*
	 * Position the "missed_events" entry a bit before (in time)
	 * the original record.
	 */
	entry->ts = record->ts - ME_ENTRY_TIME_SHIFT;

	/* All custom entries must have negative event Identifiers. */
	entry->event_id = KS_EVENT_OVERFLOW;

	entry->visible = 0xFF;

	entry->pid = tep_data_pid(kshark_get_tep(stream), record);
}

/**
 * rec_list is used to pass the data to the load functions.
 * The rec_list will contain the list of entries from the source,
 * and will be a link list of per CPU entries.
 */
struct rec_list {
	union {
		/* Used by kshark_load_data_records */
		struct {
			/** next pointer, matches entry->next */
			struct rec_list		*next;
			/** pointer to the raw record data */
			struct tep_record	*rec;
		};
		/** entry - Used for kshark_load_data_entries() */
		struct kshark_entry		entry;
	};
};

static int get_next_pid(struct kshark_data_stream *stream,
			struct tep_record *record)
{
	unsigned long long val;
	int ret;

	ret = tep_read_number_field(get_sched_next(stream),
				    record->data, &val);

	return ret ? : (int) val;
}

static void register_command(struct kshark_data_stream *stream,
			     struct tep_record *record,
			     int pid)
{
	struct tep_format_field *comm_field = get_sched_comm(stream);
	const char *comm = record->data + comm_field->offset;
	/*
	 * TODO: The retrieve of the name of the command above needs to be
	 * implemented as a wrapper function in libtracevent.
	 */

	if (!tep_is_pid_registered(kshark_get_tep(stream), pid))
			tep_register_comm(kshark_get_tep(stream), comm, pid);
}

/**
 * rec_type defines what type of rec_list is being used.
 */
enum rec_type {
	REC_RECORD,
	REC_ENTRY,
};

static void free_rec_list(struct rec_list **rec_list, int n_cpus,
			  enum rec_type type)
{
	struct rec_list *temp_rec;
	int cpu;

	for (cpu = 0; cpu < n_cpus; ++cpu) {
		while (rec_list[cpu]) {
			temp_rec = rec_list[cpu];
			rec_list[cpu] = temp_rec->next;
			if (type == REC_RECORD)
				tracecmd_free_record(temp_rec->rec);
			free(temp_rec);
		}
	}
	free(rec_list);
}

//NOTE: Changed here. (COUPLEBREAK) (2025-03-29)
// Possible (EXTENSION)
/* Below are the constants used to index bits/flags in the
 * couplebreak_evts_flags bitmask of streams.
 * This provides a single point of change when working with
 * the bitmask.
*/
///@brief Bitmask flag position for the couplebreak/sched_switch[target] event.
#define COUPLEBREAK_SSWITCH_FPOS 0
///@brief Bitmask flag position for the couplebreak/sched_waking[target] event.
#define COUPLEBREAK_SWAKING_FPOS 1
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
// Possible (EXTENSION)
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
static int couplebreak_origin_id_to_flag_pos(struct kshark_data_stream *stream,
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
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
// Possible (EXTENSION)
/**
 * @brief Function serves as a mapping between the flag position
 * of a couplebreak event (e.g. "couplebreak/sched_switch[target]")
 * and its event Id.
 * 
 * @param stream Input location for a Data stream pointer.
 * @param flag_pos The position of the flag in the couplebreak_evts_flags bitmask.
 * @return Event Id of the couplebreak event or -1 if the flag position
 * is invalid (mapping doesn't exist).
 */
static int flag_pos_to_couplebreak_id(struct kshark_data_stream *stream, int flag_pos)
{
    /*
	|0|0|0|0|0|0|0|0| ... |0|0|0|0|0|0|0|0| <- 32 bits
	Each bit is a flag. The position of the flag corresponds to the event Id,
	all according to the switch table below.
	*/

	int return_evt_id = -1;
	switch (flag_pos) {
		case COUPLEBREAK_SSWITCH_FPOS:
			return_evt_id = kshark_find_event_id(stream, "sched/sched_switch");
			break;
		case COUPLEBREAK_SWAKING_FPOS:
			return_evt_id = kshark_find_event_id(stream, "sched/sched_waking");
			break;
		default: // Represents a fault.
			return -1;
	}
	return COUPLEBREAK_EVENT_ID_SHIFT - return_evt_id;
}
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
// Possible (EXTENSION) Alternatively, a refactoring of couplebreak could
// do away with always searching for a couplebreak Id and instead have the stream
// keep a list of Ids of couplebreak events active in it.
/**
 * @brief Function finds the event Id of from an origin event's name and
 * if all goes well, returns the event Id of the couplebreak event.
 * 
 * @param stream Input location for a Data stream pointer.
 * @param origin_evt_name The name of the origin event.
 * @return Event Id of the couplebreak event or -1 if the stream is invalid
 * or origin event's name is invalid for a given stream.
 */
static int find_couplebreak_event_id_from_origin_name(struct kshark_data_stream *stream,
	const char *origin_evt_name)
{
	if (!stream || !origin_evt_name)
		return -1;
	
	int origin_event_id = kshark_find_event_id(stream, origin_evt_name);
	int result = -1;
	
	if (origin_event_id >= 0)
		result = COUPLEBREAK_EVENT_ID_SHIFT - origin_event_id;
	
	return result;
}
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
// Possible (EXTENSION)
/**
 * @brief Checks if the event Id is a couplebreak event. This check
 * always checks with the original event Ids from the trace and might
 * read from the trace stream (therefore be a little bit slow).
 * 
 * @param stream Input location for a Data stream pointer.
 * @param event_id The event Id to check.
 * @return True if the event Id is a couplebreak event, false otherwise.
 */
static bool is_couplebreak_event(struct kshark_data_stream *stream, int event_id)
{
	const int couplebreak_switch_id = find_couplebreak_event_id_from_origin_name(stream, "sched/sched_switch");
	const int couplebreak_waking_id = find_couplebreak_event_id_from_origin_name(stream, "sched/sched_waking");

	const bool is_couplebreak_sst = (event_id == couplebreak_switch_id);
	const bool is_couplebreak_swt = (event_id == couplebreak_waking_id);

	return (is_couplebreak_sst || is_couplebreak_swt);
}
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
// Possible (EXTENSION)
/**
 * @brief Get the couplebreak event name as a string.
 * 
 * @param stream Input location for a Data stream pointer.
 * @param event_id The event Id of a couplebreak event.
 * @return Name of a couplebreak event or NULL if the event Id is invalid.
 */
static char *get_couplebreak_event_name(struct kshark_data_stream *stream, int event_id)
{
	const int couplebreak_sst_id = find_couplebreak_event_id_from_origin_name(stream, "sched/sched_switch");
 	const int couplebreak_swt_id = find_couplebreak_event_id_from_origin_name(stream, "sched/sched_waking");

	const char *origin_evt_name = NULL;
	char *buffer;

	if (event_id == couplebreak_sst_id)
		origin_evt_name = "sched_switch";
	else if (event_id == couplebreak_swt_id)
		origin_evt_name = "sched_waking";
	else
		return NULL; // Incorrect event Id, not a couplebreak event
	
	asprintf(&buffer, "couplebreak/%s[target]", origin_evt_name);

	return buffer;
}
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-29)
/**
 * @brief Record a new couplebreak event type, if it was not yet
 * encountered in the stream. If it was, increase the amount of
 * encountered couplebreak events and set the flag for this event type.
 * 
 * @param stream Stream to record the encounter in.
 * @param origin_entry Origin entry of the couplebreak event.
 */
static void record_new_couplebreak_event_type(struct kshark_data_stream *stream,
	const struct kshark_entry *origin_entry) {
	// Change once per stream load: if such an event type was not yet found
	// increase the amount of encountered couplebreak events an set the flag
	// for this event type.
	int16_t origin_event_id = (origin_entry->visible & KS_PLUGIN_UNTOUCHED_MASK) ?
		origin_entry->event_id : kshark_get_event_id(origin_entry);

	const int flag_pos = couplebreak_origin_id_to_flag_pos(stream, origin_event_id);
	if (!(stream->couplebreak_evts_flags & (1 << flag_pos))) {
		stream->n_couplebreak_evts++;
		stream->couplebreak_evts_flags |= (1 << flag_pos);
	}
} 
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
/**
 * @brief Type definition for custom entry creation functions, utilised by
 * couplebreak.
 * 
 * @param stream Data stream pointer to which events belong.
 * @param temp_rec Temporary record list node.
 * @param record Trace event processing record with values from the trace.
 * @param origin_entry Origin entry for the custom entry.
 * @return Pointer to a newly created custom entry.
 */
typedef struct kshark_entry* (*custom_entry_creation_func)(
	struct kshark_data_stream *stream,
	struct rec_list *temp_rec,
	struct tep_record *record,
	struct kshark_entry *origin_entry);
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
// Copy of missed_events_action, basically
// Possible (EXTENSION) - more functions if more events get couplebreak support.
/**
 * @brief Makes a new "couplebreak/sched_switch[target]" entry with data from
 * the tracing record and the origin entry. The entry is taken from the 
 * temporary record list and its values are adjusted to become a couplebreak
 * entry.
 * 
 * @param stream Data stream pointer to which events belong.
 * @param temp_rec Temporary record list node.
 * @param record Trace event processing record with values from the trace.
 * @param origin_entry Origin entry for the target entry.
 * @return Pointer to a newly created "couplebreak/sched_switch[target]" entry.
 */
static struct kshark_entry* create_sched_switch_target(struct kshark_data_stream *stream,
	struct rec_list *temp_rec, struct tep_record *record,
	struct kshark_entry *origin_entry)
{

	struct kshark_entry *entry = &temp_rec->entry;
	/*
	 * Let's store the origin entry in the now useless offset
	 * (target events are made up - not in the file).
	*/
	entry->offset = (int64_t)origin_entry;
	
	entry->cpu = record->cpu;

	/*
	 * Position the "sched_switch[target]" entry at the same time as sched_switch.
	 * Post-calibration should put the event after the origin anyway.
	*/
	entry->ts = record->ts;

	/* All custom entries must have negative event Identifiers. */
	// Origin event's Id is the one recorded, to achieve consistency even if some plugins
	// change this field (which is very rare).
	int16_t origin_event_id = (origin_entry->visible & KS_PLUGIN_UNTOUCHED_MASK) ?
		origin_entry->event_id : kshark_get_event_id(origin_entry);
	entry->event_id = COUPLEBREAK_EVENT_ID_SHIFT - origin_event_id;

	entry->visible = 0xFF;

	/* Make the owner pid the pid of the task to be switched to. */
	entry->pid = get_next_pid(stream, record);

	record_new_couplebreak_event_type(stream, origin_entry);

	return entry;
}
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
/**
 * @brief Makes a new "couplebreak/sched_waking[target]" entry with data from
 * the tracing record and the origin entry. The entry is taken from the 
 * temporary record list and its values are adjusted to become a couplebreak
 * entry.
 * 
 * @param stream Data stream pointer to which events belong.
 * @param temp_rec Temporary record list node.
 * @param record Trace event processing record with values from the trace.
 * @param origin_entry Origin entry for the target entry.
 * @return Pointer to a newly created "couplebreak/sched_waking[target]" entry.
 */
static struct kshark_entry *create_sched_waking_target(struct kshark_data_stream *stream,
	struct rec_list *temp_rec, struct tep_record *record,
	struct kshark_entry *origin_entry)
{
	struct kshark_entry *entry = &temp_rec->entry;
	/*
	 * Let's store the origin entry in the now useless offset
	 * (target events are made up - not in the file).
	*/
	entry->offset = (int64_t)origin_entry;

	/*
	 * Position the "sched_waking[target]" entry at the same time as sched_waking.
	 * Post-calibration should put the event after the origin anyway.
	*/
	entry->ts = record->ts;

	/* All custom entries must have negative event Identifiers. */
	// Origin event's Id is the one recorded, to achieve consistency even if some plugins
	// change this field (which is very rare).
	int16_t origin_event_id = (origin_entry->visible & KS_PLUGIN_UNTOUCHED_MASK) ?
		origin_entry->event_id : kshark_get_event_id(origin_entry);
	entry->event_id = COUPLEBREAK_EVENT_ID_SHIFT - origin_event_id;

	entry->visible = 0xFF;

	/* Make the owner pid the pid of the task to be woken up. */
	// The approach below is used in plugins, so it is true and tested.
	
	struct tep_event *sched_waking_event = NULL;
	define_wakeup_event(kshark_get_tep(stream), &sched_waking_event);

	unsigned long long waked_pid_val;
	struct tep_format_field *sched_waking_pid_field = tep_find_any_field(sched_waking_event, "pid");
	int pid_succs = tep_read_number_field(sched_waking_pid_field, record->data, &waked_pid_val);
	// Set the target entry's PID to either the woken up task or, if that failed, to the
	// PID of the origin entry. 
	entry->pid = (pid_succs == 0) ? (int32_t)waked_pid_val : origin_entry->pid;

	unsigned long long waked_cpu_val;
	struct tep_format_field *sched_waking_target_cpu_field = tep_find_any_field(sched_waking_event, "target_cpu");
	int tcpu_succs = tep_read_number_field(sched_waking_target_cpu_field, record->data, &waked_cpu_val);
	// This field should hopefully change to the actual CPU on which the task will run after
	// all events have been loaded (and can be easily accessed by time).
	// But target CPU is the best estimation at this point in time.
	entry->cpu = (tcpu_succs == 0) ? (int16_t)waked_cpu_val : record->cpu;

	record_new_couplebreak_event_type(stream, origin_entry);

	return entry;
}

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
/**
 * @brief Create a custom entry with a KernelShark context, data stream,
 * trace record, related entry, temporary record list node and the one after it,
 * advanced event filter, count of current entries in the stream and a function
 * to create the custom entry.
 * 
 * @param kshark_ctx KernelShark context
 * @param stream Data stream pointer to which events belong.
 * @param rec Trace event processing record with values from the trace.
 * @param entry KernelShark entry from which to create the custom entry.
 * @param temp_next Next record after `temp_rec`.
 * @param temp_rec Temporary record list node.
 * @param adv_filter Advanced event filter.
 * @param count Count of current entries in the stream.
 * @param create_custom_func Function to create the custom entry.
 * @return 0 on successful creation of the custom entry, otherwise 1.
 * @note The function is currently used only to create custom entries for
 * couplebreak events.
 */
static int create_custom_entry(struct kshark_context *kshark_ctx,
							   struct kshark_data_stream *stream,
							   struct tep_record *rec,
							   struct kshark_entry *entry,
							   struct rec_list * **temp_next, // Need to update actual value
							   struct rec_list * *temp_rec, // Need to update actual value
							   struct tep_event_filter *adv_filter,
							   ssize_t *count,
							   custom_entry_creation_func create_custom_func)
{
	const int FAIL = 1;

	// Move forward to the next entry
	*temp_next = &((*temp_rec)->next);

	// Allocate a new rec_list node and continue.
	*(*temp_next) = (*temp_rec) = calloc(1, sizeof(*(*temp_rec)));
	if (!(*temp_rec))
		return FAIL;

	/*
	* Insert a custom target entry just
	* after sched_switch record.
	*/
	struct kshark_entry *target_entry = create_custom_func(stream,
		*temp_rec, rec, entry);

	/* Apply time calibration. */
	kshark_postprocess_entry(stream, rec, target_entry);

	target_entry->stream_id = stream->stream_id;

	/* Apply Id filtering. */
	kshark_apply_filters(kshark_ctx, stream, target_entry);
	

	/* Apply advanced event filtering. */
	if (adv_filter && adv_filter->filters &&
		tep_filter_match(adv_filter, rec) != FILTER_MATCH)
		unset_event_filter_flag(kshark_ctx, target_entry);

	*count = (*count) + 1;

	return 0;
}
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
// Change is just that this static function was moved, it is
// originally a part of KernelShark, but there was a need to see it sooner
// during compilation.
static int pick_next_cpu(struct rec_list **rec_list, int n_cpus,
	enum rec_type type)
{
	uint64_t ts = 0;
	uint64_t rec_ts;
	int next_cpu = -1;
	int cpu;

	for (cpu = 0; cpu < n_cpus; ++cpu) {
		if (!rec_list[cpu])
			continue;

		switch (type) {
		case REC_RECORD:
			rec_ts = rec_list[cpu]->rec->ts;
			break;
		case REC_ENTRY:
			rec_ts = rec_list[cpu]->entry.ts;
			break;
		default:
			return -1;
		}
		if (!ts || rec_ts < ts) {
			ts = rec_ts;
			next_cpu = cpu;
		}
	}

	return next_cpu;
}
// END of change

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
/**
 * @brief Fills the sorted_entries array with the entries from the rec_list.
 * Uses a technique same as the one used later to load the data in the record
 * list after getting all records, that is pick entries with the help of
 * `pick_next_cpu` function to output an array of entries sorted by time.
 * 
 * @param stream Data stream pointer to which events belong.
 * @param rec_list List of all events to sort - this linked list will have its
 * nodes adjusted to advance iteration (i.e. function mutates this argument).
 * @param sorted_entries Output array of pointers to kshark_entry objects,
 * which will be filled with the entries from the rec_list and sorted by time.
 * @param total Total number of entries to be sorted.
 */
static inline void fill_sorted_entries(struct kshark_data_stream *stream,
	struct rec_list **rec_list, struct kshark_entry **sorted_entries, ssize_t total)
{
	enum rec_type type = REC_ENTRY;

	for (int count = 0; count < total; count++) {
		int next_cpu;

		next_cpu = pick_next_cpu(rec_list, stream->n_cpus, type);

		if (next_cpu >= 0) {
			sorted_entries[count] = &rec_list[next_cpu]->entry;
			rec_list[next_cpu] = rec_list[next_cpu]->next;
		}
	}
}

//NOTE: Changed here. (COUPLEBREAK) (2025-03-29)
/**
 * @brief Correct CPUs of "couplebreak/sched_waking[target]" entries
 * to reflect where the task will run after being awoken and switched.
 * 
 * @param stream Data stream pointer to which events belong.
 * @param sorted_entries Output array of pointers to kshark_entry objects,
 * which will be filled with the entries from the rec_list and sorted by time. 
 * @param total Total number of entries to be possibly corrected.
 * 
 * @note This might mess the plots a little bit as it will look like the target CPU was working.
 */
static void correct_couplebreak_cpus_inner(struct kshark_data_stream *stream,
	struct kshark_entry **sorted_entries, ssize_t total)
{
	int couplebreak_swt_id = find_couplebreak_event_id_from_origin_name(stream, "sched/sched_waking");
	int couplebreak_sst_id = find_couplebreak_event_id_from_origin_name(stream, "sched/sched_switch");

	for (ssize_t i = 0 ; i < total ; i++) {
		struct kshark_entry *curr_event = sorted_entries[i];
		bool curr_event_untouched = (curr_event->visible & KS_PLUGIN_UNTOUCHED_MASK);

		int16_t curr_event_id = curr_event_untouched ?
			curr_event->event_id : kshark_get_event_id(curr_event);
		
		if (curr_event_id != couplebreak_swt_id) {
			// No need to correct the cpu of an event that is not sched_waking[target].
			continue;
		}

		int32_t curr_event_pid = curr_event_untouched ?
			curr_event->pid : kshark_get_pid(curr_event);

		// Search in the rest of the array for a switch target of this task onto some CPU
		// an adjust that field if necessary.
		for (ssize_t j = i ; j < total ; j++) {
			const struct kshark_entry *next_event = sorted_entries[j];
			bool next_event_untouched = (next_event->visible & KS_PLUGIN_UNTOUCHED_MASK);

			// One can never be too careful with possible entry field changes.
			int16_t next_event_id = next_event_untouched ?
				next_event->event_id : kshark_get_event_id(next_event);
			
			int32_t next_event_pid = next_event_untouched ?
				next_event->pid : kshark_get_pid(next_event);

			if (next_event_id == couplebreak_sst_id && // Correct event
				next_event_pid == curr_event_pid) { // Of the same task
				
				curr_event->cpu = next_event->cpu;
				break;
			}
		}
	}
}

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
// Possible (EXTENSION) - this function and all that it entails could be refactored to a plugin,
// where this correction is also possible. There was no strong reason to split couplebreak
// and how it should work from its main working environment though, hence this implementation.
/**
 * @brief Correct CPUs of "couplebreak/sched_waking[target]" entries
 * to reflect where the task will run after being awoken and switched. This
 * function prepares a copy of the record list, sorts that copy by time into an
 * array and calls the function that does the correction.
 * 
 * @param stream Data stream pointer to which events belong.
 * @param rec_list List of all events to sort - this linked list will be copied,
 * so that this one's nodes aren't changed for any possible future use.
 * @param total Total number of entries to be possibly corrected.
 * @return Return code of the function, 0 on success, -ENOMEM on
 * memory allocation fail.
 */
static int correct_couplebreak_cpus(struct kshark_data_stream *stream,
	struct rec_list **rec_list, ssize_t total)
{
	struct rec_list **temp_list = calloc(total, sizeof(*temp_list));
	if (!temp_list) {
		goto fail;
	}
	// Copy the rec_list to a temporary list.
	for (ssize_t i = 0; i < total; i++) {
		temp_list[i] = rec_list[i];
	}

	// Create a sorted array of kshark_events from the rec_list.
	struct kshark_entry **sorted_entries = calloc(total, sizeof(*sorted_entries));
	if (!sorted_entries) {
		free(temp_list);
		goto fail;
	}

	fill_sorted_entries(stream, temp_list, sorted_entries, total);

	correct_couplebreak_cpus_inner(stream, sorted_entries, total);

	// While it may look a little reckless to just allocate memory like this,
	// the sorted_entries' space will actually be needed later in the load functions
	// which called the get_records function - so this space won't be really wasted.
	// This does pose a possible (EXTENSION), where if couplebreak is enabled, the
	// sorted_entries array could be used later instead of freed, to skip the double
	// time sorting this currently creates.
	free(temp_list);
	free(sorted_entries);

	return 0;

 fail:
	fprintf(stderr,
		"Failed to allocate memory during couplebreak's CPU corrections.\n");
	return -ENOMEM;
}

static ssize_t get_records(struct kshark_context *kshark_ctx,
			   struct kshark_data_stream *stream,
			   struct rec_list ***rec_list,
			   enum rec_type type)
{
	struct tep_event_filter *adv_filter = NULL;
	struct tracecmd_input *input;
	struct rec_list **temp_next;
	struct rec_list **cpu_list;
	struct rec_list *temp_rec;
	struct tep_record *rec;
	ssize_t count, total = 0;
	int pid, next_pid, cpu;

	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	// Resets couplebreak state on each load of a stream.
	stream->couplebreak_evts_flags = 0;
	stream->n_couplebreak_evts = 0;
	// END of change

	input = kshark_get_tep_input(stream);
	if (!input)
		return -EFAULT;

	cpu_list = calloc(stream->n_cpus, sizeof(*cpu_list));
	if (!cpu_list)
		return -ENOMEM;

	if (type == REC_ENTRY)
		adv_filter = get_adv_filter(stream);

	for (cpu = 0; cpu < stream->n_cpus; ++cpu) {
		count = 0;
		cpu_list[cpu] = NULL;
		temp_next = &cpu_list[cpu];
		rec = tracecmd_read_cpu_first(kshark_get_tep_input(stream), cpu);
		while (rec) {
			*temp_next = temp_rec = calloc(1, sizeof(*temp_rec));
			if (!temp_rec)
				goto fail;

			temp_rec->next = NULL;

			switch (type) {
			case REC_RECORD:
				temp_rec->rec = rec;
				pid = tep_data_pid(kshark_get_tep(stream), rec);
				break;
			case REC_ENTRY: {
				struct kshark_entry *entry;

				/*NOTE: Changed here. ("COUPLEBREAK"). Just a note where
				  to put events if we want to add more and have them appear
				  before the event which triggers this addition.
				*/
				// Insert events BEFORE
				// END of change

				if (rec->missed_events) {
					/*
					 * Insert a custom "missed_events" entry just
					 * before this record.
					 */
					entry = &temp_rec->entry;
					missed_events_action(stream, rec, entry);

					/* Apply time calibration. */
					kshark_postprocess_entry(stream, rec, entry);

					entry->stream_id = stream->stream_id;

					temp_next = &temp_rec->next;
					++count;

					/* Now allocate a new rec_list node and continue. */
					*temp_next = temp_rec = calloc(1, sizeof(*temp_rec));
					if (!temp_rec)
						goto fail;
				}

				entry = &temp_rec->entry;
				set_entry_values(stream, rec, entry);

				if (entry->event_id == get_sched_switch_id(stream)) {
					next_pid = get_next_pid(stream, rec);
					if (next_pid >= 0)
						register_command(stream, rec, next_pid);
					
					//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
					if (stream->couplebreak_on) {
						int retcode = create_custom_entry(kshark_ctx, stream, rec, entry,
							&temp_next, &temp_rec, adv_filter, &count, create_sched_switch_target);
						if (retcode == 1) goto fail;
					}
					// END of change
				}

				entry->stream_id = stream->stream_id;

				/*
				 * Post-process the content of the entry. This includes
				 * time calibration and event-specific plugin actions.
				 */
				kshark_postprocess_entry(stream, rec, entry);

				pid = entry->pid;

				/* Apply Id filtering. */
				kshark_apply_filters(kshark_ctx, stream, entry);

				/* Apply advanced event filtering. */
				if (adv_filter && adv_filter->filters &&
				    tep_filter_match(adv_filter, rec) != FILTER_MATCH)
					unset_event_filter_flag(kshark_ctx, entry);
				
				/*NOTE: Changed here. ("COUPLEBREAK"). Just a note where
				  to put events if we want to add more and have them appear
				  after the event which triggers this addition.
				*/
				// Insert events AFTER
				// END of change
				
				//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
				// Could be change to a search in a collection of allowed events to be split in a stream
				const int sch_waking_id = kshark_find_event_id(stream, "sched/sched_waking");
				int16_t origin_event_id = (entry->visible & KS_PLUGIN_UNTOUCHED_MASK) ?
					entry->event_id : kshark_get_event_id(entry);
				if (origin_event_id == (int16_t)sch_waking_id) {
					if (stream->couplebreak_on) {
						int retcode = create_custom_entry(kshark_ctx, stream, rec, entry,
							&temp_next, &temp_rec, adv_filter, &count, create_sched_waking_target);
						if (retcode == 1) goto fail;
					}
				}
				// END of change

				tracecmd_free_record(rec);
				break;
			} /* REC_ENTRY */
			}

			kshark_hash_id_add(stream->tasks, pid);

			temp_next = &temp_rec->next;

			++count;
			rec = tracecmd_read_data(kshark_get_tep_input(stream), cpu);
		}

		if (!count)
			kshark_hash_id_add(stream->idle_cpus, cpu);
		else
			total += count;
	}

	*rec_list = cpu_list;

	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	if (stream->couplebreak_on) {
		// Use the newest data to correct the cpu of the couplebreak events.
		// The rec_list is not changed.
		int op_result = correct_couplebreak_cpus(stream, *rec_list, total);
		if (op_result == -ENOMEM) {
			return -ENOMEM;
		}
	}
	// END of change

	return total;

 fail:
	free_rec_list(cpu_list, stream->n_cpus, type);
	return -ENOMEM;
}


/**
 * @brief Load the content of the trace data file asociated with a given
 *	  Data stream into an array of kshark_entries. This function
 *	  provides an abstraction of the entries from the raw data
 *	  that is read, however the "latency" and the "info" fields can be
 *	  accessed only via the offset into the file. This makes the access
 *	  to these two fields much slower.
 *	  If one or more filters are set, the "visible" fields of each entry
 *	  is updated according to the criteria provided by the filters. The
 *	  field "filter_mask" of the session's context is used to control the
 *	  level of visibility/invisibility of the filtered entries.
 *
 * @param stream: Input location for the FTRACE data stream pointer.
 * @param kshark_ctx: Input location for context pointer.
 * @param data_rows: Output location for the trace data. The user is
 *		     responsible for freeing the elements of the outputted
 *		     array.
 *
 * @returns The size of the outputted data in the case of success, or a
 *	    negative error code on failure.
 */
ssize_t tepdata_load_entries(struct kshark_data_stream *stream,
				struct kshark_context *kshark_ctx,
				struct kshark_entry ***data_rows)
{
	enum rec_type type = REC_ENTRY;
	struct kshark_entry **rows;
	struct rec_list **rec_list;
	ssize_t count, total = 0;

	total = get_records(kshark_ctx, stream, &rec_list, type);
	if (total < 0)
		goto fail;

	rows = calloc(total, sizeof(struct kshark_entry *));
	if (!rows)
		goto fail_free;

	for (count = 0; count < total; count++) {
		int next_cpu;

		next_cpu = pick_next_cpu(rec_list, stream->n_cpus, type);

		if (next_cpu >= 0) {
			rows[count] = &rec_list[next_cpu]->entry;
			rec_list[next_cpu] = rec_list[next_cpu]->next;
		}
	}

	/* There should be no entries left in rec_list. */
	free_rec_list(rec_list, stream->n_cpus, type);
	*data_rows = rows;

	return total;

 fail_free:
	free_rec_list(rec_list, stream->n_cpus, type);

 fail:
	fprintf(stderr, "Failed to allocate memory during data loading.\n");
	return -ENOMEM;
}

static ssize_t tepdata_load_matrix(struct kshark_data_stream *stream,
				   struct kshark_context *kshark_ctx,
				   int16_t **event_array,
				   int16_t **cpu_array,
				   int32_t **pid_array,
				   int64_t **offset_array,
				   int64_t **ts_array)
{
	enum rec_type type = REC_ENTRY;
	struct rec_list **rec_list;
	ssize_t count, total = 0;
	bool status;

	total = get_records(kshark_ctx, stream, &rec_list, type);
	if (total < 0)
		goto fail;

	status = kshark_data_matrix_alloc(total, event_array,
						 cpu_array,
						 pid_array,
						 offset_array,
						 ts_array);
	if (!status)
		goto fail_free;

	for (count = 0; count < total; count++) {
		int next_cpu;

		next_cpu = pick_next_cpu(rec_list, stream->n_cpus, type);
		if (next_cpu >= 0) {
			struct rec_list *rec = rec_list[next_cpu];
			struct kshark_entry *e = &rec->entry;

			if (offset_array)
				(*offset_array)[count] = e->offset;

			if (cpu_array)
				(*cpu_array)[count] = e->cpu;

			if (ts_array) {
				kshark_calib_entry(stream, e);
				(*ts_array)[count] = e->ts;
			}

			if (pid_array)
				(*pid_array)[count] = e->pid;

			if (event_array)
				(*event_array)[count] = e->event_id;

			rec_list[next_cpu] = rec_list[next_cpu]->next;
			free(rec);
		}
	}

	/* There should be no entries left in rec_list. */
	free_rec_list(rec_list, stream->n_cpus, type);
	return total;

 fail_free:
	free_rec_list(rec_list, stream->n_cpus, type);

 fail:
	fprintf(stderr, "Failed to allocate memory during data loading.\n");
	return -ENOMEM;
}

/**
 * @brief Load the content of the trace data file into an array of
 *	  tep_records. Use this function only if you need fast access
 *	  to all fields of the record.
 *
 * @param kshark_ctx: Input location for the session context pointer.
 * @param sd: Data stream identifier.
 * @param data_rows: Output location for the trace data. Use tracecmd_free_record()
 *	 	     to free the elements of the outputted array.
 *
 * @returns The size of the outputted data in the case of success, or a
 *	    negative error code on failure.
 */
ssize_t kshark_load_tep_records(struct kshark_context *kshark_ctx, int sd,
				struct tep_record ***data_rows)
{
	struct kshark_data_stream *stream;
	enum rec_type type = REC_RECORD;
	struct rec_list **rec_list;
	struct rec_list *temp_rec;
	struct tep_record **rows;
	struct tep_record *rec;
	ssize_t count, total = 0;

	if (*data_rows)
		free(*data_rows);

	stream = kshark_get_data_stream(kshark_ctx, sd);
	if (!stream)
		return -EBADF;

	total = get_records(kshark_ctx, stream, &rec_list, type);
	if (total < 0)
		goto fail;

	rows = calloc(total, sizeof(struct tep_record *));
	if (!rows)
		goto fail_free;

	for (count = 0; count < total; count++) {
		int next_cpu;

		next_cpu = pick_next_cpu(rec_list, stream->n_cpus, type);

		if (next_cpu >= 0) {
			rec = rec_list[next_cpu]->rec;
			rows[count] = rec;

			temp_rec = rec_list[next_cpu];
			rec_list[next_cpu] = rec_list[next_cpu]->next;
			free(temp_rec);
			/* The record is still referenced in rows */
		}
	}

	/* There should be no records left in rec_list. */
	free_rec_list(rec_list, stream->n_cpus, type);
	*data_rows = rows;

	return total;

 fail_free:
	free_rec_list(rec_list, stream->n_cpus, type);

 fail:
	fprintf(stderr, "Failed to allocate memory during data loading.\n");
	return -ENOMEM;
}

static int tepdata_get_event_id(struct kshark_data_stream *stream,
				const struct kshark_entry *entry)
{
	int event_id = KS_EMPTY_BIN;
	struct tep_record *record;

	if (entry->visible & KS_PLUGIN_UNTOUCHED_MASK) {
		event_id = entry->event_id;
	} else {
		//NOTE: Changed here. (COUPLEBREAK) (2025-03-29)
		if (stream->couplebreak_on){
			// Since couplebreak events cannot tap into the trace event processing
			// handler (their offset field hosts something else), we check if an
			// event is a couplebreak one and if so, just return the event Id.
			// WARNING: This is also implies that NO PLUGIN can change the event Id of a
			// couplebreak event, otherwise this breaks.
			if (is_couplebreak_event(stream, entry->event_id))
				return entry->event_id;
		}
		// END of change

		/*
		 * The entry has been touched by a plugin callback function.
		 * Because of this we do not trust the value of
		 * "entry->event_id".
		 *
		 * Currently the data reading operations are not thread-safe.
		 * Use a mutex to protect the access.
		 */
		pthread_mutex_lock(&stream->input_mutex);

		record = tracecmd_read_at(kshark_get_tep_input(stream),
					  entry->offset, NULL);

		if (record)
			event_id = tep_data_type(kshark_get_tep(stream), record);

		tracecmd_free_record(record);

		pthread_mutex_unlock(&stream->input_mutex);
	}

	return (event_id == -1)? -EFAULT : event_id;
}

static char* missed_events_dump(__attribute__ ((unused)) struct kshark_data_stream *stream,
				const struct kshark_entry *entry,
				bool get_info)
{
	char *buffer;
	int size = 0;

	if (get_info)
		size = asprintf(&buffer, "missed_events=%i",
				(int) entry->offset);
	else
		size = asprintf(&buffer, "missed_events");

	if (size > 0)
		return buffer;

	return NULL;
}

static char *tepdata_get_event_name(struct kshark_data_stream *stream,
				    const struct kshark_entry *entry)
{
	struct kshark_generic_stream_interface *interface;
	struct tep_event *event;
	char *buffer;

	interface = stream->interface;
	if (!interface)
		return NULL;

	int event_id = interface->get_event_id(stream, entry);
	if (event_id == -EFAULT)
		return NULL;

	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	// Couplebreak events have special names composed of their origin,
	// so a special function is used to get their name.
	// This if is also placed before missing events, to avoid
	// confusion of a couplebreak event with a missed event.
	if (is_couplebreak_event(stream, event_id)) {
		return get_couplebreak_event_name(stream, event_id);
	}
	// END of change

	if (event_id < 0) {
		switch (event_id) {
		case KS_EVENT_OVERFLOW:
			return missed_events_dump(stream, entry, false);
		default:
			return NULL;
		}
	}

	/*
	 * Currently the data reading operations are not thread-safe.
	 * Use a mutex to protect the access.
	 */
	pthread_mutex_lock(&stream->input_mutex);

	event = tep_find_event(kshark_get_tep(stream), event_id);

	pthread_mutex_unlock(&stream->input_mutex);

	if (!event ||
            asprintf(&buffer, "%s/%s", event->system, event->name) <= 0)
		return NULL;

	return buffer;
}

//NOTE: Changed here. (COUPLEBREAK) (2025-03-30)
/**
 * @brief Gets the original PID of a couplebreak entry, via reconstructing how it was
 * created before any plugin could have changed it.
 * 
 * @param stream Data stream pointer to which events belong.
 * @param entry KernelShark entry for which to get original the PID.
 * @return The original PID of the entry, or KS_EMPTY_BIN if it was not found.
 */
static int get_couplebreak_pid(struct kshark_data_stream *stream,
	const struct kshark_entry *entry)
{
	const struct kshark_entry* origin_entry = (const struct kshark_entry*)entry->offset;
	struct tep_record *record;
	int pid = KS_EMPTY_BIN;

	const int sst_id = find_couplebreak_event_id_from_origin_name(stream, "sched/sched_switch");
	const int swt_id = find_couplebreak_event_id_from_origin_name(stream, "sched/sched_waking");

	// Just like in the tepdata_get_pid function, we need to guard against parallel access
	pthread_mutex_lock(&stream->input_mutex);
	record = tracecmd_read_at(kshark_get_tep_input(stream),
		origin_entry->offset, NULL);
	if (record) {
		if (entry->event_id == sst_id) {
			// Reconstruct how the PID was obtained for a switch
			pid = get_next_pid(stream, record);
		} else if (entry->event_id == swt_id) {
			// Reconstruct how the PID was obtained for a wakeup
			struct tep_event *sched_waking_event = NULL;
			define_wakeup_event(kshark_get_tep(stream), &sched_waking_event);

			unsigned long long waked_pid_val;
			struct tep_format_field *sched_waking_pid_field =
				tep_find_any_field(sched_waking_event, "pid");
			int pid_succs =
				tep_read_number_field(sched_waking_pid_field, record->data, &waked_pid_val);
			pid = (pid_succs == 0) ? (int32_t)waked_pid_val : origin_entry->pid;
		}
	}
	tracecmd_free_record(record);
	pthread_mutex_unlock(&stream->input_mutex);

	return pid;
}
// END of change

static int tepdata_get_pid(struct kshark_data_stream *stream,
			   const struct kshark_entry *entry)
{
	struct tep_record *record;
	int pid = KS_EMPTY_BIN;

	if (entry->visible & KS_PLUGIN_UNTOUCHED_MASK) {
		pid = entry->pid;
	} else {
		//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
		if (stream->couplebreak_on) {
			// Couplebreak events have no fallback for a changed PID, which is why
			// such changes are not allowed.
			if (is_couplebreak_event(stream, entry->event_id))
				return get_couplebreak_pid(stream, entry);
		}
		// END of change

		/*
		 * The entry has been touched by a plugin callback function.
		 * Because of this we do not trust the value of "entry->pid".
		 *
		 * Currently the data reading operations are not thread-safe.
		 * Use a mutex to protect the access.
		 */
		pthread_mutex_lock(&stream->input_mutex);

		record = tracecmd_read_at(kshark_get_tep_input(stream),
					  entry->offset, NULL);

		if (record)
			pid = tep_data_pid(kshark_get_tep(stream), record);

		tracecmd_free_record(record);

		pthread_mutex_unlock(&stream->input_mutex);
	}

	return pid;
}

static char *tepdata_get_task(struct kshark_data_stream *stream,
			      const struct kshark_entry *entry)
{
	struct kshark_generic_stream_interface *interface = stream->interface;
	const char *task;
	int pid;

	if (!interface)
		return NULL;

	pid = interface->get_pid(stream, entry);
	task = tep_data_comm_from_pid(kshark_get_tep(stream), pid);

	return task ? strdup(task) : NULL;
}

static char *tepdata_get_latency(struct kshark_data_stream *stream,
				 const struct kshark_entry *input_entry)
{
	struct tep_record *record;
	char *buffer;

	if (!init_thread_seq())
		return NULL;

	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	// Couplebreak events happened at the same time as their
	// origin events, so the latency can be taken from the origin events.
	// This is placed before a missed event check to avoid confusion.
	const struct kshark_entry *entry = input_entry;
	if (stream->couplebreak_on) {
		if (is_couplebreak_event(stream, entry->event_id)) {
			entry = (const struct kshark_entry*)input_entry->offset;
		}
	}
	// END of change

	/* Check if this is a "Missed event" (event_id < 0). */
	if (entry->event_id < 0)
		return NULL;

	/*
	 * Currently the data reading operations are not thread-safe.
	 * Use a mutex to protect the access.
	 */
	pthread_mutex_lock(&stream->input_mutex);
	
	record = tracecmd_read_at(kshark_get_tep_input(stream), entry->offset, NULL);

	if (!record) {
		pthread_mutex_unlock(&stream->input_mutex);
		return NULL;
	}

	trace_seq_reset(&seq);
	tep_print_event(kshark_get_tep(stream), &seq, record,
			"%s", TEP_PRINT_LATENCY);

	tracecmd_free_record(record);

	pthread_mutex_unlock(&stream->input_mutex);

	if (asprintf(&buffer, "%s", seq.buffer)  <= 0)
		return NULL;

	return buffer;
}

static char *get_info_str(struct kshark_data_stream *stream,
			  struct tep_record *record,
			  struct tep_event *event)
{
	char *buffer;

	if (!init_thread_seq() || !record || !event)
		return NULL;

	trace_seq_reset(&seq);
	tep_print_event(kshark_get_tep(stream), &seq, record,
			"%s", TEP_PRINT_INFO);

	if (!seq.len)
		return NULL;
	/*
	 * The event info string contains a trailing newline.
	 * Remove this newline.
	 */
	if (seq.buffer[seq.len - 1] == '\n')
		seq.buffer[seq.len - 1] = '\0';

	if (asprintf(&buffer, "%s", seq.buffer)  <= 0)
		return NULL;

	return buffer;
}

static char *tepdata_get_info(struct kshark_data_stream *stream,
			      const struct kshark_entry *entry)
{
	struct tep_record *record;
	struct tep_event *event;
	char *info = NULL;
	int event_id;

	if (entry->event_id < 0) {
		//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
		// Couplebreak events being just slightly altered origin events
		// means one can access the origin event's info for them.
		if (stream->couplebreak_on) {
			if (is_couplebreak_event(stream, entry->event_id)) {
				const struct kshark_entry *origin_entry = (struct kshark_entry *)entry->offset;
				return tepdata_get_info(stream, origin_entry);
			}
		}
		// END of change

		switch (entry->event_id) {
		case KS_EVENT_OVERFLOW:
			return missed_events_dump(stream, entry, true);
		default:
			return NULL;
		}
	}

	/*
	 * Currently the data reading operations are not thread-safe.
	 * Use a mutex to protect the access.
	 */
	pthread_mutex_lock(&stream->input_mutex);

	record = tracecmd_read_at(kshark_get_tep_input(stream), entry->offset, NULL);
	if (!record) {
		pthread_mutex_unlock(&stream->input_mutex);
		return NULL;
	}

	event_id = tep_data_type(kshark_get_tep(stream), record);
	event = tep_find_event(kshark_get_tep(stream), event_id);

	if (event)
		info = get_info_str(stream, record, event);

	tracecmd_free_record(record);

	pthread_mutex_unlock(&stream->input_mutex);

	return info;
}

static int *tepdata_get_event_ids(struct kshark_data_stream *stream)
{
	struct tep_event **events;
	int i, *evt_ids;

	events = tep_list_events(kshark_get_tep(stream), TEP_EVENT_SORT_SYSTEM);
	if (!events)
		return NULL;

	evt_ids = calloc(stream->n_events, sizeof(*evt_ids));
	if (!evt_ids)
		return NULL;

	for (i = 0; i < stream->n_events; ++i)
		evt_ids[i] = events[i]->id;

	return evt_ids;
}

static int tepdata_get_field_names(struct kshark_data_stream *stream,
				   const struct kshark_entry *input_entry,
				   char ***fields_str)
{
	struct tep_format_field *field, **fields;
	struct tep_event *event;
	int i= 0, nr_fields;
	char **buffer;

	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	/*
		Couplebreak target events are basically just a copy of origin entry, but
		they don't have the same values, like pid, timestamp, maybe even CPU, but
		especially not the same offset field value, which is completely changed to
		store the origin entry's address.

		Hence, accomodations are done, by asking for the origin entry's fields instead, as they
		are the same (couplebreak event is a slightly altered twin for the origin event).
		It would be a fruitless search for the program, if the made-up
		couplebreak entries were used instead, as they do not exist in the trace file.
	*/
	const struct kshark_entry *entry = input_entry;
	if (stream->couplebreak_on) {
		if (is_couplebreak_event(stream, entry->event_id)) {
			entry = (const struct kshark_entry*)input_entry->offset;
		}
	}
	// END of change

	*fields_str = NULL;
	event = tep_find_event(kshark_get_tep(stream), entry->event_id);
	if (!event)
		return 0;

	nr_fields = event->format.nr_fields + event->format.nr_common;
	buffer = calloc(nr_fields, sizeof(**fields_str));
	if (!buffer)
		return -ENOMEM;

	/* Add all common fields. */
	fields = tep_event_common_fields(event);
	if (!fields)
		goto fail;

	for (field = *fields; field; field = field->next)
		if (asprintf(&buffer[i++], "%s", field->name) <= 0)
			goto fail;

	free(fields);

	/* Add all unique fields. */
	fields = tep_event_fields(event);
	if (!fields)
		goto fail;

	for (field = *fields; field; field = field->next)
		if (asprintf(&buffer[i++], "%s", field->name) <= 0)
			goto fail;

	free(fields);

	*fields_str = buffer;
	return nr_fields;

 fail:
	for (i = 0; i < nr_fields; ++i)
		free(buffer[i]);

	free(buffer);
	return -EFAULT;
}

/**
 * Custom entry info function type. To be user for dumping info for custom
 * KernelShark entries.
 */
typedef char *(tepdata_custom_info_func)(struct kshark_data_stream *,
					const struct kshark_entry *,
					bool);

static char* tepdata_dump_custom_entry(struct kshark_data_stream *stream,
				       const struct kshark_entry *entry,
				       tepdata_custom_info_func info_func)
{
	char *entry_str;
	int size = 0;

	size = asprintf(&entry_str, "%" PRIu64 "; %s-%i; CPU %i; ; %s; %s; 0x%x",
			entry->ts,
			tep_data_comm_from_pid(kshark_get_tep(stream), entry->pid),
			entry->pid,
			entry->cpu,
			info_func(stream, entry, false),
			info_func(stream, entry, true),
			entry->visible);

	if (size > 0)
		return entry_str;

	return NULL;
}

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
/**
 * @brief Dump information of a couplebreak event into a CSV string delimited by
 * semicolons. The contents are information special to couplebreak events.
 * 
 * @param stream Data stream pointer to which an entry belongs
 * @param entry A KernelShark entry to be dumped.
 * @param get_info Whether to get special event info or just the event name.
 * @return Origin task's name an origin task's pid or name of the event.
 */
static char *couplebreak_target_dump(struct kshark_data_stream *stream,
			    const struct kshark_entry *entry,
			    bool get_info)
{
	char *buffer;
	int size = 0;

	if (get_info) {
		struct kshark_generic_stream_interface *interface = stream->interface;
		char *og_task = interface->get_task(stream, (struct kshark_entry*)entry->offset);
		size = asprintf(&buffer, "origin_task_name=%s; origin_pid=%ld",
			og_task, entry->offset);
		free(og_task);
	} else {
		char* event_name = tepdata_get_event_name(stream, entry);
		size = asprintf(&buffer, "%s", event_name);
		free(event_name);
	}

	if (size > 0)
		return buffer;
	return NULL;
}
// END of change

/**
 * @brief Dump into a string the content of one entry. The function allocates
 *	  a null terminated string and returns a pointer to this string.
 *
 * @param stream: Input location for the FTRACE data stream pointer.
 * @param entry: A Kernel Shark entry to be printed.
 *
 * @returns The returned string contains a semicolon-separated list of data
 *	    fields. The user has to free the returned string.
 */
static char *tepdata_dump_entry(struct kshark_data_stream *stream,
				const struct kshark_entry *entry)
{
	char *entry_str, *task, *latency, *event, *info;
	struct kshark_generic_stream_interface *interface;
	struct kshark_context *kshark_ctx = NULL;
	int n = 0;

	if (!kshark_instance(&kshark_ctx) || !init_thread_seq())
		return NULL;

	interface = stream->interface;
	if (!interface)
		return NULL;
	
	if (entry->event_id >= 0) {
		if (kshark_get_tep(stream)) {
			task = interface->get_task(stream, entry);
			latency = interface->aux_info(stream, entry);
			event = interface->get_event_name(stream, entry);
			info = interface->get_info(stream, entry);
			n = asprintf(&entry_str,
				     "%i; %" PRIu64 "; %s-%i; CPU %i; %s; %s; %s; 0x%x",
				     entry->stream_id,
				     entry->ts,
				     task,
				     interface->get_pid(stream, entry),
				     entry->cpu,
				     latency,
				     event,
				     info,
				     entry->visible);

			free(task);
			free(latency);
			free(event);
			free(info);
		} else {
			n = asprintf(&entry_str,
				     "%i; %" PRIu64 "; [UNKNOWN TASK]-%i; CPU %i; ; [UNKNOWN EVENT]; [NO INFO]; 0x%x",
				     entry->stream_id,
				     entry->ts,
				     interface->get_pid(stream, entry),
				     entry->cpu,
				     entry->visible);
		}

		if (n < 1)
			return NULL;
	} else {
		//NOTE: Changed here. (COUPLEBREAK) (2025-03-29)
		// Because couplebreak events' Ids are dynamically created, they cannot be
		// used in a switch statement as a case. Instead, the event_id is checked here
		// and only if couplebreak is on.
		if (stream->couplebreak_on) {
			// The entry dump will contain the same things as a normal entry, but with additional
			// information specific to couplebreak events.
			if (is_couplebreak_event(stream, entry->event_id)) {
				char *specials = tepdata_dump_custom_entry(stream, entry, couplebreak_target_dump);
				char* origin_entry_dump = tepdata_dump_entry(stream,
					(struct kshark_entry*)entry->offset);
				
				asprintf(&entry_str, "%s; %s", specials, origin_entry_dump);

				free(specials);
				free(origin_entry_dump);

				return entry_str;
			}
		}
		// END of change

		switch (entry->event_id) {
		case KS_EVENT_OVERFLOW:
			entry_str = tepdata_dump_custom_entry(stream, entry,
							     missed_events_dump);
			break;
		default:
			return NULL;
		}
	}

	return entry_str;
}

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
// Possible (EXTENSION)
/**
 * @brief Finds the event ID of a couplebreak event based on the name given
 * in the argument. Function checks for existence of a couplebreak event in
 * a stream and 
 * 
 * @param stream Data stream pointer to which an entry belongs
 * @param event_name Name of the event to be searched for
 * @return Event ID of the couplebreak event, or 0 if the event name is not
 * of a couplebreak event, or 1 if the couplebreak event does not exist in the
 * stream.
 * @note This function is a prime candidate for extensions, if couplebreak is
 * ever expanded upon.
 */
static int couplebreak_find_event_id(struct kshark_data_stream *stream,
	const char *event_name) {
	static const int COUPLEBREAK_NAME_LEN = 32;
	static const char* SCHED_SWITCH_NAME = "sched/sched_switch";
	static const char* SCHED_WAKING_NAME = "sched/sched_waking";

	bool is_sst_name = (strncmp(event_name, "couplebreak/sched_switch[target]",
		COUPLEBREAK_NAME_LEN) == 0);
	bool is_swt_name = (strncmp(event_name, "couplebreak/sched_waking[target]",
		COUPLEBREAK_NAME_LEN) == 0);
	
	if (!is_sst_name && !is_swt_name) {
		// User was not asking for a couplebreak event
		return 0;
	}
	
	// User was asking for a couplebreak event
	int result;
	// The following two lines could be a cause of concern, since we are effectively
	// performing a recursive call, which, if not handled, could be endless.
	// The above if statement exists to reslove that.
	int sched_switch_id = kshark_find_event_id(stream, SCHED_SWITCH_NAME);
	int sched_waking_id = kshark_find_event_id(stream, SCHED_WAKING_NAME);

	bool sst_exists = (stream->couplebreak_evts_flags &
		(1 << couplebreak_origin_id_to_flag_pos(stream, sched_switch_id))); 
	bool swt_exists = (stream->couplebreak_evts_flags &
		(1 << couplebreak_origin_id_to_flag_pos(stream, sched_waking_id)));

	// If such events exist, return the couplebreak event ID according to the name
	if (sst_exists && is_sst_name) {
		result = find_couplebreak_event_id_from_origin_name(stream, SCHED_SWITCH_NAME);
	} else if (swt_exists && is_swt_name) {
		result = find_couplebreak_event_id_from_origin_name(stream, SCHED_WAKING_NAME);
	} else { // Couldn't find ID of said name - events do not exist in the stream
		result = 1;
	}

	return result;
}

static int tepdata_find_event_id(struct kshark_data_stream *stream,
				 const char *event_name)
{
	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	if (stream->couplebreak_on) {
		int couplebreak_id = couplebreak_find_event_id(stream, event_name);
		switch (couplebreak_id) {
		case 0: // User was not asking for a couplebreak event
			break;
		case 1: // Couplebreak event does not exist in the stream
			return -1;
		default:
			return couplebreak_id;
		}
	}
	// END of change
	
	struct tep_event *event;
	char *buffer, *system, *name;

	if (asprintf(&buffer, "%s", event_name) < 1)
		return -1;

	system = strtok(buffer, "/");
	name = strtok(NULL, "");
	if (!system || !name)
		return -1;

	event = tep_find_event_by_name(kshark_get_tep(stream), system, name);

	free(buffer);

	if (!event)
		return -1;

	return event->id;
}

static struct tep_format_field *
get_evt_field(struct kshark_data_stream *stream,
	      int event_id, const char *field_name)
{
	struct tep_event *event = tep_find_event(kshark_get_tep(stream),
						 event_id);
	if (!event)
		return NULL;

	return tep_find_any_field(event, field_name);
}

/**
 * @brief  Get the type of a trace record field. For the moment only integer
 *	   fields are supported.
 *
 * @param stream: Input location for the FTRACE data stream pointer.
 * @param entry: Input location for the Kernel Shark entry asociated with thes
 *		 record.
 * @param field: The name of the field.
 *
 * @returns KS_INTEGER_FIELD in case the field has an integer type. Otherwise
 *	    KS_INVALID_FIELD.
 */
kshark_event_field_format
tepdata_get_field_type(struct kshark_data_stream *stream,
		       const struct kshark_entry *input_entry,
		       const char *field)
{
	struct tep_format_field *evt_field;
	int mask = ~(TEP_FIELD_IS_SIGNED |
		     TEP_FIELD_IS_LONG |
		     TEP_FIELD_IS_FLAG);

	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	/*
		Couplebreak target events are basically just a copy of origin entry, but
		they don't have the same values, like pid, timestamp, maybe even CPU, but
		especially not the same offset field value, which is completely changed to
		store the origin entry's address.

		Hence, accomodations are done, by asking for the origin entry's fields instead, as they
		are the same (couplebreak event is a slightly altered twin for the origin event).
		It would be a fruitless search for the program, if the made-up
		couplebreak entries were used instead, as they do not exist in the trace file.
	*/
	const struct kshark_entry *entry = input_entry;
	if (stream->couplebreak_on) {
		if (is_couplebreak_event(stream, entry->event_id)) {
			entry = (const struct kshark_entry*)input_entry->offset;
		}
	}
	// END of change

	evt_field = get_evt_field(stream, entry->event_id, field);
	if (!evt_field)
		return KS_INVALID_FIELD;

	if (mask & evt_field->flags)
		return KS_INVALID_FIELD;

	return KS_INTEGER_FIELD;
}

/**
 * @brief  Get the value of a trace record field.
 *
 * @param stream: Input location for the FTRACE data stream pointer.
 * @param rec: Input location for the trace record.
 * @param field: The name of the field.
 * @param val: Output location for the field value.
 *
 * @returns Returns 0 on success, otherwise a negative error code..
 */
int tepdata_read_record_field(struct kshark_data_stream *stream,
			      void *rec,
			      const char *field, int64_t *val)
{
	struct tep_format_field *evt_field;
	struct tep_record *record = rec;
	int event_id, ret;

	if (!record)
		return -EFAULT;

	event_id = tep_data_type(kshark_get_tep(stream), record);
	evt_field = get_evt_field(stream, event_id, field);
	if (!evt_field)
		return -EINVAL;

	ret = tep_read_number_field(evt_field, record->data,
				    (unsigned long long *) val);

	return ret;
}

/**
 * @brief  Get the value of a trace record field.
 *
 * @param stream: Input location for the FTRACE data stream pointer.
 * @param entry: Input location for the Kernel Shark entry asociated with thes
 *		 record.
 * @param field: The name of the field.
 * @param val: Output location for the field value.
 *
 * @returns Returns 0 on success, otherwise a negative error code.
 */
int tepdata_read_event_field(struct kshark_data_stream *stream,
			     const struct kshark_entry *input_entry,
			     const char *field, int64_t *val)
{
	struct tep_format_field *evt_field;
	struct tep_record *record;
	int ret;

	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	/*
		Couplebreak target events are basically just a copy of origin entry, but
		they don't have the same values, like pid, timestamp, maybe even CPU, but
		especially not the same offset field value, which is completely changed to
		store the origin entry's address.

		Hence, accomodations are done, by asking for the origin entry's fields instead, as they
		are the same (couplebreak event is a slightly altered twin for the origin event).
		It would be a fruitless search for the program, if the made-up
		couplebreak entries were used instead, as they do not exist in the trace file.
	*/

	const struct kshark_entry *entry = input_entry;
	if (stream->couplebreak_on) {
		if (is_couplebreak_event(stream, entry->event_id)) {
			entry = (const struct kshark_entry*)input_entry->offset;
		}
	}
	// END of change

	evt_field = get_evt_field(stream, entry->event_id, field);
	if (!evt_field)
		return -EINVAL;

	record = tracecmd_read_at(kshark_get_tep_input(stream),
		entry->offset, NULL);
	if (!record)
		return -EFAULT;

	ret = tep_read_number_field(evt_field, record->data,
				    (unsigned long long *) val);
	tracecmd_free_record(record);

	return ret;
}

//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
/**
 * @brief Gets all couplebreak event IDs from a stream as an integer
 * array. Only the Ids of detected couplebreak events in a stream will be returned.
 * 
 * @param stream Data stream pointer in which to search for couplebreak events
 * @return Array of couplebreak event IDs, or NULL if no such events exist in the stream.
 * Its size will always be stream->n_couplebreak_evts & the array must be freed by the caller.
 */
static int *couplebreak_get_all_ids(struct kshark_data_stream *stream)
{
	int *ids = NULL;
	if (stream->couplebreak_on) {
		ids = calloc(stream->n_couplebreak_evts, sizeof(*ids));
		int i = 0;
		for (int j = 0; j < stream->n_couplebreak_evts; j++) {
			if (stream->couplebreak_evts_flags & (1 << j)) {
				ids[i] = flag_pos_to_couplebreak_id(stream, j);
				i++;
				// This shouldn't really happen, since the flags are set once, just like increment is done once.
				// But it never hurts to be careful.
				if (i == stream->n_couplebreak_evts)
					break;
			}
		}
	}
	return ids;
}

/** Initialize all methods used by a stream of FTRACE data. */
static void kshark_tep_init_methods(struct kshark_generic_stream_interface *interface)
{
	if (!interface)
		return;
	
	interface->get_pid = tepdata_get_pid;
	interface->get_task = tepdata_get_task;
	interface->get_event_id = tepdata_get_event_id;
	interface->get_event_name = tepdata_get_event_name;
	interface->aux_info= tepdata_get_latency;
	interface->get_info = tepdata_get_info;
	interface->find_event_id = tepdata_find_event_id;
	interface->get_all_event_ids = tepdata_get_event_ids;
	interface->dump_entry = tepdata_dump_entry;
	interface->get_all_event_field_names = tepdata_get_field_names;
	interface->get_event_field_type = tepdata_get_field_type;
	interface->read_record_field_int64 = tepdata_read_record_field;
	interface->read_event_field_int64 = tepdata_read_event_field;
	interface->load_entries = tepdata_load_entries;
	interface->load_matrix = tepdata_load_matrix;
	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	// Added couplebreak specific function.
	interface->get_couplebreak_ids = couplebreak_get_all_ids;
	// END of change
}

/** A list of built in default plugins for FTRACE (trace-cmd) data. */
const char *tep_plugin_names[] = {
	"sched_events",
	"missed_events",
	"kvm_combo",
};

/**
 * Register to the data stream all default plugins for FTRACE (trace-cmd) data.
 */
int kshark_tep_handle_plugins(struct kshark_context *kshark_ctx, int sd)
{
	struct kshark_plugin_list *plugin;
	struct kshark_data_stream *stream;
	int i, n_tep_plugins;

	n_tep_plugins = (sizeof(tep_plugin_names) / sizeof((tep_plugin_names)[0]));
	stream = kshark_get_data_stream(kshark_ctx, sd);
	if (!stream)
		return -EEXIST;

	for (i = 0; i < n_tep_plugins; ++i) {
		plugin = kshark_find_plugin_by_name(kshark_ctx->plugins,
						    tep_plugin_names[i]);

		if (plugin && plugin->process_interface) {
			kshark_register_plugin_to_stream(stream,
							 plugin->process_interface,
							 true);
		} else {
			fprintf(stderr, "Plugin \"%s\" not found.\n",
				tep_plugin_names[i]);
		}
	}

	return kshark_handle_all_dpis(stream, KSHARK_PLUGIN_INIT);
}

/** The Process Id of the Idle tasks is zero. */
#define LINUX_IDLE_TASK_PID	0

static int kshark_tep_stream_init(struct kshark_data_stream *stream,
				  struct tracecmd_input *input)
{
	struct kshark_generic_stream_interface *interface;
	struct tepdata_handle *tep_handle;
	struct tep_event *event;

	stream->interface = interface = calloc(1, sizeof(*interface));
	if (!interface)
		return -ENOMEM;

	interface->type = KS_GENERIC_DATA_INTERFACE;

	tep_handle = calloc(1, sizeof(*tep_handle));
	if (!tep_handle)
		goto fail;

	tep_handle->input = input;
	tep_handle->tep = tracecmd_get_tep(tep_handle->input);
	if (!tep_handle->tep)
		goto fail;

	tep_handle->sched_switch_event_id = -EINVAL;
	event = tep_find_event_by_name(tep_handle->tep,
				       "sched", "sched_switch");
	if (event) {
		tep_handle->sched_switch_event_id = event->id;

		tep_handle->sched_switch_next_field =
			tep_find_any_field(event, "next_pid");

		tep_handle->sched_switch_comm_field =
			tep_find_field(event, "next_comm");
	}

	stream->n_cpus = tep_get_cpus(tep_handle->tep);
	stream->n_events = tep_get_events_count(tep_handle->tep);
	stream->idle_pid = LINUX_IDLE_TASK_PID;
	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	stream->couplebreak_on = false;
	stream->n_couplebreak_evts = 0;
	stream->couplebreak_evts_flags = 0;
	// END of change
	
	tep_handle->advanced_event_filter =
		tep_filter_alloc(tep_handle->tep);

	kshark_tep_init_methods(interface);

	interface->handle = tep_handle;

	return 0;

 fail:
	free(tep_handle);
	free(interface);
	stream->interface = NULL;
	return -EFAULT;
}

static inline char *set_tep_format(struct kshark_data_stream *stream)
{
	return kshark_set_data_format(stream->data_format,
				      TEP_DATA_FORMAT_IDENTIFIER);
}

static struct tracecmd_input *get_top_input(struct kshark_context *kshark_ctx,
					    int sd)
{
	struct kshark_data_stream *top_stream;

	top_stream = kshark_get_data_stream(kshark_ctx, sd);
	if (!top_stream)
		return NULL;

	return kshark_get_tep_input(top_stream);
}

/**
 * @brief Get an array containing the names of all buffers in FTRACE data
 *	  file.
 *
 * @param kshark_ctx: Input location for context pointer.
 * @param sd: Data stream identifier of the top buffers in the FTRACE data
 *	  file.
 * @param n_buffers: Output location for the size of the outputted array,
 *	    or a negative error code on failure.
 *
 * @returns Array of strings on success, or NULL on failure. The user is
 *	    responsible for freeing the elements of the outputted array.
 */
char **kshark_tep_get_buffer_names(struct kshark_context *kshark_ctx, int sd,
				   int *n_buffers)
{
	struct tracecmd_input *top_input;
	char **buffer_names;
	int i, n;

	top_input = get_top_input(kshark_ctx, sd);
	if (!top_input) {
		*n_buffers = -EFAULT;
		return NULL;
	}

	n = tracecmd_buffer_instances(top_input);
	buffer_names = calloc(n, sizeof(char *));
	if (!buffer_names) {
		*n_buffers = -ENOMEM;
		return NULL;
	}

	for (i = 0; i < n; ++i) {
		buffer_names[i] =
			strdup(tracecmd_buffer_instance_name(top_input, i));
		if (!buffer_names[i])
			goto free_all;
	}

	*n_buffers = n;
	return buffer_names;

 free_all:
	for (i = 0; i < n; ++i)
		free(buffer_names[i]);
	free(buffer_names);

	*n_buffers = -ENOMEM;
	return NULL;
}

static void set_stream_fields(struct tracecmd_input *top_input, int i,
			      const char *file,
			      const char *name,
			      struct kshark_data_stream *buffer_stream,
			      struct tracecmd_input **buffer_input)
{
	*buffer_input = tracecmd_buffer_instance_handle(top_input, i);

	buffer_stream->name = strdup(name);
	buffer_stream->file = strdup(file);
	set_tep_format(buffer_stream);
}

/**
 * @brief Open a given buffers in FTRACE (trace-cmd) data file.
 *
 * @param kshark_ctx: Input location for context pointer.
 * @param sd: Data stream identifier of the top buffers in the FTRACE data
 *	  file.
 * @param buffer_name: The name of the buffer to open.
 *
 * @returns Data stream identifier of the buffer on success. Otherwise a
 *	    negative error code.
 */
int kshark_tep_open_buffer(struct kshark_context *kshark_ctx, int sd,
			   const char *buffer_name)
{
	struct kshark_data_stream *top_stream, *buffer_stream;
	struct tracecmd_input *top_input, *buffer_input;
	int i, sd_buffer, n_buffers, ret = -ENODATA;
	char **names;

	top_stream = kshark_get_data_stream(kshark_ctx, sd);
	if (!top_stream)
		return -EFAULT;

	top_input = kshark_get_tep_input(top_stream);
	if (!top_input)
		return -EFAULT;

	names = kshark_tep_get_buffer_names(kshark_ctx, sd, &n_buffers);
	if (!names)
		return n_buffers;

	sd_buffer = kshark_add_stream(kshark_ctx);
	buffer_stream = kshark_get_data_stream(kshark_ctx, sd_buffer);
	if (!buffer_stream) {
		ret = -EFAULT;
		goto end;
	}

	for (i = 0; i < n_buffers; ++i) {
		if (strcmp(buffer_name, names[i]) == 0) {
			set_stream_fields(top_input, i,
					  top_stream->file,
					  buffer_name,
					  buffer_stream,
					  &buffer_input);

			if (!buffer_stream->name || !buffer_stream->file) {
				free(buffer_stream->name);
				free(buffer_stream->file);
				buffer_stream->name = NULL;
				buffer_stream->file = NULL;
				ret = -ENOMEM;
				break;
			}

			ret = kshark_tep_stream_init(buffer_stream,
						     buffer_input);
			break;
		}
	}

end:
	for (i = 0; i < n_buffers; ++i)
		free(names[i]);
	free(names);

	return (ret < 0)? ret : buffer_stream->stream_id;
}

/**
 * @brief Initialize data streams for all buffers in a FTRACE (trace-cmd) data
 *	  file.
 *
 * @param kshark_ctx: Input location for context pointer.
 * @param sd: Data stream identifier of the top buffers in the FTRACE data
 *	  file.
 *
 * @returns The total number of data streams initialized on success. Otherwise
 *	    a negative error code.
 */
int kshark_tep_init_all_buffers(struct kshark_context *kshark_ctx,
				int sd)
{
	struct kshark_data_stream *top_stream, *buffer_stream;
	struct tracecmd_input *buffer_input;
	struct tracecmd_input *top_input;
	int i, n_buffers, sd_buffer, ret;

	top_stream = kshark_get_data_stream(kshark_ctx, sd);
	if (!top_stream)
		return -EFAULT;

	top_input = kshark_get_tep_input(top_stream);
	if (!top_input)
		return -EFAULT;

	n_buffers = tracecmd_buffer_instances(top_input);
	for (i = 0; i < n_buffers; ++i) {
		sd_buffer = kshark_add_stream(kshark_ctx);
		if (sd_buffer < 0)
			return -EFAULT;

		buffer_stream = kshark_ctx->stream[sd_buffer];

		set_stream_fields(top_input, i,
				  top_stream->file,
				  tracecmd_buffer_instance_name(top_input, i),
				  buffer_stream,
				  &buffer_input);

		if (!buffer_stream->name || !buffer_stream->file) {
			free(buffer_stream->name);
			free(buffer_stream->file);
			buffer_stream->name = NULL;
			buffer_stream->file = NULL;
			return -ENOMEM;
		}

		ret = kshark_tep_stream_init(buffer_stream, buffer_input);
		if (ret != 0)
			return -EFAULT;
	}

	return n_buffers;
}

/** Is this a stream corresponding to the "top" buffer in the file. */
bool kshark_tep_is_top_stream(struct kshark_data_stream *stream)
{
	return strcmp(stream->name, KS_UNNAMED) == 0;
}

/** Check is the file contains TEP tracing data. */
bool kshark_tep_check_data(const char *file_name)
{
	/*
	 * TODO: This is very naive. Implement more appropriate check. Ideally
	 * it should be part of the trace-cmd library.
	 */
	char *ext = strrchr(file_name, '.');
	if (ext && strcmp(ext, ".dat") == 0) {
		return true;
	}

	return false;
}

/** Initialize the FTRACE data input (from file). */
int kshark_tep_init_input(struct kshark_data_stream *stream)
{
	struct kshark_context *kshark_ctx = NULL;
	struct tracecmd_input *input;

	if (!kshark_instance(&kshark_ctx) || !init_thread_seq())
		return -EEXIST;

	/*
	 * Turn off function trace indent and turn on show parent
	 * if possible.
	 */
	tep_plugin_add_option("ftrace:parent", "1");
	tep_plugin_add_option("ftrace:indent", "0");

	input = tracecmd_open_head(stream->file, 0);
	if (!input)
		return -EEXIST;

	/* Read the tracing data from the file. */
	if (tracecmd_init_data(input) < 0)
		goto fail;

	/* Initialize the stream asociated with the main buffer. */
	if (kshark_tep_stream_init(stream, input) < 0)
		goto fail;

	stream->name = strdup(KS_UNNAMED);

	return 0;

 fail:
	tracecmd_close(input);
	return -EFAULT;
}

/** Initialize using the locally available tracing events. */
int kshark_tep_init_local(struct kshark_data_stream *stream)
{
	struct kshark_generic_stream_interface *interface;
	struct tepdata_handle *tep_handle;

	stream->interface = interface = calloc(1, sizeof(*interface));
	if (!interface)
		return -ENOMEM;

	interface->type = KS_GENERIC_DATA_INTERFACE;

	tep_handle = calloc(1, sizeof(*tep_handle));
	if (!tep_handle)
		goto fail;

	tep_handle->tep = tracefs_local_events(tracefs_tracing_dir());
	if (!tep_handle->tep)
		goto fail;

	stream->n_events = tep_get_events_count(tep_handle->tep);
	stream->n_cpus =  tep_get_cpus(tep_handle->tep);
	//NOTE: Changed here. (COUPLEBREAK) (2025-03-21)
	stream->couplebreak_on = false;
	stream->n_couplebreak_evts = 0;
	stream->couplebreak_evts_flags = 0;
	// END of change

	set_tep_format(stream);
	if (asprintf(&stream->file, "Local system") <= 0)
		goto fail;

	interface->handle = tep_handle;
	kshark_tep_init_methods(interface);

	return 0;

 fail:
	free(tep_handle);
	free(interface);
	stream->interface = NULL;
	return -EFAULT;
}

/** Method used to close a stream of FTRACE data. */
int kshark_tep_close_interface(struct kshark_data_stream *stream)
{
	struct kshark_generic_stream_interface *interface = stream->interface;
	struct tepdata_handle *tep_handle;

	if (!interface)
		return -EFAULT;

	tep_handle = interface->handle;
	if (!tep_handle)
		return -EFAULT;

	if (seq.buffer) {
		trace_seq_destroy(&seq);
		seq.buffer = NULL;
	}

	if (tep_handle->advanced_event_filter) {
		tep_filter_reset(tep_handle->advanced_event_filter);
		tep_filter_free(tep_handle->advanced_event_filter);
		tep_handle->advanced_event_filter = NULL;
	}

	if (tep_handle->input)
		tracecmd_close(tep_handle->input);

	free(tep_handle);
	interface->handle = NULL;

	return 0;
}

/** Check if the filter any filter is set. */
bool kshark_tep_filter_is_set(struct kshark_data_stream *stream)
{
	struct tep_event_filter *adv_filter = get_adv_filter(stream);

	if (adv_filter && adv_filter->filters)
		return true;

	return false;
}

/**
 * @brief Add a filter based on the content of the event.
 *
 * @param stream: Input location for the FTRACE data stream pointer.
 * @param filter_str: The definition of the filter.
 *
 * @returns 0 if the filter was successfully added or a negative error code.
 */
int kshark_tep_add_filter_str(struct kshark_data_stream *stream,
			       const char *filter_str)
{
	struct tep_event_filter *adv_filter = get_adv_filter(stream);
	int ret = tep_filter_add_filter_str(adv_filter, filter_str);

	if (ret < 0) {
		char error_str[200];
		int error_status =
			tep_strerror(kshark_get_tep(stream), ret, error_str,
				     sizeof(error_str));

		if (error_status == 0)
			fprintf(stderr, "filter failed due to: %s\n",
					error_str);
	}

	return ret;
}

/**
 * @brief Get a string showing the filter definition.
 *
 * @param stream: Input location for the FTRACE data stream pointer.
 * @param event_id: The unique Id of the event type of the filter.
 *
 * @returns A string that displays the filter contents. This string must be
 *	    freed with free(str). NULL is returned if no filter is found or
 *	    allocation failed.
 */
char *kshark_tep_filter_make_string(struct kshark_data_stream *stream,
				    int event_id)
{
	struct tep_event_filter *adv_filter = get_adv_filter(stream);

	return tep_filter_make_string(adv_filter, event_id);
}

/**
 * @brief Remove a filter based on the content of the event.
 *
 * @param stream: Input location for the FTRACE data stream pointer.
 * @param event_id: The unique Id of the event type of the filter.
 *
 * @return 1: if an event was removed or 0 if the event was not found.
 */
int kshark_tep_filter_remove_event(struct kshark_data_stream *stream,
				   int event_id)
{
	struct tep_event_filter *adv_filter = get_adv_filter(stream);

	return tep_filter_remove_event(adv_filter, event_id);
}

/** Reset all filters based on the content of the event. */
void kshark_tep_filter_reset(struct kshark_data_stream *stream)
{
	return tep_filter_reset(get_adv_filter(stream));
}

/** Get an array of available tracer plugins. */
char **kshark_tracecmd_local_plugins()
{
	return tracefs_tracers(tracefs_tracing_dir());
}

void kshark_tracecmd_plugin_list_free(char **list)
{
	tracefs_list_free(list);
}

/**
 * @brief Free an array, allocated by kshark_tracecmd_get_hostguest_mapping() API
 *
 *
 * @param map: Array, allocated by kshark_tracecmd_get_hostguest_mapping() API
 * @param count: Number of entries in the array
 *
 */
void kshark_tracecmd_free_hostguest_map(struct kshark_host_guest_map *map, int count)
{
	int i;

	if (!map)
		return;
	for (i = 0; i < count; i++) {
		free(map[i].guest_name);
		free(map[i].cpu_pid);
		memset(&map[i], 0, sizeof(*map));
	}
	free(map);
}

/**
 * @brief Get mapping of guest VCPU to host task, running that VCPU.
 *	  Array of mappings for each guest is allocated and returned
 *	  in map input parameter.
 *
 *
 * @param map: Returns allocated array of kshark_host_guest_map structures, each
 *	       one describing VCPUs mapping of one guest.
 *
 * @return The number of entries in the *map array, or a negative error code on
 *	   failure.
 */
int kshark_tracecmd_get_hostguest_mapping(struct kshark_host_guest_map **map)
{
	struct kshark_host_guest_map *gmap = NULL;
	struct tracecmd_input *peer_handle = NULL;
	struct kshark_data_stream *peer_stream;
	struct tracecmd_input *guest_handle = NULL;
	struct kshark_data_stream *guest_stream;
	struct kshark_context *kshark_ctx = NULL;
	unsigned long long trace_id;
	const char *name;
	int vcpu_count;
	const int *cpu_pid;
	int *stream_ids;
	int i, j, k;
	int count = 0;
	int ret;

	if (!map || !kshark_instance(&kshark_ctx))
		return -EFAULT;
	if (*map)
		return -EEXIST;

	stream_ids = kshark_all_streams(kshark_ctx);
	for (i = 0; i < kshark_ctx->n_streams; i++) {
		guest_stream = kshark_get_data_stream(kshark_ctx, stream_ids[i]);
		if (!guest_stream || !kshark_is_tep(guest_stream))
			continue;
		guest_handle = kshark_get_tep_input(guest_stream);
		if (!guest_handle)
			continue;
		trace_id = tracecmd_get_traceid(guest_handle);
		if (!trace_id)
			continue;
		for (j = 0; j < kshark_ctx->n_streams; j++) {
			if (stream_ids[i] == stream_ids[j])
				continue;
			peer_stream = kshark_get_data_stream(kshark_ctx, stream_ids[j]);
			if (!peer_stream || !kshark_is_tep(guest_stream))
				continue;
			peer_handle = kshark_get_tep_input(peer_stream);
			if (!peer_handle)
				continue;
			ret = tracecmd_get_guest_cpumap(peer_handle, trace_id,
							&name, &vcpu_count, &cpu_pid);
			if (!ret && vcpu_count) {
				gmap = realloc(*map,
					       (count + 1) * sizeof(struct kshark_host_guest_map));
				if (!gmap)
					goto mem_error;
				*map = gmap;
				memset(&gmap[count], 0, sizeof(struct kshark_host_guest_map));
				count++;
				gmap[count - 1].guest_id = stream_ids[i];
				gmap[count - 1].host_id = stream_ids[j];
				gmap[count - 1].guest_name = strdup(name);
				if (!gmap[count - 1].guest_name)
					goto mem_error;
				gmap[count - 1].vcpu_count = vcpu_count;
				gmap[count - 1].cpu_pid = malloc(sizeof(int) * vcpu_count);
				if (!gmap[count - 1].cpu_pid)
					goto mem_error;
				for (k = 0; k < vcpu_count; k++)
					gmap[count - 1].cpu_pid[k] = cpu_pid[k];
				break;
			}
		}
	}

	free(stream_ids);
	return count;

mem_error:
	free(stream_ids);
	if (*map) {
		kshark_tracecmd_free_hostguest_map(*map, count);
		*map = NULL;
	}

	return -ENOMEM;
}

/**
 * @brief Find the data stream corresponding the top buffer of a FTRACE
 *	  (trace-cmd) data file.
 *
 * @param kshark_ctx: Input location for context pointer.
 * @param file: The name of the file.
 *
 * @returns Data stream identifier of the top buffers in the FTRACE data
 *	    fileon success. Otherwise a negative error code.
 */
int kshark_tep_find_top_stream(struct kshark_context *kshark_ctx,
			       const char *file)
{
	struct kshark_data_stream *top_stream = NULL, *stream;
	int i, *stream_ids = kshark_all_streams(kshark_ctx);

	for (i = 0; i < kshark_ctx->n_streams; ++i) {
		stream = kshark_ctx->stream[stream_ids[i]];
		if (strcmp(stream->file, file) == 0 &&
		    kshark_tep_is_top_stream(stream))
			top_stream = stream;
	}

	free(stream_ids);

	if (!top_stream)
		return -EEXIST;

	return top_stream->stream_id;
}

static bool find_wakeup_event(struct tep_handle *tep, const char *wakeup_name,
			      struct tep_event **waking_event_ptr)
{
	struct tep_event *event;

	event = tep_find_event_by_name(tep, "sched", wakeup_name);

	if (event)
		*waking_event_ptr = event;

	return !!event;
}

/**
 * @brief Search the available trace events and retrieve a definition of
 *	  a waking_event.
 *
 * @param tep: Input location for the the Page event object.
 * @param waking_event_ptr: Output location for the the waking_event object.
 *
 * @returns True on success, otherwise False.
 */
bool define_wakeup_event(struct tep_handle *tep,
			 struct tep_event **waking_event_ptr)
{
	bool wakeup_found;

	wakeup_found = find_wakeup_event(tep, "sched_wakeup",
					 waking_event_ptr);

	wakeup_found |= find_wakeup_event(tep, "sched_wakeup_new",
					  waking_event_ptr);

	wakeup_found |= find_wakeup_event(tep, "sched_waking",
					  waking_event_ptr);

	return wakeup_found;
}
