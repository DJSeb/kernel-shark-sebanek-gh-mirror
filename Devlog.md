Purpose of this is simply to jot down progress (self-motivation)
and keep track of noticed issues and possibly great ideas (tracker),
as well as quetions arisen during development (design decisions & encountered challenges).

Can be informally written, but better keep it neutral most of the time for any potential
viewers in the future.

Parts may be published later on in documentation.

# Tracker

These should be interchangeable with GitLab issues and their comments.

## Bugs

If any bug occurs, note it here and its environment + behaviour.
If a bug has been solved, mark it and provide explanation (or a commit ID where it was solved).

- Switching between trace files results in a segmentation fault
  - Status: OPEN
  - Cause: unknown
  - Environment: WSL-openSUSE-Tumbleweed

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
