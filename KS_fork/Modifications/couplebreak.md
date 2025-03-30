# Purpose

Allow splitting of events, which involve two processes (a "couple" by definition of this modification),
namely `sched/sched_switch` and `sched/sched_waking`. Update plugins that could leverage this new functionality
(especially `sched_events`).

# Main design objectives

- Almost-transparent modifications
  - No modification should change what already works, only
    when couplebreak is active should the feature do its work.
  - Same behviour if off as if it was never there
- Entries & stream interface support
- Easy to use by a programmer and a user
- As similar to a regular event as possible
  - Exceptions: must not change offset & event_id fields
- Sessions support
- Serve as a compatibility bridge between some plugins

# Solution

(Read more in sections below.)

Source code change tag: `COUPLEBREAK`. Some changes are marked with `"COUPLEBREAK"`, which means there were changes
done during couplebreak, but not necessarily limited to it (e.g. typedef of a custom_entry_creation_func function type).

## Design

### General overview

Minimal additional API created, only functions along the lines of "getters" instead of "setters". The newly created
header file only has supported event Ids. New file to include if expecting to work with couplebreak, namely 
`libkshark-couplebreak.h`, with its own implementation file to keep thngs tidy.

Almost every possible couplebreak-related call guarded by an if statement asking if couplebreak is enabled.
If couplebreak is showing itself off to the world (header files inclusions), it's always explicitly something with
"couplebreak" in its name.

A lot of changes are in `libkshark-tepdata.c`, where events are loaded - exactly where couplebreak needs to operate.
This is also where the stream interface gets defined and also where couplebreak injects changes to these interface
calls - couplebreak events are handled in a special way each time.

Though the architecture most likely resembles a Big Ball of Mud, any changes to existing implementation must never break
it, that is the most important goal.

### Couplebreak events, couples, integration with existing code.

The main concepts here were couples and couplebreak events/entries. Couples, as mentioned
above, are two processes, which "share" an event. For an example, a scheduler switch event will have a task that is being
switched from and a task that is being switched to. These two tasks are a couple. Couplebreak entries (also called "target 
entries" or "target events", sometimes with "waking" or "switch", which denote the origin events - "sched/sched_waking" 
and "sched/sched_switch" respectively) are sort of altered twins to the actual events. They contain almost all the same
data, they will attempt to behave as any other event entry, but they were never captured in the trace. They happened at the
same time, but belong to different tasks (sometimes to different CPUs as well).

User must be able to work with couplebreak events as if they were real events in the basic ways, which implies
integration with stream interfaces.

### Session support

As couplebreak will be a setting per stream, sessions should be compatible with this new functionality. Exporting one
must export couplebreak settings, if loading one without it, it can be set to off, as changing this is simple.

## Implementation

### Couplebreak events creation

All couplebreak events are created in the `get_records` function. They are created after their origin event, if stream is
set to create couplebreak events.

As custom events, they abide by the rule written in the source code, stating that custom entries must have a negative
event Id. Every couplebreak event has event Id lesser than or equal to -10 000 (negative ten thousand). The macros with supported couplebreak event Ids are included in the header file `libkshark-couplebreak.h`, so that others including this file can detect couplebreak events.

To have some connection to their origin event, the couplebreak entries replace their offset field's value with a pointer
to the origin entry. This then allows them to redirect some requests to them or to get data they otherwise wouldn't have
access to, because they are missing their offset.

Upon name request, each couplebreak event will generate a name of this format: `couplebreak/[ORIGIN EVENT NAME][target]`.
\[ORIGIN EVENT NAME\] will be something like "sched_switch" or "sched_waking", missing their usual prefixes. The suffix
"\[target\]" appears as written.

Couplebreak entries will try to follow their origin event's intentions with the next task. Scheduler switches contain
information on what task will run on the same CPU next, so the couplebreak event gets the next PID and makes it his owner.
Scheduler's waking event also says on which task the awakened should run (but that's not definitive, more below), so
the target entries change their PID and their CPU to values that are given in the closest target entry of a switch to
the awakened task (these include definitive information).

They are marked as visible in the graph and mimick normal events' behaviour to the best of their ability.

#### Targets for sched/sched_waking

