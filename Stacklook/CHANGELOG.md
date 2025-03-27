# 1.4.4 - 2025-03-01

Fixed bugs related to switching trace files, namely:
- Program produced a segmentation fault sometimes, when 
  opened detailed stacktrace views for a particular file
  were and Qt was attempting to notify these deleted widgets
- Triangle buttons of Stacklook would be unable to use
  task colors if already in drawing distance during file
  switch
 
# 1.4.3 - 2024-08-02

Added copyright comments.
Finished the project.

# 1.4.2 - 2024-08-30

Changed responding from sched_wakeup to sched_waking.
Added processing of tep info for sched_wakings.

# 1.4.1 - 2024-08-25

Fixed and updated Doxygen documentation.

New features:
- nap rectangles added - visualization of prev_state of a switch up until the next wakeup.
  * can be turned off in the configuration

# 1.3.3 - 2024-08-22

Tried getting some more prominent prev state visibility through rectangles, doesn't work very well, so it's compile-option opt-in and labeled as WIP. Updated CMakeLists and source appropriately.

# 1.3.1 - 2024-08-17

Build instructions were modifed - if the plugin is to be used by unmodified
KernelShark, the user can specify that via a CMake variable `_UNMODIFIED_KSHARK`.

# 1.3.0 - 2024-08-16

Config is here! (At least the first working version.)

New features:
- Tools tab of KernelShark now have a button labeled "Stacklook Configuration"
- Configuration window allows one to change limit of entries displayed
  in the histogram before the plugin kicks in (don't set it too high though -
  always check what your computer can handle)
- Configuration window allows one to change the color of Stacklook buttons'
  outlines, which are by default black
- Configuration window allows one to change the default color of Stacklook
  buttons' inner area (refer to when the default color kicks in)
  * There is a possibility to make default color or task-based color configurable as well
- Configuration window also allows one to change what events to display from
  the set of allowed ones (which are hard-coded) & change the offset from the top of the stack used in the preview bar (i.e. if it is set to 3, it will skip first three items from the top of the stack)

Ideas:
- Change indication of S, R, D, I, ... states in for buttons above sched_switch
  (possibly just draw another shape next to those when hovering?)

# 1.2.7 - 2024-08-11

Newly, some constants have been moved to a future config window.

New features:
- Switch events now have a little letter in parentheses under the
  STACK text of the button, this letter indicates the prev_state of
  the task recorded in the switch event.

Bugfixes:
- No longer skipping an entry in the stack trace during preview

# 1.2.6 - 2024-08-06

Fixed:
- Unchecked nullptr in _mouseHover of SlTriangleButton

# 1.2.5 - 2024-08-04

Added final technical documentation page (or at least final until
revisions come).

Added a symlink in the build process and Stacklook's version as a
suffix to the actual SO file.

# 1.2.4 - 2024-08-03

Code documentation fixes & additions.

Build instructions were remade.

# 1.2.3 - 2024-07-30
Code documentation finished.

# 1.2.2 - 2024-07-29

More documented code, more cleanup of development code.

Code documentation is almost finished.

# 1.2.1 - 2024-07-28

Just cleaned some dependencies in code and leftover development
code.

# 1.2.0 - 2024-07-26

New features:
- Can hover over buttons to display the name of the task and
  first three items in the kernel stack for the entry's
  stacktrace under the button.
  * This has been made possible thanks to KernelShark
    modification, which adds the possibility of reacting to
    hovering over a PlotObject

Fixed:
- No longer seeing buttons even though a task or CPU plot
  is hidden

# 1.1.0 - 2024-07-24

Plugin is decomposed into more files, functions
are cleaned up, unnecessities are removed. Work
on task plotting & button hover event starts.

New features:
- Can show buttons over task plots, with a BIG but*
    * `sched/sched_switch` events are not a part of the
      task plot of the task that is switching
    * Only tasks switching into the task with the task
      plot have their `sched_switch` events shown and
      available
- Plugin now responds to `sched/sched_wakeup` events as well
    * These are included in task plots

Contemplating next features:
- Configuration menu (non-persistent)
    * To filter what events (if any) the plugin should
      respond to.
- Extend what events the plugin responds to
    * Should only be scheduler ones though
    * Theoretically it could be any (would require refactoring)
        * Just shift the buttons above `ftrace/kernel_stack` &
          filter based on information in the info string

Bugs:
- Turning off tasks from the graph will not hide the buttons
- Similarly with CPUs in the graph

# 1.0.0 - 2024-07-23:
## The first working version
Plugin finally works as it should in its most basic
form.

New features:
- Display buttons over sched_switch events
- Double clicking on buttons spawns a new window
    * Window includes stacktraces of kernel's stack
        * The top is marked with the label *(top)*
    * Can choose between list view and raw view
        * Raw works best when copying
        * List works best when simply reading
          the stack's contents
    * Windows are collected by the plugin and freed
      during plugin deinitialization
    * Windows say which task's kernel stack is shown
    * Windows can be closed either via the "X" button
      or the "Close" button
    * There can be multiple windows at the same time
    * All windows will close when KernelShark is closed

Currently missing:
- Task plot buttons
- Preview in info bar
- Documentation