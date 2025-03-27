# Intro

This document serves as a simple to grasp manual for the "naps" KernelShark plugin.

![Fig. 1](../images/SlWorking.png)
Figure 1.

# "How do I build and install Stacklook?"

## Prerequisites

- CMake of version at least 3.1.2
- KernelShark and its dependencies
  - version *2.4.0-couplebreak* and higher for custom KernelShark
  - version *2.3.2* for unmodified KernelShark

## Compatibility

Plugin is compatible with KernelShark's **custom** version *2.4.0-couplebreak* and higher.
Unmodified KernelShark usage is achievable through a build argument. Unmodified KernelShark removes
these features from the plugin:
- Buttons optionally being the same color as the task is in the graph
- Mouse hover showing a preview of the kernel stack with an adjustable stack offset for it in the configuration

No other dependencies are necessary, except maybe the standard libraries of C and C++.

## Build and install only this plugin

1. Set your working directory in terminal as the build directory (best created in the project's root directory (see
   [README](../../README.md)), if not already present).
2. Run `cmake ..` command (if the main `CMakeLists.txt` file isn't in the parent folder, provide cmake with its
   valid location).
   - If using an unmodified KernelShark copy, add `-D_UNMODIFIED_KSHARK` to the command.
   - If **Doxygen documentation** is desired, include `-D_DOXYGEN_DOC=1` in the command.
   - By default, the **build type** will be `RelWithDebInfo` - to change this, e.g. to `Release`, use the option `-DCMAKE_BUILD_TYPE=Release`.
   - If **Qt6 files** aren't in `/usr/include/qt6`, use the option `-D_QT6_INCLUDE_DIR=[PATH]`, where `[PATH]` is 
     replaced by the path to the Qt6 files.
     - Build instructions still expect that the specified directory has same inner structure as the default case (i.e. 
       it contains `QtCore`, `QtWidgets`, etc.).
   - If **KernelShark source files** aren't in the relative path `../KS_fork/src` from this directory, use
     the option `-D_KS_INCLUDE_DIR=[PATH]`, where `[PATH]` is replaced by the path to KernelShark source files.
   - If **KernelShark's shared libraries** (`.so` files) aren't in `/usr/local/lib64`, use the option
     `-D_KS_SHARED_LIBS_DIR=[PATH]`, where `[PATH]` is replaced by the path to KernelShark shared libraries.
3. Run `make` while still in the `build` directory.
   - If only a part of building is necessary, select a target of your choice.
   - Just running `make` builds: **the plugin** (target `stacklook`), **symlink** to the plugin SO (target 
     `stacklook_symlink`) and, if specified, the **Doxygen documentation** (target `docs`).
4. (**Installation**) Plug in the plugin into KernelShark - either via KernelShark's GUI or when starting it via the 
   CLI with the `-p` option and location of the symlink or the SO itself.

Instructions will remove the binary upon running `make clean`, but won't remove the symlink.

## Building KernelShark from source and this plugin with it 

1. Ensure all source files (`.c`, `.cpp`, `.h`) of Naps are in the `src/plugins` subdirectory of your KernelShark project directory.
2. Ensure the `CMakeLists.txt` file in said subdirectory contains instructions for building the plugin (copy the style of other Qt-using GUI plugins).
3. Build KernelShark (plugins are built automatically).
4. (**Installation**) Start KernelShark. Plugins built this way will be loaded automatically. If that for some reason failed, look for the SO as for any other default-built KernelShark plugin, again in GUI or via the CLI.

# "How do I enable/disable Stacklook?"

Enabling the plugin is very simple. All one has to do is open KernelShark and navigate to 
`Tools > Manage Plotting plugins` toolbar menu button. If the plugin was loaded via the command-line interface,
it will be shown in the list of plotting plugins as a checkbox plus the name, checkbox already being ticked.
If not, it is possible to search for the plugin via provded `Tools > Add plugin` button - it's sufficient to
find the symlink, but searching for the actual shared object file is possible too. As you can see, the plugin
follows standard KernelShark plugin loading behaviour.

![Fig. 2](../images/SlManagePlottingPlugins.png)
Figure 2.

Ticked checkbox means the plugin is enabled, empty checkbox means the plugin is disabled.

# "How do I use Stacklook?"

## Configuration

Plugin configuration can be done at any time, even before any trace file is loaded. To use it, simply open the dialog
window through `Tools > Stacklook Configuration` in the main window. There can be only one window open at a given time.

<!--TODO-->

The "Apply" button will save the changes made an close the dialog - if not pressed, changes made won't take effect. 
Only active confguration values show up in the control elements - the configuraton window doesn't persist changes made 
to it after closing, unless they were applied. The "Close" button and top-right corner "X" button will discard changes 
made in the window and close the dialog.

There is no configuraton of:
- Supported events - plugin currently only support `sched/sched_switch` and `sched/sched_waking`
- The text in stacklook windows
- The text in stacklook buttons
- Stacklook button sizes
- Stacklook button positions

## In the graph

After loading (and maybe configuring) the plugin, zoom in until less than the configured number of visible entries are 
visible in the graph. A button will show up above each supported entry, colored by the default color in the 
configuration, or if using custom KernelShark and having the corresponding option on, colored like the task in the 
graph. The task color option is fully compatible with KernelShark's color slider.

<!--TODO-->

The plugin won't show the buttons above event entries that aren't `sched/sched_switch` or `sched/sched_waking`, or if
such entries are missing their kernel stack trace event.

## Stacklook windows

<!--TODO-->

Toggle between the raw text view and the list view using two radio buttons above the stack view area.

The window can be closed with the "Close" button at the very bottom of the window or with the "X" button of the 
window's header. Last option is to close the main KernelShark window, which will close all of Stacklook's opened 
windows.

## Using Stacklook as a library

See technical documentation, as this is not intended usage of the plugin and such usage explanations will be omitted.

# Bugs & glitches

If there are multiple stacklook buttons would lay over one another, the one belonging to an *earlier* event will be 
drawn above a button for an event happening *later*, **BUT** clicking on or hovering over a button will select the 
button for the *later* event. This is, to the author's knowledge, something internal to KernelShark and cannot be
fixed on the plugin side.

If more are discovered, contact the author via e-mail `djsebofficial@gmail.com`.

# Recommendations

A few recommendations of usage by the author for the smoothest user experience.

Do not open hundreds upon hundreds of stacklook windows, lest you wish your memory to suffer.

It is recommended to not set the histogram limit in the configuration too high as to not make the plugin use
too much memory with many of stacklook's buttons being present.

While KernelShark's sessions work, they are a little buggy. This plugin attempts its best to not get in the way of
their inner logic, but a warning should be issued that if the plugin isn't loaded beforehand, there might be
unexpected behaviours, e.g. loading a session when the plugin was active won't add the plugin's menu to the
`Tools` menu.