Waking events only contain a "target_cpu" field, which is a request to where to be awoken next, but not an order. If the
scheduler chooses, it can migrate the task to a different CPU, multiple times before having the task truly start working
after a switch. Scheduler's switches contain only the end information, which is what couplebreak needs to get. As such,
couplebreak sorts all entries after they've been loaded into an array sorted by timestamps (reusing approach done in
some layers that call `get_records`) and finds the actual CPU a task will run on and puts that into the CPU field of
the waking target event.

This requires a copy of the record (linked) list, iteration through it, where it's mutated throughout the iteration,
and also allocating memory for all the entries to be in the sorted array. Thankfully, the only overhead here is the
linked list copy, which is necessary for proper iteration and not changing values of the actual list that will be used
later by KernelShark. The sorted array will be freed at the end - though in most functions calling `get_records`, the
same array will bre created later, so the memory suffered basically nothing.

This happens once per load of data, so the time spent here is mostly negligible (unless loading a massive file).

### Couplebreak API

A small API was also created to help users work with couplebreak. All functions provide a read-like functionality,
so no writes to any variable are done. They are as follows:
- *couplebreak_get_origin*: returns the origin entry of a couplebreak entry
- *couplebreak_origin_id_to_flag_pos*: returns a flag position in the stream's couplebreak flag variable
- *couplebreak_id_to_flag_pos*: returns a flag position based on the couplebreak event Id
- *flag_pos_to_couplebreak_id*: returns a couplebreak event Id based on the flag position
- *is_couplebreak_event*: returns whether or not the event entry given is a couplebreak event
- *get_couplebreak_event_name*: returns a couplebreak event's name

There are also macros with values relevant to couplebreak, namely the couplebreak event Ids and flag positions.
These could have been static const variables, but macros can be used in switch statements, making for prettier code.

### Data stream API

There have been additional changes made to the stream interface methods, whose behaviour changes only for couplebreak
events as follows:
- Getting original PID reconstructs it for the entry.
- Getting event name constructs one on demand.
- Latency of couplebreak events is the latency of their origin events.
- Info of couplebreak events is redirected to info of their origin events.
- Finding couplebreak entry's event Id through its name will construct one on demand.
- Getting all event Ids through the regular `kshark_get_all_event_ids` (and the stream interface method)
  does NOT return couplebreak event Ids. Use the newly added `kshark_get_couplebreak_ids` instead.
  - This was one due to KernelShark expecting only as many events as in the trace data in many of its implementations - 
    because couplebreak events are on load, this would be inconsistent (and far too much work for this
    extension modification) - hence stream's two of the three new state variables have been introduced and they are used
    instead when talking about couplebreak.
- Dumping an entry will dump the couplebreak entry, with a prefix of the dump being the original task and original task's
  PID.
- Getting a field name, field type, field value from the entry or the trace record will redirect the requests to
  the origin event.
- Other interface functions have not been changed. 

### GUI

GUI only gets a single button in the `Tools` menu, `Couplebreak Settings`, which will show a `KsCouplebreakDialog` window
dialog, which contains explanation of couplebreak and a list of currently open streams and if couplebreak is enabled
in them (couplebreak works per stream, so there is a checkbox per stream). Applying changes will reload the graph
and plugins to have couplebreak take effect.

### Sessions

Sessions are a way to store configured environment in KernelShark for easier future load. They are a little buggy, but
they shoul know of and use couplebreak, as it is now an important stream state variable, at the very least if it is on
or not. And so, there were some additions done to `libkshark-configio.c`, mostly an added static function for importing
the couplebreak on/off state in a stream from a JSON file and a bunch of additions to functions about exporting settings
and importing them.

Couplebreak is fully compatible with sessions.

### Existing plugins change

The only plugin changed (or the one who seemed that they needed it) was `sched_events`. It could leverage target events
instead of changing the owner PID of `sched/sched_switch` events and it works with couplebreak like a charm.

## Extensions

- Some couplebreak functions could be moved into a header file - currently, most of them are static functions hidden in
  `libkshark-tepdata.c`, where they have access to most of their needs, but at the cost of not being callable anywhere 
  else.
- If there was a way to reuse the sorted array created when correcting CPUs of waking target events, it could erase
  creating the same array twice in some situations.
- More events could support be supported, e.g. `sched/sched_migrate_task` could be split between the original CPU
  and the chosen CPU for migration (though this event doesn't have much to do with two processes, it does have a
  CPU "couple").
