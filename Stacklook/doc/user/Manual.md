# Manual for Stacklook

## Using Stacklook in KernelShark

### How to build

Please follow the [guide in README](../../../README.md).

### Load it up

Having built a KernelShark shared library, remember
where it is. Then, when starting KernelShark from
the terminal, add `-p [path/to/plugin/binary]` to
the `kernelshark` command. This approach will
follow symlinks.

Alternatively, you can start KernelShark yourself and
then use the GUI the add the plugin that way via the
`Tools > Add plugins` button.

### Use in a plot

To use the plugin, have a trace file ready which includes
*sched/sched_switch* and *sched/sched_wakeup* event entries.

After loading the plugin, zoom until less than 200 entries
are visible in the histogram plot. If there are any of the
above entries, a button will show up, colored like the task
the event is for.

Since the color theme can be changed in KernelShark, the
buttons respect this possibility, albeit they need the user
to zoom out and back in for this to take effect.

[Image of buttons]

If *ftrace/kernel_stack* events were not collected, no stack
information will be shown upon moving the cursor over the
buttons or double-clicking on them, except error messages.

[Images of errors here]

Else, upon mouse hover/move over a button, the name of
the task and the first three items from the top of the
kernel stack will be shown.

[Image of preview change here]

Moving away from the button's boundaries will show nothing.

Double-clicking on a button will bring up a new window,
which will show the full kernel stack, either as raw text
or in the form of a list of strings. Both will have "(top)"
written above the stack, signifying where the top of the
stack is. It is possible to toggle between the raw text
view and the list view using two radio buttons above the
view area. On the top of the window is a small message
with the name of the task from which we had the kernel
stack traced.

The window can be closed with the "Close"
button at the very bottom of the window or with the X
button of the window's header. Last option is to close
the main KernelShark window, which will close all of
Stacklook's opened windows.

There can be more than one window opened for a single
event and there can be more windows with different
events open (performance shouldn't be demanding, but
it isn't recommended to open hundreds upon hundreds).

[Image of a KernelShark window here]

## Using Stacklook as a library

### It's not really a library

Stacklook was built as a standalone piece of
software, not meant as a library (it is a shared library
solely because this is what KernelShark expects).

Still, since there are header files, it is good to
mention what is in them just in case it is needed.

### What's in the header files?

I recommend looking into the **technical Doxygen**
**documentation**. Below are only short summaries
of each member of header files.

In **stacklook.h**:
- Plugin context struct `plugin_stacklook_ctx`, which
  serves as a sort of plugin-wide global variable collection.
  It stores event IDs of collected events, container of
  collected events and a pointer to a collection of
  plugin's opened windows.
  
  A possible extension could be a collection of plugin's
  buttons so that one can manipulate them from anywhere.

- Global functions:
    * `get_font_ptr` - getting a pointer to KernelShark's
      font
    * `draw_plot_buttons` - draw handler for drawing buttons
      on the plot
    * `plugin_set_gui_ptr` - gets a pointer to KernelShark's
      main window during plugin load
    * `clean_opened_views` - cleans the opened window collection
    * `init_views` - initializes the opened window collection
    - Last four are defined in C++, first one in C. This is
      due to the necessities of using all of them in C code,
      but them needing to work with C++ structures.

In **SlButton.hpp**:
- Class `SlTriangleButton`, used for drawing the buttons.
  It is also responsible for the necessity of modified
  KernelShark build, due to it having a mouse hover action
  defined. It is also used to draw the buttons in a way
  which makes their parts not bleed into each other,
  thanks to it being a composite of other plot objects.

  This class also houses all logic of plugin's buttons'
  inner workings. As a child of the general plot object
  it can be used in graphs. In fact, it is, without this
  plugin's overall context, just a button that can do
  actions like a plot object and stores which entry it is
  gaining data from.

In **SlDetailedView.hpp**:
- Class `SlDetailedView`, class that represents a window
  with more detail about a kernel stack entry of an event.
  Class itself holds a pointer to the main KernelShark
  window and a pointer to all opened windows. Otherwise
  it is mostly composed of other Qt objects.