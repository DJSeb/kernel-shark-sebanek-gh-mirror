Purpose of this is simply to jot down progress (self-motivation)
and keep track of noticed issues and possibly great ideas (tracker),
as well as quetions arisen during development (design decisions & encountered challenges).

Can be informally written, but better keep it neutral most of the time for any potential
viewers in the future.

Parts may be published later on in documentation.

# Tracker

(Bugs moved to [buglog](./Buglog.md)).

## Performance concerns

If any performance is deemed suspiciously low, note it here.
If a concern is either solved or dismissed, write an explanation.

- Explore optimizations to mouse hover detection
  - Note: Worth to meddle with KernelShark's insides,
    since that will be inevitable now anyway

## QnInA - Questions & Ideas & Answers

Questions are the main points, ideas are prefixed via `I:`, answers via `A:`.
Each idea may contain debate (pros and cons) and each answer should contain a reason.
This is noted mostly as a journal to not attempt some approaches again and as design decisions
documentation.

- How to achieve better cohesion and less coupling between `Stacklook` and `sched_events`?

  - I: Implement a "record changes history" to easily find out original values before modification
    - PRO - easily finds any historical change
    - PRO - a pretty lightweight and future-proof solution
    - CON - changes all of KernelShark's plugins behaviour, i.e. breaking change
    - CON - requires a lot of rewriting
    - CON - would need a history data structure implementation
    - CON - rare use case, since plugins shouldn't need to peek like that usually
      - If a plugin needs some other plugin, they make a list of depenencies by design.
        Plugins looking for original data should be able to read tep data themselves.
        Linked list would only prove useful if it was impossible to read original data
        for an intended functionality.
    - Verdict: An option for sure.
  - I: Find out if it is possible to load original data and use them even after KernelShark
    let other plugins do their work.
    - CON - might need to store copies of a whole file, i.e. memory-unfriendly
    - CON - semester project showed that this is most probably tedious work and either not directly
      supported by KShark or not at all
    - PRO - less rewrites of existing code
    - PRO - separated only into plugin's code, i.e. no SRP violation
  - I: Implement the splitting of events and work from there - sched_events and stacklook
    then shouldn't interfere with each other.
    - PRO - this will have to happen anyway, might as well leverage the feature
    - CON - will require at least some rewrites to make both plugins behave nicely

- How to enable NUMA visualization support?

  - A: Simply put, KernelShark's source code will be dissected and the visualization abilities
    written by hand.
    - Reason: KernelShark does not directly support reordering of CPUs in the graph, hence that
      ability will need to be implemented. KernelShark also doesn't support different visualization
      options, so that will also be written.

- How to split events from trace-cmd to target and initiator where applicable?
  - I: Try to intercept the incoming stream of files and add target's (initiators are the ones that are
    collected by default) 'fake' event.
    - PRO: Most straightforward, most likely will be the answer.
    - Will have to parse through trace-cmd's raw output or change parsed data when a splittable event
      is detected or do a second pass after data are in KernelShark's memory.
    - Verdict: Plausible solution, most likely the answer.
  - I: Don't add any events, but create a special filter through which normal events go through without a
    hitch, but splittable events are drawn as if they were in the data.
    - PRO: Less memory used by KernelShark holding real events.
    - PRO: Consistency with trace-cmd output.
    - CON: Cannot manipulate the drawn fake events, since they wouldn't hold data.
      - Unless they were pointing to the initiator event and would change relevant fields -> seems to be a lot
        of work for little benefit.
    - Verdict: Dismissed, while a nice idea on paper, it is most likely too much work for the memory benefit
      (abstract cost/value ratio isn't favourable).

# History

## 2025-02-21

Set up Windows Subsystem for Linux to allow smoother workflow when Linux computer isn't available.

Started rewrite of goals/requirements in the repository.
Migrated KernelShark from a submodule into main repository.

## 2025-02-22

More configuration work done on WSL.

Tip for the future: if polkit is giving you trouble, just disable it via adding a specific override group :)

More work done on README.md and Devlog.md

## 2025-02-24

Hopefully last configuration done on WSL.

More work done on Devlog.md.
Code changes should start after all is settled in here.

## 2025-02-25

No real work done, just some more configuration on WSL and Git.

Really only squash-merged the Devlog, README and documentation changes into main from development.

## 2025-02-25

Debugged the segmentation fault bug, gdb backtrace says it wasn't Stacklook, but KernelShark itself.
Problem might actually be the progress bar... will check source code.

GDB backtrace (above and below are loads of Qt functions):

