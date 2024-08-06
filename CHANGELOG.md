# Timeless info
Currently missing:
- Task plot buttons for sched_switch of the plotted task
  (may be impossible without deeper KernelShark rewrite)
- User documentation

# 2024-08-06
# 1.2.6

Fixed:
- Unchecked nullptr in _mouseHover of SlTriangleButton

# 2024-08-04
# 1.2.5

Added final technical documentation page (or at least final until
revisions come).

Added a symlink in the build process and Stacklook's version as a
suffix to the actual SO file.

# 2024-08-03
# 1.2.4

Code documentation fixes & additions.

Build instructions were remade.

# 2024-07-30
## 1.2.3

Code documentation finished.

# 2024-07-29
## 1.2.2

More documented code, more cleanup of development code.

Code documentation is almost finished.

# 2024-07-28
## 1.2.1

Just cleaned some dependencies in code and leftover development
code.

# 2024-07-26
## 1.2.0

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

# 2024-07-24
## 1.1.0 Documenting and cleaning up
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

# 2024-07-23:
## 1.0.0 - The first working version
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