Purpose of this is simply to jot down progress (self-motivation)
and keep track of noticed issues and possibly great ideas (tracker),
as well as quetions arisen during development (design decisions & encountered challenges).

Can be informally written, but better keep it neutral most of the time for any potential
viewers in the future.

Parts may be published later on in documentation.

# Tracker
## Bugs
If any bug occurs, note it here and its environment + behaviour.
If a bug has been solved, mark it and provide explanation (or a commit ID where it was solved).

- Switching between trace files results in a segmentation fault
    * Status: OPEN
    * Cause: unknown
    * Environment: WSL-openSUSE-Tumbleweed

## Performance concerns
If any performance is deemed suspiciously low, note it here.
If a concern is either solved or dismissed, write an explanation.

- Explore optimizations to mouse hover detection
    * Note: Worth to meddle with KernelShark's insides,
      since that will be inevitable now anyway

## Questions & Ideas & Answers
Questions are the main points, ideas are prefixed via `I:`, answers via `A:`.
Each idea may contain debate (pros and cons) and each answer should contain a reason.
This is noted mostly as a journal to not attempt some other stuff again.
Also a journal for later trips down memory lane.

- How to achieve better cohesion and less coupling between `Stacklook` and `sched_events`?
    * I: Implement a linked list to easily find out original values before modification
        * PRO - easily finds any historical change
        * PRO - a pretty lightweight and future-proof solution
        * CON - changes all of KernelShark's plugins behaviour, i.e. breaking change
        * CON - requires a lot of rewriting
        * CON - would need a linked list implementation
        * CON - rare use case, since plugins shouldn't need to peek like that usually
            * If a plugin needs some other plugin, they make a list of depenencies by design.
              Plugins looking for original data should be able to read tep data themselves.
              Linked list would only prove useful if it was impossible to read original data
              for an intended functionality.
        * Verdict: ON-HOLD, due to amount of work
    * I: Find out if it is possible to load original data and use them even after KernelShark
      let other plugins do their work.
        * CON - might need to store copies of a whole file, i.e. memory-unfriendly
        * PRO - separated only into plugin's code
//TODO: The rest of this

# History
## 2025-02-21
