# Stacklook - a Kernelshark plugin for visualizing stack traces

## Intro to Kernelshark
Kernelshark is a tool for visualizing data taken from `trace-cmd` - such data
often helps when analyzing performance issues, testing and in
general checking how the system and application ran on the CPU(s). 
Kernelshark also offers a support of plugins for user's optional extensions of the tool.

## Project goal
The aim of this project is to create a Kernelshark plugin, with which one may
be able to access a sleeping process' stack data in the main visualization of
the trace and show them to the user.

## Development considerations
The plugin will be written in C++, using standard C++20 to allow newest
features. The project shall be compiled using CMake, which will allow more
flexible cross-platform compilation (it's also said to be friendlier than GNU
Make, at least toward the build script writer).

Though possible to build the program from source, it is also possible to use
the `kernelshark` and especially `kernelshark-devel` packages, which will be
used in this project.

As a plugin, the software will use Kernelshark's graphical capabilites and its
API.

## Project structure
- source code for the plugin
- source code for capturing stacktraces in Kernelshark's GUI
- Cmake build instructions
- example trace.dat file that can be visualized using the plugin

The goal of the plugin is not to start trace-cmd and record the stack, but
merely to show in Kernelshark already captured stack data. The plugin is for
visualization and access to the captured data, so that's what it will be
structured as.

Since Kernelshark also allows the user to use GUI to start `trace-cmd`, the project
will also include modifications to Kernelshark's source code to enable
stacktrace capture in the GUI.

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

All of this should be much easier thanks to Kernelshark's API, which allows for
data record finding and also drawing user interface objects, like buttons.

### Example trace file
It should show situations, where using the plugin and looking at
the stack traces helps to figure out "what's the holdup". It might be
best to write a simple program to create such a trace. Screenshots
showing the view of the trace in Kernelshark should be included.

### Documentation
Of course, the project will also include user documentation as well as
a technical documentation. The user documentation should include
how the plugin is used as well as how to enable stacktrace capture in
Kernelshark. The technical documentation will be about plugin's structure,
what design, classes and methods are used and should also include a part
about the change to Kernelshark's source code.

## Similar projects
There aren't, to the author's knowledge, any Kernelshark plugins that solve the
issue of allowing the user to view the stack trace from trace-cmd in
Kernelshhark, and Kernelshark itself shows only scheduling events across CPUs
by default. As such, I will list other stack tracing programs that allow
visualization of stack tracing or the stack tracing itself, so as to have
something to compare with Kernelshark + Stacklook.

A notable and apparently popular software to visualize stack traces is using
**FlameGraphs**. While very cool in execution, they are not necessary for a
stack of a single process at one point in time. However, they are a very
beautiful way of visualizing such data too.

Next on the list is **perf**, which collects and also visualizes stack data in
some capacity. However, perf is a different program than trace-cmd and also not
what Kernelshark uses. It's way of visualizing the data is rather inspirational
too, however, as it displays them in a text format, which is common for it and
this project's idea of stack visualization.

Another similar program is **Ftrace**. This is a trace-collecting program only
though and doesn't, by itself, allow any visualization of its data. While also
possible to have its output be read by Kernelshark, it doesn't provide a way to
visualize such data.

A competitor for Kernelshark when it comes to trace visualizations is
**LTTng**. This one supports viewing the stack through flame graphs and also
has other useful functionalities, like memory usage or time spent plotting of
functions. A nice program, but it's not Kernelshark, so this plugin wouldn't
quite work for it.