- Another way to assign a range of numbers to couplebreak events is to for example get the origin event Ids and then
  shift them all. This woul be useful if the reverse situation, that is getting the origin event's Id from the target was 
  necessary, but we couldn't access anything other than the target event's Id and our knowledge of this shifting. As
  this situation would be incredibly rare and because being explicit is usually better, the explicit listing of event Ids
  was chosen.
- The `get_all_event_ids` stream interface function cannot do what its name suggests due to couplebreak events just being
  custom made. If one found a way to connect the `n_couplebreak_evts` variable with the `n_events` variable in streams,
  it could be a less fractured way to work with couplebreak.

# Usage

## GUI

All a GUI user has to do is navigate to the `Tools` menu and click on `Couplebreak Settings`, which should be just below
the `Record` button. A configuration window for each stream will pop up, along with an explanation of this modification.

Checking the checkboxes for which couplebreak should activate and applying them will have the affected streams insert new
couplebreak events, which can be filtered, selected, found and interacted with.

Couplebreak entries can be filtered by the simple checkbox filter as any other event, advanced filtering is currently
NOT supported. The entries will be visible in the main window's list view and in the graph. They might overlap with
their origin events, but even if that happens, their nature as a "twin" of the origin event makes this a non-issue.

## API

### Couplebreak API

Check for couplebreak events via the provided couplebreak event Id macros. Check the flag bitmask for present
couplebreak event types in a stream with the flag position macros. Query for flag position, event Id, event name or
origin event via the provided functions.

To detect a couplebreak event, look for entries with event Ids same as any of the "COUPLEBREAK_\[EVENT ABBREVIATION\]T_ID"
macros, where \[EVENT ABBREVIATION\] is something like SW, which symbolises `sched/sched_waking`. The T symbolises target.

By including the couplebreak header file, your code agrees to respect the possibility of being used when couplebreak is
active.

### Data streams

Streams' new state variables `couplebreak_on`, `n_couplebreak_evts` and `couplebreak_evts_flags` are now present. Use
`n_couplebreak_evts` to check how many couplebreak events there are in a stream. The flags bitmask sets each bit to 1
if a couplebreak event for a type of an event was created. Positions are 0-indexed (0th bit is least significant bit) and 
each bit flags an event as present as follows:
- 0 -> `couplebreak/sched_switch[target]` (event ID: `COUPLEBREAK_SST_ID`)
- 1 -> `couplebreak/sched_waking[target]` (event ID: `COUPLEBREAK_SWT_ID`)
The bitmask also serves as list of supported events.

It is recommended to not touch these, same as not touching `n_events` in streams, as this modification depends on them 
heavily.

Every function of a stream interface can be used in the same manner as for any other events, but couplebreak events will
most likely either redirect the request to their origin Id or construct values on the fly, they will not read the trace
data source.

### Couplebreak event entries

Couplebreak events are dynamically added and have no way of checking their "original data", except redirecting requests
for those to their origin entry or reconstructing that data as if being made anew, who must have been found in the data 
source for the current trace graph. Consequences of this are as follows:
- Couplebreak events **MUST NEVER** have their `event_id` and `offset` fields changed.
  - Offset has been changed to the origin entry's address - everything will break if this is changed.
  - Event Id is created on data load and cannot be found later somewhere else - asking for the original will just
    redirect to `entry->event_id`. Changing a couplebreak entry's event Id is the same as breaking it.

The entries can be used in plugins (e.g. `sched_event`'s couplebreak integration) and they shine especially well when
something needs to have an event in one task plot, but it cannot be done without editing an entry, possibly creating
an incompatiblity with other plugins. Using couplebreak events can resolve this. 

# Bugs

There have been two occurences of possibly parallel execution of KernelShark causing a crash when correcting CPUs of
`sched/sched_waking` events. These were only encountered during session support development. As such, they may have already
been resolved. However, because of their unexpected appearences, they must be at least mentioned. If one encounters such
an issue, it is recommended to simply restart the program and try again.

# Trivia
- Original name was "Couplebreaker", which sounds a lot like "troublemaker", making the author always think of Olly Murs'
  song of the same name.
- Session support was not requested, but to complete the feature, the modification included it anyway.
- Header file was a last minute inclusion, when it was deliberated upon that couplebreak hardly gives any interaction,
  even though some would be greatly appreciated.