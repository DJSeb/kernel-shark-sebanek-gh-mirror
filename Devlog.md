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

1. Stacklook performed too badly when loading a trace without kernel stack.
  - **Solution**: check if the trace includes kernel stack trace events +
    perform a on-load search of kernel stack entries, so that further searches are unnecessary.

2. Explore optimizations to mouse hover detection
  - Note: Worth to meddle with KernelShark's insides, since that will be inevitable now anyway
  - **Solution**: Ignored, KernelShark behaves perfectly well with many entries and mouse hover over
    plot objects implemented as is.

## QnInA - Questions & Ideas & Answers

Questions are the main points, ideas are prefixed via `I:`, answers via `A:`.
Each idea may contain debate (pros and cons) and each answer should contain a reason.
This is noted mostly as a journal to not attempt some approaches again and as design decisions
documentation.

- How to achieve better cohesion and less coupling between `Stacklook` and `sched_events`?
  - **A**: Implement the splitting of events and work from there - sched_events and stacklook
    then shouldn't interfere with each other.
    - PRO - this will have to happen anyway, might as well leverage the feature
    - CON - will require at least some rewrites to make both plugins behave nicely
  - \[REJECTED\] I: Implement a "record changes history" to easily find out original values before modification
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
  - \[REJECTED\] I: Find out if it is possible to load original data and use them even after KernelShark
    let other plugins do their work.
    - CON - might need to store copies of a whole file, i.e. memory-unfriendly
    - CON - semester project showed that this is most probably tedious work and either not directly
      supported by KShark or not at all
    - PRO - less rewrites of existing code
    - PRO - separated only into plugin's code, i.e. no SRP violation

- How to enable NUMA visualization support?
  - **A**: Simply put, KernelShark's source code will be dissected and the visualization abilities
    written by hand.
    - Reason: KernelShark does not directly support reordering of CPUs in the graph, hence that
      ability will need to be implemented. KernelShark also doesn't support different visualization
      options, so that will also be added.

- How to split events from trace-cmd to target and initiator where applicable?
  - **A**: Try to intercept the incoming stream of files and add target's (initiators are the ones that are
    collected by default) 'fake' event.
    - Reason: Most straightforward, perfect place for implementation, simple integration with existing modules.
  - \[REJECTED\] I: Don't add any events, but create a special filter through which normal events go through without a
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

## 2025-03-14

Sadly, not enough work has been done, but there is quite a bit of unfinished
Qt-related GUI additions to dynamically control whether couplebreaking is allowed
or not.

## 2025-03-15

More work on creating a Qt widget settings for couplebreaking.
Mostly works, but it still has some bugs.

Newly discovered would be:

- Fault in stacklook: segmentation fault upon data reloads
- Following error displays upon attempting to summon the settings menu
  ```
    QLayout: Attempting to add QLayout "" to KsWidgetsLib::KsCouplebreakerDialog "", which already has a layout
  ```
- Changes in data are applied only when the settings menu is opened again.
  It should upate immediately after apply, but it seems something is missing.
  Best solution would be to copy everything that \_pluginSelect does, but it seems
  that was unsuccessful.

...

And apparently, last bug was fixed by simulating \_pluginSelect (mainly determining
loaded plugins). Same solution also fixed the first bug, or at least it appears to.

QLayout bug was also quickly fixed, turns out including a parent for a layout
makes the layout the parent's layout, so it was just about deleting a list constructor line.

Pretty successful today :)

...

Even managed to make stacklook (except nap rectangles) compatible with
the couplebreaker feature.

_Author's note: "I might get sick from such a long name, but hey, it_
_perfectly encapsulates what the added feature does."_

Next goals:

- add sched_waking to couplebreaker
  - subgoal: make couplebreaking generic for multiple possible
    "couple"-events (user-supplied event names in settings,
    generic splitting in KShark code)
- make nap rectangles compatible with couplebreaker
- split nap rectangles from stacklook into their own plugin
- make sched_events compatible with couplebreaker
- change KernelShark's CMake (e.g. add suffix '-modified' to version)

After finishing this, move onto the NUMA-aware feature.

## 2025-03-16

Today, sched_waking events have been made breakable,
couplebreaking in code was made a bit more generic when adding events.
Not generic in dealing with these found events, but I don't think that's really
an issue.

