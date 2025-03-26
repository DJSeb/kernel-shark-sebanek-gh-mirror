<!-- Answer these:

- How do I use it?
- Bugs & Glitches

-->
# Intro

This document serves as a simple to grasp manual for the "naps" KernelShark plugin.

Seek information on how to build and install the software in the project's [README](../../README.md).

# "How do I enable/disable naps?"

Enabling the plugin is very simple. All one has to do is open KernelShark and navigate to 
`Tools > Manage Plotting plugins` toolbar menu button. If the plugin was loaded via the command-line interface,
it will be shown in the list of plotting plugins as a checkbox plus the name, checkbox already being ticked.
If not, it is possible to search for the plugin via provded `Tools > Add plugin` button - it's sufficient to
find the symlink, but searching for the actual shared object file is possible too. As you can see, the plugin
follows standard KernelShark plugin loading behaviour.

Ticked checkbox means the plugin is enabled, empty checkbox means the plugin is disabled.

# "How do I use naps?"

## Configuration

If the plugin is enabled, additional button will appear in `Tools` menu with the label `Naps Configuration`.
Clicking on it will show a window dialog, which will house configuraton options for the plugin. The only configuration
optionsavailable for this plugin is the maximum amount of entries visible on the graph before the plugin is allowed
to work. This configuration option can help if there are either not enough visible plugin shapes for a current zoom
level or if there are too many and program memory is is too great.

Clicking on `Apply` button will confirm changes made to the configuration and close the window, showing a pop-up if
the the operation was successful. Clicking on the `Close` button or the X button in the window header will close the
window without applying any changes. Changes made to a window that hasn't applied them to the configuration will be
lost and upon reopening, the window will again show what's in the currently applied configuration.

## In the graph

# Bugs & glitches

There is only one crashng bug known right now - starting KernelShark without installing the plugin beforehand
and then loading a session, where this plugin was active, will result in a segmentation fault.

The solution to this is to install/load the plugin before loading said session.