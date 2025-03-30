<!--TODO-->

# Source code modifications navigation

Each modification has a source code tag in a comment, which navigates to parts of code relevant to that modification.
Each modification is enclosed in `//NOTE: Changed here. ([TAG]) ([DATE OF CHANGE])` starting comment and `// END of change`
closing comment, \[TAG\] being a modification tag and \[DATE OF CHANGE\] being a YYYY-MM-DD date of when a change was made.

<!------------------------------------------------OLD----------------------------------------------------->
# KernelShark modifications

# Modifications
This modified KernelShark has had 4 main changes made:
1) `kshark-record` now allows to optionally trace the kernel stack
   on each event through `trace-cmd`'s `-T` option.
2) `KsPlot::PlotObject` class has two new member functions:
    - Public function `mouseHover`, which works just like `doubleClick`,
      but invokes the other new function (below)
    - Private virtual function `_mouseHover`, which specifies an
      action to do when the mouse hover over an object.
3) In `KsGLWidget.cpp`, there has been added a static function
   `_mouseMoveOverPluginShapes` which works almost the same as
   `mouseDoubleClickEvent` (the only difference is place of
   invocation, its staticity and it not finding and selecting an
   event on left mouse button). It is called in `mouseMoveEvent`
   at the very end.
4) `KsTraceGraph` got an additional public member function `setPreviewLabels`,
   which allows other code that can get hold of the graph to change
   the six information labels next to the "Pointer" field in the main
   window's top section.

# Why were they made?

Firstly, specification demanded some kind of way to make KernelShark
catch stack traces.

Secondly, specification demanded mouse hover functionality over plugin's
buttons, which would show short stack information in preview text-boxes
in the top part of KernelShark's main window.

Neither of these was possible with existing API.

# What has been changed?

@subsection capturing Capturing the (kernel) stack

When it comes to capturing the stack trace, all that needed to be done
was adding a check-box button via Qt's API. This way, the `kshark-record`
program was allowed to use the `-T` option of `trace-cmd`, which captures
kernel stack via `ftrace` on each event captured by `trace-cmd`. Check-box's
state was used as a toggle between capturing the stack and not doing so.

@subsection hover_func Action on mouse hover

Adding the hover functionality required a bit more work. Firstly, the
`KsPlot::PlotObject` class had to be extended with functions
`_mouseHover` and `mouseHover`. Both work exactly the same as their
`doubleClick` counterparts from original code - the one with the underscore
serves as a private virtual function for actions that will happen when
mouse is hovered/moved over the plot object. The underscore-less one
is a public function which will call the underscored one when the plot
object is visible.

Of course this also required KernelShark to call the public function
when certain conditions were met. Because the idea is very similar,
most of the code from `mouseDoubleClickEvent` function in `KsGLWidget.cpp`
file was copied and pasted into a static function the same file called
`_mouseMoveOverPluginShapes`. In short, it checks all plot objects
when the mouse is moved and if they are under the cursor, the `mouseHover`
function is called. To call the static function, a little line of code
was added at the very end of `mouseMoveEvent` function, which also reacts
to hovering, but only when it occurs above data entries in the plot.

Plugin's buttons use this to manipulate what's shown in the preview bar.

@subsection preview_func Seeing short stack information in the preview bar

Once the hovering was done, the last job was a way to change information
in the upper preview bar (next to the "Pointer" field in the main window).
For this, the class `KsTraceGraph` got a new public function called
`setPreviewLabels`, which would allow anyone, who could get a hold of the
graph (which is possible thanks to the API) to change the preview bar.

The function itself will set everything to empty string, thanks to
default arguments, but the plugin will show task's name, the first
three items it had on the kernel stack and then ellipsis.

@section dependency Can I use the plugin without the modifications?

Kind of, but not easily.

The stack capture modification is only relevant for `kshark-record`.
KernelShark will accept any trace-cmd output, so it is possible to just create
the output with stack traces by hand and only then give it to KernelShark.

The bigger problem lies in the hover functionality of the buttons.
If it were removed, then there is no need for the preview bar manipulation
either. As such, the code that relies on these two functionalities
includes a warning that everyone using the plugin as is needs to have
a modified KernelShark.

@subsection the_how How to use Stacklook without modified KernelShark

Consider your KernelShark is unmodified. Then, delete the declaration of
the override of `_mouseHover` function in the `SlButton.hpp` file and
subsequently remove the definition from the `SlButton.cpp` file.

Both of these are found at the bottom of their files and are the only
parts of code that have the tags `WARN` or `warning` in comments above them.