Found a problem though - applying filters makes these events disappear and
they are also not in the filter list, not very good.

This also, very frighteningly, implies that there is more stuff to do
if couplebreak events are to be found, filtered or otherwise worked on.

Scary indeed.

Next goals:

- Modify kshark functions so that 'couplebreak/\*' events can be
  found like any other event (at least by name) & filtered
- Subgoal: more dynamic couplebreak events choice
- nap rectangles indepenence
- nap rectangles work with couplebreaking
- sched_switch compatibility with cbreak
- change KShark's CMake

## 2025-03-18

Nothing really happened (work-induced involuntary vacation for a few days),
only an addition to KShark's CMake to add version descriptor suffix, marking
this version as very much different.

... Ok, after a little bit of frustration, there was an update to
the interface functions of libkshark-tepdata.c, so that some of the
searches could be done. It's hard-coded, not perfect, but it should get
the job done with a reasonable amount of effort.

They will all have a ripple effect, surely, there is also the matter
of ease of use (currently the user MUST expect that there could be
couplebreak enabled and MUST handle it themselves, not that great of a UX).

Still, let's update next goals:

- Optimize (usage-wise) couplebreak search modifications
- nap rectangles independence
- nap rectangles interop with couplebreak
- sched_switch interop with couplebreak
- subgoal: more dynamic couplebreak events choice
  - would necessitate a more dynamic approach to the search modifications
    too

## 2025-03-19

PHEW! Filtering events now interops with couplebreaker! A little bit of a
cleanup happened to the interface functions in lib...-tepadata, seeing as
returning more than is expected is just a bad look, especially for collections.
However, though hardcoded now, with the Open-Close Principle's (because it's good
to flex what we've studied) mindset, there was more of an interface extension,
slight modifications that won't hurt during normal operations and C++-side changes
to how the UI handles things - all to say that couplebreaker events can now
be filtered in and out of the graph (and list).

Next goals:

- De-hardcode couplebreak filter & stream interface modifications, if deemed too
  restrictive
- nap rectangles independence
- nap rectangles interop with couplebreak
- sched_switch interop with couplebreak
- subgoal: more dynamic couplebreak events choice
  - would necessitate the hardcoded modifications to be rewritten

Overall, productive enough for the 2 hours spent on this. It is time to move
onto other goals now though, as de-harcodisation can wait.

...

Add an hour to that, which parametrised some hardcoded stuff that happened today,
namely the state of couplebreaker and how many and which events have been made.

...

Add another hour, in which event IDs and name strings were once again de-hardcoded.
It is a little sad that there is more string search now (as we need dynamic searching
of couplebreak-able events), but it shouldn't be that bad for a GUI application.

## 2025-03-20

