Purpose of this is simply to jot down progress (self-motivation).

Can be informally written, but better keep it neutral most of the time for any potential
viewers in the future.

Parts may be published later on in documentation.

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

...

A lot of documentation work on Stacklook and Naps done.

## 2025-03-28

Documentation saga continues. User documentation is taking a very long time to look
as good as possible. Some CMake changes were tried out, but late errors showed they
were more error prone than not.

...

Stacklook user manual finished for markdown. Now to just carry this over into the PDF
version.

...

Now also the PDF version is done. The only things that need finishing are:

Next goals:

- document Get PID Colors
- document Record kstack
- document Mouse hover plot objects
- document Preview labels changeable
- document Couplebreak
- revise Naps' design document
- write Stacklook's design document

More future goal is to make NUMA Topology Views (implementaton & documentation).

Then "just" write the survey paper.

## 2025-03-29 - 2023-03-30

Writing documentations and improving parts of code that
were less visibly wrong before (seeing with fresh eyes).

A lot of boring work, but hopefully a good amount of sources
to use in the survey paper later.

...

There was a couplebreak change, now the supported event Ids are
in the header file libkshark-tepdata.h, which deletes a lot of
string lookups.

...

Even greater couplebreak change, there is some sort of interface
defined in a header file. CMake was also edited to fit all this.

Next goals (in priority order):

- Document Couplebrek -> then send to supervisor
- Create NUMA Topology Views modification
  - Implementation
  - Documentation
- Revise Naps' design document
- Write Stacklook's design document
- Write survey paper

## 2025-04-04

_Author note: Back from vacation._
NUMA work starts (-ed a little earlier with "thinking about it").
Currently the main goal is designing some sort of Qt window, which
will be the main interface for users.

The feature will most likely work per-stream, as streams can be any
set of traces for some CPUs and tasks, but multiple traces can
be opend via "Append traces". It definitely will require some sort
of stream-NUMA connection, either in streams directly (not the best
choice, doesn't feel like it's right) or via some sort of hash map
kept by the feature (which will require more work on managing which
stream closed and which stream is up, but hopefully nothing major).

## 2025-04-05

Deliberations are continuing. Quite a lot to think about with this
modification. Design is starting from the GUI. The window for the
modification will look like this:

```
-----------------------------------------------------
|                   HEADER                          |
-----------------------------------------------------
|                                                   |
|                   EXPLANATION                     |
|                                                   |
|---------------------------------------------------|
||  Stream #1             ____________             ||
||  STATUS                | LOAD BTN |             ||
||                        ------------             ||
||  0 Default   O Tree ...[more radio options]     ||
||-------------------------------------------------||
||  Stream #2             ------------             ||
||  STATUS                | LOAD BTN |             ||
||                        ------------             ||
||  O Default   0 Tree ...[more radio options]     ||
||-------------------------------------------------||
.....................................................
||-------------------------------------------------||
|       _____________           ______________      |
|       | APPLY BTN |           | CANCEL BTN |      |
|       -------------           --------------      |
-----------------------------------------------------
```

Similar in nature to couplebreak's dialog (maybe a
generalisation of that could help here, retooling it
into a "KsPerStreamFeatureDialog" or similar).

Apply would obviosly apply changes and make the graph (and
any of its drawn parts) to be redrawn, CPUs hidden or shown, etc.
Cancel would close the dialog and cancel any temporary changes.
Header is the standard header with an X button, which will do the
same as Cancel probably.

The radio buttons for each stream would determine which kind of
view is to be shown for each stream's CPU plot part. Tree would
be the one showing off the NUMA topology. Default is what KernelShark
uses currently. Load button will open a file dialog window, which
will allow only an XML file of a topology to be loaded.

There should probably be validation of the number of PUs and CPUs in
a stream. Anything else would be difficult to verify, so that won't
be done. Status field would probably show something as "LOADED \[FILE\]",
"NO TOPOLOGY", "BAD TOPOLOGY". There should be some space for more
radio buttons/options later.

