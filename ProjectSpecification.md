# Stacklook - a KernelShark plugin for visualizing stack traces

## Intro to KernelShark

KernelShark is a tool for visualizing data taken from `trace-cmd` - such data
often helps when analyzing performance issues, testing and in
general checking how the system and application ran on the CPU(s).
KernelShark also offers a support of plugins for user's optional extensions of the tool.

## Project goal

The aim of this project is to create a KernelShark plugin, with which one may
be able to access a sleeping process' stack data in the main visualization of
the trace and show them to the user.

### Bachelor thesis extensions

Bachelor thesis aims to extend and improve functionality of the plugin and to add NUMA topology
visualization support into KernelShark.

Another addition is a survey paper on stack tracing in Linux and basic visualization methods of trace data.
Along with that, a brief theoretical part on KernelShark, how it visualizes data from trace-cmd/perf
and a demonstration how the implemented plugin works with KernelShark, with examples.

## Development considerations

The plugin will be written in C++, using standard C++20 to allow newest
features. The project shall be compiled using CMake, which will allow more
flexible cross-platform compilation (it's also said to be friendlier than GNU
Make, at least toward the build script writer).

It is possible to either build the program from source or it's also possible to use
the `kernelshark` and especially `kernelshark-devel` packages. The project shall use
source-built KernelShark.

As a plugin, the software will use KernelShark's graphical capabilites and its
API.

## Project structure

- source code for the plugin
- Cmake build instructions
- example trace.dat file that can be visualized using the plugin
- (Bc extension) KernelShark's repository with modified parts of the code to allow
  project functionalities
- (Bc extension) survey paper

The goal of the plugin is not to start trace-cmd and record the stack, but
merely to show in KernelShark already captured stack data. The plugin is for
visualization and access to the captured data, so that's what it will be
structured as.

Since KernelShark also allows the user to use GUI to start `trace-cmd`, the project
will also include modifications to KernelShark's source code to enable
stacktrace capture in the GUI. For that purpose, KernelShark's GitHub repository
(https://github.com/yordan-karadzhov/kernel-shark) shall be forked and the fork
will be a part of the project.

- (Bc extension) Seminar project used a simple submodule inclusion, bachelor thesis project
  shall include the repository in full due to greater amount of changes that will be made,
  which feels more appropriate to fully include as part of the project's Gitlab repository

### Graphical structure (usage)

The user will be able to double-click on a button (e.g. shape of an upside down
isoceles, possibly with text saying "STACK") above the sleep-starting event of
a process. Doing so will open another window, in which the stack trace data
will be listed, one item under another in the order they are on the stack (the
last item that was pushed will be on the top). This window will of course be
closeable. Opening one window doesn't block others from being opened - be they
of the same event or different ones.

The plugin will also leverage the top status bar to show a shortened view of
the stacktrace. This way, the lookup of the process will be faster for quick
look at where the stack was at before going to sleep.

### Code

Code will have three main goals: get the stack data of a process, show buttons
for each event, that can be visualized and visualize the stack data themselves.
Getting the data will involve some filtering from the tracing file.
Visualization in the desired shape, with the desired text in the plot area will
be another thing to do, they will be the main entrypoint for the user and will
need to call upon the window-creating part of the plugin. Lastly, the windows
with the stack visualizations are our third problem, it will involve creating
more windows (pop-ups are an idea, though maybe too simple for our cause) to be
displayed, with the right data.

All of this should be much easier thanks to KernelShark's API, which allows for
data record finding and also drawing user interface objects, like buttons.

### Bachelor thesis extensions

Using formatted data (e.g. XML format) from Istopo program, KernelShark shall be able to visualize
NUMA topology and hyperthread sibling relationships in its graph.

- If KernelShark does NOT receive such a file, it shall not be able to create such a graph form and
  will instead use its default ordering style, i.e. CPU0, CPU1, ..., CPU(n-1), CPUn for n CPUs.
- To be precise, the extension should work like this:
  - No NUMA, no hyperthreading - default Kernelshark ordering
  - No NUMA, hyperthreading on - HT siblings are grouped together and marked as HT siblings,
    ordered by smallest sibling ID in a sibling group
    - (CPU0, CPU50) and (CPU1, CPU51) are two HT sibling groups, the CPU list in Kernelshark
      shall first show CPU0, then CPU50, then CPU1, then CPU51 to visualize the groupings easily
  - NUMA on, no hyperthreading - NUMA node primary ordering, cores in a NUMA node as secondary ordering and
    cores will be visibly marked as belonging to a node
  - NUMA on, hyperthreading on - NUMA nodes used for primary ordering, cores (in a NUMA node) used as secondary
    ordering, hyperthread siblings on a core used as ternary ordering
  - Visual representations of each grouping type ("belongs to NUMA node#X", "is HT sibling of X", "is part of core X")
    will differ to be easily distinguishable (e.g. via different colors)
- User shall be able to choose which display method will be used if both are possible (a few possible
  solutions are a menu option, a window dialog, command-line arguments)

Non-exclusive integration with KernelShark's existing `sched_events` plugin shall be implemented.
During the seminar project's development, it became clear the two plugins would step on each other's
toes and a discouragment has been written as part of the seminar project's documentation.
However, disabling an existing functionality is rather inelegant and restrictive.
This extension shall attempt to have both plugins coexisting.

- With past insights, this may be a complete refactoring of the original plugin into the source code of
  `sched_events`, which would then allow intimate manipulation of both of their internals such that neither
  plugin disrupts working of another.

Events which involve two processes shall be displayed on both process timelines. E.g. `sched_waking` process
is initiated by one process and wakes up another. This event is present only once though, as for the whole
system, the waking happened only once. Even so, it would be nice to see a process display `sched_waking (initator)`
and `sched_waking (being woken)` on both events separately or something similar, goal being friendlier
view for analysis.

- Extension should exist only within the plugin, though KernelShark source modification is another possibility.

### Example trace file

It should show situations, where using the plugin and looking at
the stack traces helps to figure out "what's the holdup". It might be
best to write a simple program to create such a trace. Screenshots
showing the view of the trace in KernelShark should be included.

### Documentation

Of course, the project will also include user documentation as well as
technical documentation. The user documentation should include
how the plugin is used as well as how to enable stacktrace capture in
KernelShark. The technical documentation will be about plugin's structure,
what design, classes and methods are used and should also include a part
about the change to KernelShark's source code.

## Similar projects

There aren't, to the author's knowledge, any KernelShark plugins that solve the
issue of allowing the user to view the stack trace from trace-cmd in
KernelShark, and KernelShark itself shows only scheduling events across CPUs
by default. As such, I will list other stack tracing programs that allow
visualization of stack tracing or the stack tracing itself, so as to have
something to compare with KernelShark + Stacklook.

A notable and apparently popular software to visualize stack traces is using
**FlameGraphs**. While impressive in execution, they are not necessary for a
stack of a single process at one point in time. However, they are a very
beautiful way of visualizing such data too.

Next on the list is **perf**, which collects and also visualizes stack data in
some capacity. It's way of visualizing the data is rather inspirational
too, as it displays them in a text format, which is common for it and
this project's idea of stack visualization. KernelShark

Another similar program is **Ftrace**. This is a trace-collecting program only
though and doesn't, by itself, allow any visualization of its data. While also
possible to have its output be read by KernelShark, it doesn't provide a way to
visualize such data.

A competitor for KernelShark when it comes to trace visualizations is
**LTTng**. This one supports viewing the stack through flame graphs and also
has other useful functionalities, like memory usage or time spent plotting of
functions. A nice program, but it's not KernelShark, so this plugin wouldn't
quite work for it.