Naming has been unified. Also, a helpful article on what sched events are has been
[found](https://perfetto.dev/docs/data-sources/cpu-scheduling).

There's a bit of a dilemma with no right or wrong answer happening in couplebreaker:
Either I can keep the target's cpu as the same one from which the origin event
was fired, or I could maybe somehow get the CPU.

...

It was solved by iterating through every sorted entry after getting
them from the file and searching for a nearest sched_switch of the same
task, which determined on which CPU the task would run right after.

Easy to disable in code though.

Next goals:

- nap rectangles independence
- nap rectangles interop with couplebreak
- sched_switch interop with couplebreak
- subgoal: more dynamic couplebreak events choice
  - would necessitate the hardcoded modifications to be rewritten

...

Actually, since the correction would have to happen at two separate places,
it was moved into the get_records function as a last iteration through
a created, then destroyed sorted entries structure.

It is a bit of a messy way, but it is the most straightforward to
understand in code and it happens only once per load, so it's not terribly
inefficent.

## 2025-03-21

A few improvements to existing functionalities were finalised in couplebreak's
branch and the branch was merged into development, as other improvements will only
be bug fixes, maybe slight readability improvements.

Work has started on nap rectangles' own plugin, which should hopefully be just a
lot of copying, renaming and CMake writing. At least we will be more SRP-friendly.

There have also been slight corrections done in Stacklook, as some functions were
needlessly complicated, others didn't need to exist at all. Nothing major though, as
that wasn't the branch's goal - even so, these changes should have been done on
development branch.

...

A quick mistake was fixed - if there are no ftrace/kernel_stack events, there is no
point in searching for them, so any such searches will return a nullptr.

## 2025-03-22

Successfully copied nap rectangles code into its own plugin, with new build instructions
its own directory and only missing its documentation. Pretty good!

A little update to couplebreaking, now the shift numerical macro is accessible
from the header file, making getting a couplebreak ID simpler (one thing couplebreak
is plagued by is that it cannot give you an ID of the event until the stream is
loaded, since the events are created "on-demand" during load and their IDs are
just negative origin entry's event IDs and shifted by said macro).

Also added an access function to KernelShark, which helps with getting task colors, so
no need for hacks or needlessly duplicated color tables in code now. This change
and the removal of nap rectangles was also shown in Stacklook's code, along with
deleting a few unnecessary functions or making them static.

Remaining goals until NUMA:

- sched_switch interop with couplebreak
- naps plugin documentation (that's only partway done, mostly copied comments
  from Stacklook's time with that code)

Subgoal for dynamic events will most likely be dropped, it isn't really needed
to successfully complete the project, but it can be a good extra improvement.

README.wip.md was updated to reflect current progress.

## 2025-03-23

Compatibility fix for Stacklook (in the wrong branch, oopsie) to allow
consistent, but also correctly pid-checked kstack entry detection.

...

As work would have it, nothing much was done today, but a quick examination of
sched_events plugin did at least marked points of interest when it comes to
couplebreak interoperability.

## 2025-03-24

Sched_switch with couplebreak almost works, but it seems to take issue with
switch-to-switch boxes. Though it apparently was just an issue of setting the
wrong pid in the C code.

It would seem all works now :D

## 2025-03-25

Started documentaton work on the naps plugin. Then Stacklook documentation
revision will follow. Afterwards, couplebreak documentation. Then, turn in
for revision and start work on NUMA topology.

_Author's note: Documentation is always so taxing on the mind, I feel my soul
leaving my body._

## 2025-03-26

While finishing naps' user manual was supposed to be the main course of work,
it instead devolved into session integration of both naps and stacklook, while
also making couplebreak persistent via a session. The latter went rather well,
the former posed more issues than expected. Not only that, a possibly
concurrency-related issue also popped up twice, but it was impossible to
replicate. No matter, if it is really serious it will pop up again. Otherwise,
it's a KernelShark bug not to be fixed in this project.

Integrating stacklook and naps into KernelShark's sessions was quite the agony.
Default plugins don't share these issues and it may be partly due to some hidden
problem. First problem was double freeing in both plugins if a session was
loaded from the GUI while the plugins themselves were also loaded. Exiting
KernelShark then popped up a double-free error each time, GDB revealing that
the plugin attempted to free an invalid stream with a negative ID. This was
easily fixed by checking if a stream ID is non-negative.

The other problem still kind of persists. While creating naps, an idea
blossomed into a feature, where KernelShark allows sharng of its inner color
table made for task colors. The feature is a simple const color table
reference getter. This works almost every time, as the widget is always
available with the colors. However, opening KernelShark without loading any
user plugins (default ones do not qualify) and then attempting to import a
session resulted in a segmentation fault, where the plugin couldn't access
the color table, even though the KsGLWidget was already giving the reference
and calling its `_makePluginShapes` function. The cause of this is unknown.
Leading theory was that the KsGLWidget object was not yet constructed, but
with the shape-making function being called and the program returning a
reference, just one which cannot be accessed, it seems the problem was
somewhere else, but it is unknown where. Naps (and in the future stacklook)
evaded issues by always starting in non-experimental mode, where its
rectangles' outline colors were set to the color of the prev_state, as is
the case with its filling color. Only when the user wishes to use experimental
feature can it begin using task colors. This approach is sure to work,
especially since plugin configurations do not persist. Stacklook will have it
more difficult, but the same approach might work there as well. Maybe the
cause was instead a reference to a dead object. Either way, without
bigger KernelShark rewrites, this issue goes into very low priority.

...

But at least images are now in the user manual.

## 2025-03-27

A much better PDF user manual for Naps was made.

Stacklook got its button coloring changed as well. It also helps
a little with readability, so that's nice.

But there is a performance issue apparently, seems that thousands of
entries will make KernelShark freeze with Stacklook on. Which
is a little funny.

...

Performance issue was fixed and Stacklook was improved to rely
less on runtime searches - kstack entries are now found once per
stream load, improving performance anyway.