```shell
#18 0x00007ffff7f02f07 in KsWidgetsLib::KsProgressBar::setValue (this=0x7fffffffa5d0, i=0)
    at /home/djseb-wsl/Documents/School/RP_BP/Git/KS_fork/src/KsWidgetsLib.cpp:67
#19 0x00007ffff7f3d11d in KsMainWindow::_load (this=0x7fffffffbb90, fileName=..., append=false)
    at /home/djseb-wsl/Documents/School/RP_BP/Git/KS_fork/src/KsMainWindow.cpp:1320
help
#20 0x00007ffff7f3d44f in KsMainWindow::loadDataFile (this=0x7fffffffbb90, fileName=...)
    at /home/djseb-wsl/Documents/School/RP_BP/Git/KS_fork/src/KsMainWindow.cpp:1348
#21 0x00007ffff7f387ee in KsMainWindow::_open (this=0x7fffffffbb90)
    at /home/djseb-wsl/Documents/School/RP_BP/Git/KS_fork/src/KsMainWindow.cpp:597
```

Call actually ends at std::\_\_atomic_bool<QtThreadData\*>::load.

## 2025-02-28

Well, seems the fault lies with Stacklook after all.
Calls end at something Qt-related. The segfault doesn't happen when
Stacklook isn't loaded. It most likely happens, when the Qt window
for stacktrace details is not destructed well, though that is a guess.

Probably a good idea to put a breakpoint into the destructor of the window,
or somewhere close.

...

And indeed it is a good idea, seems _'clean_opened_views'_ isn't as good
at its job as one might think. Still unsure where exactly the problem is though,
as debug step was too large, so it's time to try more.

New bug found though - switching trace files also makes it so
that detailed stack view doesn't get data about task state properly,
keystring here: _"No specific info for event."_ Assumedly, the problem
will be related to pointers (keys ARE taken from _ctx->..._, which might
be the core issue along with the fact the inner map in the function
is static).

...

So, after changing the function's const variable from static to normal,
the data are normally loaded... but we are still segfaulting. Not quite
sure why, but it is definitely connected to the detailed views.

## 2025-03-01

The more debugging happens, the more it seems like the segfault is either
compilation optimization related or timing related, none of which are
easy to debug. Stepping through the function seems to really just not
trigger the bug, continuing sometimes does, sometimes does not catch it.

Also, new bug, the triangle buttons should be hidden in reverse order
(currently the one on the left is the top-most in drawing order, it should
be reversed).

On the bright side, it seems that the triangle button's color bug had an
easy fix with an on-file-load function being called.

...

Seems the cause of segfault bug was a double free, once in clean_opened_views,
once somewhere else (unsure where though).

...

Easiest solutions are the best - instead of managing pointers and unsteady
lifetimes, just make it so the detailed
view is deleted either by closing it via the X button or
closing via the Close button or by main window closing (i.e.
process execution ending).
This also makes it so that windows persist after trace file is changed - it
might provide more utility that way anyway, if
not, they are one click away from being destroyed.
Just to be sure though, destructor of the detailed views
was implemented as empty, to hopefully lessen compiler's
imaginative optimizations, if there were any due to it not
being present (very speculative problem though).

## 2025-03-06

After noticing an unnecessarily void function writing to a
C-style array, I managed to change it to a function returning a C++ std::array,
improving that function. The
function was also long and was decomposed into smaller functions.

Also, a myriad of problematic solutions came forth to solve the reverse-drawing
of stacklook buttons, yet none are elegant. This is reminiscent of when SlButton
wasn't yet a class and just an ordered drawing procedure, when it was discovered that
the background, foreground and text of the to-be button had to be drawn as
text, foreground, background in this order, so reverse-drawing seems to be
baked into KernelShark's code, yet the reason why eludes this author's mind. There
are no `.reverse()` methods done on the `_shapes` object in `argVCpp`, meaning that
there might be some discrepency between OpenGL's rendering and Qt detecting events on
the shapes.