The other design decisions are about how to connect the topology with
KernelShark's visualisation. There'll firstly need to be some sort of
a data structure representing a machine (a stream for KernelShark),
which means tracking the nodes, cores and PUs (CPUs for KShark).
Additionally, groups and packages should be tracked too, for completeness
(and possible future extensions, though they won't be used, most likely, in
this project). Cores will probably hold its PUs and memberships, but some
parts might be shaved if proven to be unnecessarily complex. This part isn't
fully fleshed out yet.

Next problem is, how do we keep this for a stream to be drawn and redrawn?
An obvious possibility is giving streams some sort of pointer to the C++
structures, but this feels incorrect though possible. Another possibility is
keeping a map of loaded topologies and give streams the means to ask about them.
This should work nicely, as unordered_map in C++'s STL would be a good fit and
memory could be efficiently kept. Only question is where to store the map -
for that, the best place is probably some sort of static data or a class member.
Another possiblity is a singleton, as this would technically hold configurations
and a "lot" of data, which singletons are good for. Either way, the map has to
be widely accessible to other KernelShark modules, so maybe a special class for
access could be made as well. This will be subject to further deliberation.

Another question is how to parse the XML file. There "could" be a custom XML
parsing code... but that is a lot of work for not as much profit. A better
solution would be some XML parsing library for C++. Rapid XML seems rather
widespread, but no exact library was chosen yet. It would also introduce
another dependency to KernelShark, which might be less favourable, but then
again, it is just an XML parsing library, which shouldn't be that heavy.

Penultimate decisions lie in control on the graph. The tree view would be best
collapsible, it is a really user-friendly feature for this kind of view. This
does mean introducing even more Qt objects into the graph area though and worse,
connecting actions to them.

Lastly, session support. It WOULD be nice to keep data in sessions. This is not
something outlined in the specification, but it seems like a natural thing to
include, as many per-stream settings are included. There would have to be the
whole topology and which layers are shown and which are hidden. It wouldn't be
impossible (nothing really ever is), but it would be a bit of work. Probably the
last thing to be done, if to be done.

Sidenote, naming can sometimes be nicely shortened. Henceforth, NUMA Topology Views
will be also referred to as NUMA TV, which is slightly funny and unintentionally so.

NUMA TV goals:

