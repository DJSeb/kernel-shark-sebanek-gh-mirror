# Repository overview

## Repository owner

David Jaromír Šebánek (contact e-mail: `djsebofficial@gmail.com`)

## Supervisor

Jan Kára RNDr.

## About

This is the main repository for the semester project & bachelor thesis for Faculty of Mathematics and Physics of
Charles University. It is, in general, concerned with improving existing software **KernelShark** with features
explained below.

It is composed of four main directories, each representing a portion of the thesis project.

- **Stacklook** - plugin for visualising stack traces via a more GUI-centric way
- **Naps** - plugin for visualising timeslices between a task's switch and next waking
- **NoBoxes** - plugin which disables some plugin's ability to partake in drawing taskboxes
  - This plugin doesn't work perfectly, as it seems there's a lot of seemingly random pop-in upon GL Widget updates,
    as the bins are a little unruly when it comes to being consistent with their visibilites.
    But unless a pretty big guttting of KernelShark's visualisation is done, this is currently possibly the best approach.
  - This was a requested bug fix, but is presented as a plugin for easy on/off changes - flickering may be too much for some.
    It is not a necessity for the project, hence it's presented as "best effort" solution.
- **KS_fork** - modified copy of KernelShark's source code with multiple additions, what many parts across different
  documentations will call "custom KernelShark"
  - _Couplebreak_ functionality is a new ability of KernelShark to split some events (chosen in code) into two
  - _NUMA Topology Views_ functionality gives KernelShark the ability to show CPU plots with respect to the NUMA
    topology given by the user to the program
  - _Other smaller additions to KernelShark's abilities_
- **SurveyPaper** - directory containing the survey paper **in Czech** about the plugins & enhancements above, along
  with `ExamplePrograms` directory in which traces of simple programs showcasing above's abilities may be found.

These five components' requirements are defined in the [project specification document](./ProjectSpecification.md). Each plugin has
their own README and documentations, **KS_fork** includes a document detailing changes made there.

See each directory for more details about that part of the project.

## Projects' status

### Semester project (NPRG045) general tasks

Semester project part is finished.

(Stacklook plugin and small changes to KernelShark.)

- [x] Brief project description
- [x] Specification
- [x] Basic working version\*
- [x] Final version

\* The most important things work.

### Bachelor thesis project general tasks

Bachelor thesis project part WIP.

(Stacklook revision, Naps plugin, Couplebreak, NUMA Topology Views.)

- [x] Brief project descrption
- [x] Specification
- [x] Basic working version\*
  - [x] Split events involving two processes
  - [x] NUMA topology visualisation
  - [x] sched_events interoperability with created plugin(s)
- [ ] Final version

\* The most important things work.

### Roadmap

Things not in specification aren't mandatory, but would be pretty useful.

**Semester project part:**
_Functionalities:_

- [x] KernelShark Record adjustment for stacktracing
- [x] Stacklook
  - [x] Get scheduler event for when the task was about to go to sleep and the stacktracing event
  - [x] Add clickable shapes above each sleep event
    - \[ABANDONED\] _(Not in specification) Add such clickable shapes into entry viewer_
      - NOTE: It is highly unlikely that this is easily doable as a plugin, unless a lot of KernelShark is rewritten.
  - [x] Show a dialogue/window with detailed stack information
  - [x] Display preview of the stack in the top info bar
  - [x] Create CMake build instructions
    - NOTE: Could be circumvented if the plugin files were put into KernelShark's `src/plugins` directory
  - [x] Create plugin documentation (user & technical)
    - [x] Create build instructions for the documentation
    - [x] Technical documentation (Doxygen)
    - [x] User documentation (Markdown or HTML)
  - [x] _(Not in specification) Add a settings menu for the plugin_
  - [x] _(Not in specification, but requested)_ Visualization of prev_states after a sched/sched_switch
  - [x] _(Implied)_ Make it work for CPU and task plots

_Demonstration:_

- [x] Create example trace files for demonstration

**Below are bachelor thesis part's appended requirements:**
_Functionalities:_

- [x] Events involving two processes shall be split into initiators and targets
  - E.g. `sched/sched_waking` should be split into "awaker" and "awakened" events, one belonging to each respective
    process
  - [x] Specification demands at least `sched/sched_switch` and `sched/sched_waking` should be split.
  - \[ABANDONED\] Try to keep the logic inside the plugin (minimizing KernelShark changes)
    - NOTE: Not doable, insufficeint API, KernelShark source code changes necessary.
- [x] NUMA topology visualization in KernelShark (either as a plugin or KernelShark modification)
  - [x] Parse data from Istopo (XML format)
  - [x] Visualize said data on the screen
    - [x] If no Istopo data are given, use default KShark visualization
    - [x] CPU reordering according to topology (NUMA nodes, hyperthread siblings)
    - [x] Tree grouping of CPUs
      - [x] May be collapsible (preferred)
        - NOTE: It is collapsible, but as a whole, not a single tree or
          or single node - that would be unnecessarily complicated
      - NOTE: Other grouping visualizations are possible, if
        CPUs are sorted by topology, but tree-like is the one
        that will be implemented as basis
    - [x] Colorful differentiation of different groups
  - [x] Configurable display method via some option in KShark's menu
- [x] Allow `sched_events` plugin compatibility
  - NOTE: XOR choice
  - \[ABANDONED\] Either integrate Stacklook into sched_events plugin
    - Would violate the holy Single Responsibility principle
    - Direct approach
    - Ensured success (this statement was proven false)
  - [x] Or figure out how to allow compatibility while keeping plugins separate
    - Less direct
    - Keeps SRP
    - Unknown probability of success (very high)
    - Ultimately chosen due to KernelShark's insufficient API for creating new entries.

_Demonstration:_

- [ ] Create example trace files for demonstration
  - [ ] Write a program that will do something with the kernel stack to see it with Stacklook.
  - [ ] Create a trace file without kernel stack entries to show off inacting Stacklook.
  - [x] Create/get a trace file to show off NUMA topology views
    - \[NOTE\] This might be just a matter of implementation of said feature and previous trace files might be reused.

_General SW goal:_

- [x] Debug the plugins even more, stabilise performance where necessary
- [x] Debug created extensions for KernelShark
  - [x] Record kstack
  - [x] Get PID color
  - [x] Preview labels changeable
  - [x] Mouse hover plot objects
  - [x] Couplebreak
  - [x] NUMA Topology Views
- [x] Revise README and other non-code parts of repository to reflect project extensions
- [ ] _("Optional")_ Create proper design documents
  - Would be quite good to include in technical documentation

_Non-software goal - write survey paper:_

- [ ] Survey paper
  - [ ] Stack tracing (in Linux)
    - Mostly basics, some more info about KernelShark's stack tracer
    - Mainly to set context for the project
  - [ ] Basic visualizations of trace data
    - Not exclusive to stack tracing, but mostly about it
  - [ ] Theoretical part of KernelShark
    - [ ] Basic overview of design
    - [ ] How is it visualizing the data?
    - [ ] What are the limitations?
      - I.e. why make the project in the first place?
    - [ ] What is the job of plugins? / Removing limitations
  - [ ] Theoretical part of Stacklook, Naps & KShark modifications
    - [ ] Overview of piece of SW's design
      - Should be included in the documentation
    - [ ] Purpose & limits
    - [ ] Results of implementation
      - [ ] "Ultra-documentation" - user, design and maybe interesting parts
            of technical
      - [ ] "How was development? What interesting things happened?"
    - [ ] Extensions
