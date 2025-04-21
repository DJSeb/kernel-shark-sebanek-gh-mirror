Purpose of this file is to keep track of noticed issues and possibly great ideas (tracker),
as well as quetions arisen during development (design decisions & encountered challenges).

Can be written informally, but parts of it may be published.

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