- Create configuration window (maybe reuse couplebreak's)
- Create a data structure for a machine in topology's eyes
- Parse XML data into the data structure
- Create a connection between streams and topologies
- Figure out control elements of the tree view, create more space in the graph
- Figure out session support

## 2025-04-07

A little bit of work done on the NUMA TV config window, nothing too difficult.
More importantly, the annoying warnings about deprecated stateChanged function
have been resolved by just updating code with adjustments to the new function
(which really just changes a state being represented by an int into an enum).
Rejoice, for no more annoying warnings about things that should never have been
present to begin with happen now.

XML topology file examinations are underway. Because the XML is large and
custom XML parsing seems like far too much work, external XML library will be
used. For being light, fast and time-proven, [pugixml](https://pugixml.org) was
ultimately chosen. This does mean KernelShark will gain a new dependency, but
it's better than toiling with XML parsing, which has been solved N times over,
instead of focusing on the actual project contents.

...

Scrath the previous paragraph altogether (but it is kept as something that
happened). [Hwloc](https://www.open-mpi.org/projects/hwloc/), which produces
the XML file, can also load the topology from it, so no XML parsing will be
done, just operations with the hwloc library.

It was added to CMake as a dependency, although how it works with it is not
made clear on the internet.

...

One thing is clear though, the data structure seems too "rich" with data, so
either some sort of slimming down via hwloc's API will be necessary or a custom
class will store what's necessary for displaying the topology (which currently
means nodes, their cores and the cores' physical units).

Packages and groups won't be supported just yet, as they will be more of a
completeness feature, not a necessity, since they are not outlined in the
specification. There's also the question of nested NUMA nodes in NUMA nodes
(which, sadly for the author, is possible), but let's focus on the easy
situation first.

## 2025-04-08 - 2025-04-09

NUMA TV GUI work, its reactivity to the user, hwloc topologies and all that was
worked on. Not much to say, as it was mostly trial and error regarding restricted
pointers, lifetimes and expected behaviours and nothing too interesting except
endless debugging took place.

## 2025-04-10

Supervisor sent review of couplebreak and co., only real bug is that the CPU
runtime rectangles shouldn't appear after sched_switch's stacktrace and after
sched_waking target event. Will investigate later, after NUMA most likely, to
search for a fix of these. Otherwise, it was apparently very good, which is
great to read, after so much work.

## 2025-04-11 - 2025-04-13

Lot of experimentation with how to draw the topology - most unsuccessful.
Anchoring some drawing to another inside the GL widget is simple enough, but
the biggest problem currently is creating space for the topology part. The idea
was either to make some Qt thing or creating it inside the GL canvas itself.

Both Qt and GL approaches have resulted in failure. Qt moreso, as it completely
cut off the graph on the left, making everything on that side useless. The GL
approach was a little better, but shifting every drawing proved to be not enough
as the drawing's positions are not shifted, leading into inconsistencies with
what's displayed and where it is in terms of coordinates. The right part of the
graph also keeps getting cut off, with no obvious fix.

However, the GL approach will most likely be chosen, as it contains the necessary
contexts for the tree view and makes visualisation less confusing, albeit a
massive chore, because a lot of positioning will now have to somehow get a
"topology correction offset".

...

Actually, this approach also seems to have some inherent problems, like there
just not existing any way to sanely reposition everything. It truly feels like
nothing works.

Really unfun to be stuck. It feels like being trapped in a never-resolving hell.

...

Honestly, no idea this will be solved. Praying for a miracle, currently.

## 2025-04-14 - 2025-04-15

Time management on this is horrible, first of all.

Second of all, Qt-based approach was chosen. It does have better separation of
concerns and is much more flexible. All that is missing currently is somehow
intertwining the graph with whatever will display the topology, at least in that
visualisation department.

The approach gave way to a button to dynamically hide or show the topology
view portion and a dynamic resizing of the contents of the topology widget.
It is also much simpler to use in code.

Goals:

- Reorder CPUs based on topology
- Pick a visualisation of the topology
  - Qt tree (basic, most likely won't work)
  - Own grid-based widget
  - Amalgamation of labels and lines and spacings
  - Note: collapsibility is an important factor here
- Session support maybe
- Clean up after yourself
- Document it all
- Maybe package tree view as well

## 2025-04-16

Today, it was first cleaner code in the KsTraceGraph constructor, some stylisation
if the toggle button for the topology view and then the first big job: reordering
CPUs based on topology. It took a while to find the best suitable candidate, but
we managed to do so - find number of numa nodes, index from zero to the count (which
counts as logical indices), find PU from the given CPU (which is a physical/OS index,
according to hwloc (sidenote, pronouncing it as \[have lock\] or as a name Havelock
makes it sound pretty cool), find the ancestor core of the PU (which should be just
one level higher), put them in a map (which is sorted and can keep more maps
as values) of maps of maps, with the structure
{numa:{core:{PU-logical:PU-physical}}}. Then just reiterate through this fully
sorted collection and have the CPUs display in a sorted manner. It currently
ignores what PUs were supposed to be hidden, but that will be ironed out, maybe
with a search through the vector upon each attempted adition of a PU to the sorted
collection (though that could take a while - on the other hand, CPU draw is a
blocking function called rarely, design helps us in ignoring this currently).

...

Actually, it was really easy to just go through the vector and check if it
contains a CPU number.

...

And now a file with a different amount of cpus cannot be a topology for a
stream with a set amount of CPUs. Lastly, applying a NUMA topology will also
redraw the CPUs. It will force to show all of them (which makes sense, since
we now want to see a topology, it only makes sense (to the author) that the
full scope of the topology be shown).

...

Topology configs are now destroyed upon opening a new trace file (which is
the only way to destroy data streams except exiting).

...

Absolutely final work today was cleaning up some code, introducing static
functions and a few member functions, etc. and deleting NUMA TV's presence
from the GL Widget, which has been declared as forbidden grounds, as its
behaviour is far too unpredictable.

Goals:

- Pick a visualisation of the topology
  - Qt tree (basic, most likely won't work)
  - Own grid-based widget
  - Amalgamation of labels and lines and spacings
  - Note: collapsibility is an important factor here
- Session support maybe
- Clean up after yourself
- Document it all
- Maybe package tree view as well

## 2025-04-17 (and a bit of 2025-04-18)

Visualisation of NUMA topology works! Rather splendidly as well.

Only problem now is a weird segmentation fault bug upon trace append, but
it otherwise works great! It is a blocky-tree, but it gets the job done well.

_Author's note: Writing this at 3AM, so not a lot to say in this state of mind._

- Fix bugs, polish
- Session support maybe
- Clean up after yourself
- Document it all
- Maybe package tree view as well

## 2025-04-18

So, in regards to that bug found "today" - it seems that histogram's data pointer
cannot be accessed for unknown reasons. Best theory from testing though is
that since the CPUs have been reordered, the histogram gets very confused
and somewhere along the line, the data pointer becomes crazy.

With more experimentation, the fault seems entirely situated in a single state
of the graph: one trace is opened && it has a topology view, which rearranges
its CPUs && that topology is used && task redraw has never been called.

It's super-situational and likely smell of something else than just rearrangement
of CPUs being a big bad change. Either way, since it actually nicely coincided
with needing to redraw tasks to adjust task padding, just calling task redraw
each time NUMA TV calls cpu redraw seems to have fixed it.

...

Fixed scaling bug of topologz views (who would have thought that the bottom
space is not margin, but one additional spacing), added some stream identification
to the topology constructor.

API of KsTraceGraph was also privatized more and explicitly named with "numatv"
prefixes.

...

Colors in the topology widget now (allegedly) correspond to colors of CPUs held
by the GL widget. Cores use an average of their PUs' color, analogously for
NUMA nodes. Each part of the topology block-tree now has a 1px solid black
border around itself, which makes it a little prettier + it highlights
connections between the children and parent (2px solid black line naturally
forms).

KsPlotTools also got new tools, getting color intensity (courtesy of Stacklook),
and black or white color based on intensity (again, thank you Stacklook).
It felt more natural to put these two tools there, as they popped up thrice
by now in the project and seem rather useful.

Overall, a good day.

Goals:

- Fix bugs, polish
- Session support maybe
- Clean up after yourself
- Document it all

Package tree view will be left as something for extensions.
Session support most likely will happen.
Everything else is necessary and required.

## 2025-04-19

Today was mostly spent on code decomposition and bug fixes.
KsStreamNUMATopology's monster-constructor was broken up into setup functions
and some helper static functions were also created, namely for creating
stylesheets for topology items.

Coloring looks quite random, which is a sad sight to see, but there that is
a consequence of using KernelShark's colors. Not much to do about that.
A possible change would be somehow choosing colors independently. But that
loses a connection to already existing context. Another change is using
lines instead of blocks, but that loses "quick" check of what is currently
visible (not as quick, unless one is good at remembering colors). Maybe
instead of repeating "PU P#X L#Y", it could say "N#A-C#B-P#X" or similar.
It would make the information clearer. It is not necessary to specify machine,
since those are represented by color in KernelShark already and aren't really
as important as a topology of a stream - which also means there isn't THAT
big of a reason to display a machine column, though it is done for completeness.

...

Well, names were shortened to M, NN, C, PU for machine, NUMA node, core and
processing unit respectively. This will be easy to understand and convey in
the documentation anyway. PU tree leaves will have a prefix of '(NN X, C Y) ',
cores will have a prefix of '(NN ) '. This is done to quicken lookup. Machines
are, again, differentiable easily by color and most likely don't need to be
included (might even be unnecessary as is).

To be fair, maybe even the PU column is a little redundant - it's already
connecting to the CPUs, so maybe all of this is unnecessary. the information is
already there.

...

PU column was commented out after some deliberation if it is actually useful.
Removing it brought more space and less color chaos to the topology widget.
The machine column will stay - while not super useful, it is better to show off
the topology as an actual tree and not a forest implictly connected.

...

Session support will actually be postponed until specifically request - current
implementation might be problematic due to the static data of KsTopoViewsContext, it
being a singleton. They would logically belong to streams, but those their
destruction code is hard to find.

It might be worth just "exporting" stream topology configuration data into
a stream and work from there. Issue for tomorrow I suppose.

Goals:

- Fix bugs, polish
- Clean up after yourself
- Document it all

## 2025-04-20

Nevermind, session support exists after it was determined that markers are saved,
so other C++ parts have to be saved, that also aren't a part of the stream.

What's more worrying is that loading a topology for stream #1, while having
none for stream #1, assigns the topologyt tree to stream #0 in the visual model.
That's REALLY bad. They are supposed to be at their stream's position, this
is a huge inconsistence.

...

OK, big rewrite, but it was fixed. We've come to ALWAYS make a topology
widget if the Apply button is clicked, which can be reasoned about, but
the main need was to allow "gaps" in the topology layout. It shouln't
slow down things much though, since this will happen literally only if
a CPU redraw is called (which is not often) or by NUMA TV dalog (still not
often). It's fine.

But this SHOULD mean all bugs found in this are fixed. There are sometimes
segmentation faults or json faults, but they are random and hard to pinpoint.
Probably nothing major though.

Now just the couplebreak and kernelstack bugs need fixing, along with
documentation of NUMA TV.

Goals:

- Fix bugs, polish
- Clean up after yourself
- Document it all

## 2025-04-21

NoBoxes plugin now exists. It only enables behaviour of not drawing some
taskboxes, if the bin's visMask has it unset. The behaviour intself is a little
janky, but it works most of the time, which is the important part. Too bad, but
this is what we have, plus it's not a part of the specification, so further
requirements can be left as an extension.

There have been some changes to the meta README, Tracker, this Devlog, names of
modifications - all to start converging on the final part of the bachelor thesis
software work, that being the documentation.

_Author's note: Creative juices are starting to run out, so this is a welcome step._

...

Doxygen documentation is finished! Thank you Copilot, you have done a good job
this time, a lot of hints were actually what I needed.

After NUMA TV documentation is finished and approved, survey paper work can
begin. Actually, that and all design documentations for NUMA TV, Stacklook,
Naps and maybe an improvement to Couplebreak. At the very least some diagrams
will be needed.

Goals:

- Document it all
  - Design docs
  - Pretty pictures
  - NUMA TV modification documentation
- Survey paper

## 2025-04-22

Today was pretty fast. NUMA TV context type was de-singletoned,
NUMA TV modification documentation was written and some sort
of Stacklook design document was also written.

Some names in the code were made more explicit, some comments
removed or adjusted. A fix for Naps plugin's dependency on
traceevents was made, though a pretty poor one, just
copying KernelShark's FindTraceEvent.cmake file. Weirdly, it
seems that Qt is the odd one out when it comes to being easily
found by CMake as a package.

Current work was sent to the project's supervisor and is
awaiting evaluation. It is time for another recuperation.

Goals:

- Fix whatever the supervisor deems to be in need of a change.
- Survey paper