But here, working software is better than no software, so the bug shall be put on hold
for now, stay solved in separate branches until the best solution is found (such
solution hopefully won't require massive KernelShark rewrites).

## 2025-03-08

(Keywords should be used for a super-compressed summary of the text.)

**Keywords:** bottom list, Manage Plotting plugins, separate naps, sched_events interoperability

Today is not really very code-productive, it was mainly about thinking how
to allow coexistence of sched_events and Stacklook plugins, per specification.

To achieve this, main problems between the two are:

1. Stacklook needs unchanged data to properly make use of its nap rectangles
   - This feature might actually be extracted and put into sched_events, as it doesn't really
     have much to do with looking into the stack of a process. It might even become its
     own separate plugin (which would make sched_events still an antagonistic force).
2. Sched_events changes owners of sched\_\* events, especially sched_switch. Stacklook won't really be
   affected, it will still work (excluding naps). Main issue is, it could make it harder for the user to
   see stacktraces of events that actually occured on a specific task, though at this point, it is
   the user who is responsible for having sched_events activated, i.e. the behaviour is as expected.
   - It is still quite unfortunate, but if it leads to adhering to SRP and intended design, it is most
     logical to just keep it that way. Many plugin-ready systems require the user to manage their
     own plugins with load orders (e.g. modding videogames uses this approach immensely, one needs to
     only look at the (in)famous The Elder Scrolls series and its modding tools)

With these in mind, one course of action to be taken is extraction of nap rectangles and their integration
into sched_events. This should allow the competitors to work together instead of against each other, as both
will have access to the source code of the other.
The other course of action is extraction of nap rectangles into their own plugin (if only to follow SRP
and allow Stacklook to be "pure" when it comes to functionality). This would not solve the competition problem
of sched_events and nap rectangles though - which means this course of action would also see addition to
Kshark's API, a function which would return events without any plugin infestation or events with only
selected plugin's infestation. Neither of these are easy to get to, but there is a clue it is possible.
Kshark always reloads when plotting plugins are turned off or on in the Manage Plotting plugins menu in GUI.
So, there has to a version of the pure events somewhere. Another clue is the bottom part of the GUI, the events
list - that one never changes its contents, as plotting plugins don't affect it, meaning it also houses the
pure forms of events, its accessibility seems rather barred though.

## 2025-03-09

**Keywords**: sched_events vs stacklook, changing owners, project requirements

The problems seem to be much deeper than last day's attempts at solving this
riddle make it seem. Let's see what the detailed problem is:

Further morning hypothesizing revealed painful truths: the plugins cannot be made
compatible because sched_events's "changing owners" behaviour fundementally alters
semantics of task plots - events which would be there usually cannot be found, which
means their properties in the plot (namely, position and data reference) can NOT be
utilizied.
For Stacklook, this means two things: it cannot show buttons over events
that "do not belong" in the task plot, because sched_events changed their owners.
If the user wanted to see sched_switch's kernel stacktrace of e.g. PID 1587,
it cannot, as each sched_switch switched owners to the PID they are switching to
(e.g. PID 3575). Implication chain:
Sched_events is on -> sched_switch events change owners -> sched_switch events aren't
on the task plots of their actual owners (task plots can only use events which CURRENTLY
belong to them (plugins can manipulate that)) -> sched_events can show its boxes between
scheduling events -> no other event, which expects original information, can work ->
Stacklook cannot show buttons for events not in the task plot - they aren't there to begin
with.

The other issue for Stacklook are the naps. Naps, too, expect original data (fair
assumption) - only then can they include sched_waking events into task_plots and link
them with sched_switch from that task, showing their nap in between work. They of
course suffer from sched_events' meddling with sched_events' owners, especially
sched_switch events. There are nap rectangles shown, but on task plots and
between events that shouldn't be using them - the originators can be two different
events and the data that are pulled are incorrect.

Seems there is no hope left, right? Originators and targets get switched around to
fulfill plugin-specific actions, which clash heavily. So, what IS the solution?

One of the other requirements is to show pairs when an event is regarding two processes.
This might very well be the solution we were looking for - these event pairs, if
explicitly shown and included from the start in the graph by KernelShark, could
have sched_events link sched\_\*\[target\] events in the task plot they are in
and allow sched_switch\[origin\] and sched_waking\[target\] to link up for
Stacklook.

_Author's cries: "WOW, let's hope this is the hope at the end of the tunnel or I am
definitely getting something to punch! Who knew programs could make you this angry!"_

...

So, tomorrow should be the day these theories start being put to practice.
Either this will be a change to KernelShark or a plugin upon which others can
depend (in any case, sched_events will have to be altered, but in a hopefully
minor way - other default plugins might not need to face such problems).

## 2025-03-10

Well, plugin way seems to not be it, as there is no way to inject events into
an opened stream from a plugin's standpoint.

## 2025-03-11 - 2025-03-12

Not a lot of actual work, but work was spent on analyzing and trying to copy
what `missed_events` custom entries do. Will test and build later though.

## 2025-03-13

Finally! Custom events are being successfully created after a sched_switch happens!
They do break Stacklook, since it cannot find the stacktrace event for them yet (easy fix though).
Similarly, sched_events change the split event's origin pid - but once they are modified too,
the problem should be fixed.

With this, we'll hit two birds with one stone - sched_events and stacklook compatibility
and also the splitting of events.
