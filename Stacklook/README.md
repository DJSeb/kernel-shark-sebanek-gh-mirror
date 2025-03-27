# Project Overview

**Stacklook** (also referred to as stacklook or just Plugin) plugin for visualizing kernel stack traces in 
KernelShark's trace graph above sched_switch and sched_waking entries. Plugin also informs the user about the previous
state of the task just before the switch.

## Purpose
It's main purpose is to show allow a more graphically friendly access to kernel stack traces of events. This is an
evolution of KernelShark's ability to display kernel stack traces in its list view of a trace. No plugin that achieves
an interactive visualisation exists however and this is the hole that Stacklook is trying to fix.

## Features

<!--TODO-->

# Project directory layout

- Stacklook (root directory, this directory)
  - build
    - if present, houses *build files*
    - if present, might contain bin directory
      - houses *built binaries*
  - doc
    - technical
    - user
      - *user-manual.md*
      - *user-manual.odt*
      - *user-manual.pdf*
    - images
      - images used in both user and technical documentations
    - doxygen-awesome-css (houses files which prettify doxygen output)
    - *mainpage.doxygen*
    - *design.doxygen*
    - *Doxyfile*
  - src
    - *CMakeLists.txt* (Further CMake instructions for building the binary)
    - **source files of the plugin**
  - *CMakeLists.txt* (Main build file)
  - *README.md* (what you're reading currently)

# Usage & documentation

For user documentation refer to the [user manual](./doc/user/user-manual.md).

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