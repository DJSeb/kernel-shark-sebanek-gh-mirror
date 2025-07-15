# Project Overview

**Stacklook** (also referred to as stacklook or just Plugin) plugin for visualizing kernel stack traces in
KernelShark's trace graph above sched_switch and sched_waking entries. Plugin also informs the user about the previous
state of the task just before the switch.

## Purpose

It's main purpose is to allow a more graphically friendly access to kernel stack traces of events. This is an
evolution of KernelShark's ability to display kernel stack traces in its list view of a trace. No plugin that achieves
an interactive visualisation exists however and this is the hole that Stacklook is trying to fix.

## Features

- Above each `sched/sched_switch` and `sched/sched_waking` event in the KernelShark plot, an upside down triangle
  button (also called **stacklook button**) will appear when the plugin is loaded in and there aren't too many entries
  visible (this is configurable).
  - Double clicking on such button will spawn a window with the full kernel stack trace in its text form.
  - The button will be labeled "STACK" an if it is above a `sched/sched_switch`, it will also include a label below
    with an abbreviation of the previous state the task was in before it switched, enclosed in parentheses.
    - _Example_: a task was preempted while it was waiting for an HDD operation to finish - the stacklook button will
      show the text "STACK" and below it "(D)", as D is abbreviation for "uninterruptible (disk) sleep".
  - _If built for custom KernelShark_, hovering over buttons with the mouse cursor will show the top three items on the
    kernel stack for this entry and the name of the task this entry belongs to. The preview skips first three entries
    in the stack (this is configurable).
    - If there are no entries to display (or we are going over the stack), dashes will be shown instead and the last
      label in the preview will notify us that we are seeing the end of the stack.
    - Moving away from the button's boundaries will clear the preview.
  - _If built for custom KernelShark_, the button's filling color may be configured to be the same as the one the task
    owning the entry above which a button is.
- **Stacklook windows** show the kernel stack at the time of an event. The stack is in text form and can be viewed as
  raw text with newlines or a list of strings. A text is shown above the stack text with the name of the task the
  stack belongs to.
  - The raw view is more useful for text copying, the list view for specific stack entry highlight.
  - If the stacklook button was above a `sched/sched_switch`, information about the previous state of the task is also
    shown in the window. If the event was instead a `sched/sched_waking`, text saying that the task has woken up will
    be present.
  - The windows can be closed, resized and minimzed. When KernelShark's main window closes, so do the, but not the
    other way around.
  - There can be multiple windows spawned for a single entry or different entries.
  - Windows do not spawn their own processes, rather they are graphical elements under KernelShark's hierarchy.
- Plugin adds a configuration window. It can be accessed via KernelShark's main window via
  `Tools/Stacklook Configuration`. It is possible to configure:
  - The limit of visible entries before the plugin kicks in
  - _If built for custom KernelShark_, Using task colors for stacklook buttons
  - Default color of Stacklook's buttons and the default color of their outlines
  - Plugin's meta information of supported events:
    - Whether to show Stacklook buttons above them
    - _If built for custom KernelShark_, how great should the offset into the kernel stack for the preview be

# Project directory layout

- Stacklook (root directory, this directory)
  - build
    - if present, houses _build files_
    - if present, might contain bin directory
      - houses _built binaries_
  - doc
    - technical
    - user
      - _user-manual.md_
      - _user-manual.odt_
      - _user-manual.pdf_
    - images
      - images used in both user and technical documentations
    - doxygen-awesome-css (houses files which prettify doxygen output)
    - _mainpage.doxygen_
    - _design.doxygen_
    - _Doxyfile_
  - src
    - _CMakeLists.txt_ (Further CMake instructions for building the binary)
    - **source files of the plugin**
  - _CMakeLists.txt_ (Main build file)
  - _README.md_ (what you're reading currently)
  - _LICENSE_

# Usage & documentation

For user documentation refer to the [user manual](./doc/user/user-manual.md) (slightly out of date in comparison
to its Czech version).

Code is heavily commented, up to every private function. For detailed understanding of how Plugin works,
refer to the source files in `src` directory.

Technical documentation has to be generated via Doxygen and the included Doxyfile.

For a design document, please refer to the `doc/technical` directory, as this is included in the technical
documentation, or read it unprocessed in file `doc/design.doxygen`.

For API documentation, please refer to the `doc/technical` directory.

# Acknowledgments

- The idea of this plugin was suggested by **Jan Kára RNDr.**, who also serves as project's **supervisor**.
- **Yordan Karadzhov** - maintains KernelShark, inspiration in already written plugins was of immense help
- ChatGPT deserves the "greatest evolution of the rubber ducky" award - not very helpful, but great to just unload bad ideas onto
- **Geeksforgeeks** - [Check if a point is inside a triangle](https://www.geeksforgeeks.org/check-whether-a-given-point-lies-inside-a-triangle-or-not/)
- **Markdown PDF** - VS Code extension, allowed me to export the manual with pictures easily
- **Doxygen Awesome CSS** - [beautiful CSS theme](https://jothepro.github.io/doxygen-awesome-css/index.html)
  that makes Doxygen documentation look visually pleasing

# About author

David Jaromír Šebánek (e-mail: `djsebofficial@gmail.com`). Direct all inquiries about this plugin there.

# License

This plugin uses [MIT licensing](./LICENSE). KernelShark uses LGPL-2.1.

