# Project Overview

**NoBoxes** (also referred to as noboxes or just Plugin) is a plugin for the trace-cmd data visualiser KernelShark, which
modifies visibility of some entries' bins to not partake in drawing of taskboxes, i.e. colorful boxes between bins in graphs.

## Purpose

It's main purpose is to disable bugs `ftrace/kernel_stack` and `couplebreak/sched_waking[target]` event entries bring. Kernel stack
traces recored after a `sched/sched_switch` event confuse the graph into drawing work that doesn't exist. The plugin utilises a new
KernelShark modification to hide these boxes specifically.

## Features
- Registers an event handler when activated, which uses KernelShark modfication (tagged NOBOXES in the source code), which masks a
  bit in the visibility field of an entry, which is then checked during KernelShark's drawing of a graph - if the bit is unset,
  task box either passes through the entry's bin or doesn't draw the taskbox, if it would start from this entry.

# Project layout
- NoBoxes (root directory, this directory)
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
  - *LICENSE*

# Usage & documentation

For user documentation refer to the [user manual](./doc/user/user-manual.md).

Code is heavily commented, up to every private function. For detailed understanding of how Plugin works,
refer to the source files in `src` directory.

Technical documentation has to be generated via Doxygen and the included
Doxyfile.

For a design document, please refer to the `doc/technical` directory, as this is included in the technical documentation,
or read it unprocessed in file `doc/design.doxygen`.

For API documentation, please refer to the `doc/technical` directory.

# Acknowledgments

- The idea of this plugin was suggested by **Jan Kára RNDr.**, who also serves as project's **supervisor**.
- **Yordan Karadzhov** - maintains KernelShark, general plugin structure was inspired by his work 
- ChatGPT deserves the "greatest evolution of the rubber ducky" award - not very helpful, but great to just unload bad ideas onto
- **Doxygen Awesome CSS** - [beautiful CSS theme](https://jothepro.github.io/doxygen-awesome-css/index.html)
  that makes Doxygen documentation look visually pleasing

# About author

David Jaromír Šebánek (e-mail: `djsebofficial@gmail.com`). Direct all inquiries about this plugin there.

# License

This plugin uses [MIT licensing](./LICENSE). KernelShark uses LGPL-2.1.