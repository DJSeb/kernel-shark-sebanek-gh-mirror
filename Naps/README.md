# Project Overview

**Naps** (also referred to as Plugin) is a plugin for the trace-cmd data visualiser KernelShark. Nap, in Plugin's definition, is the space in a single task's plot between a switch event and a waking event which wakes this task again. Plugin also informs the user about the previous state of the task just before the switch.

## Purpose
It's main purpose is to show duration and previous state of a task that has just switched and will wake up during the trace again. Such feature is not included in KernelShark or its default plugins. Closest one may get is the sched_events plugin, which only visualises switch-to-switch or waking-to-switch portions of the trace.

## Features
- Plugin will visualise naps as rectangles between events *sched/sched_switch* & either *sched/sched_waking* or *couplebreak/sched_waking\[target\]*. This is chosen during plugin's load per data stream, by detection of couplebreak's on/off status in each stream. If couplebreak is inactive, the sched-generated event will be used, otherwise couplebreak's generated target event will.
- Plugin will color the nap rectangles using information from previous state of the task and its PID. PID-color is determined internally by KernelShark, Plugin only accesses it and uses it for the rectangle's outlines, if the
experimental coloring option was checked in Plugin's configuration. Color from the previous state is used to fill
the shape's insides. It is chosen via to a state-to-color mapping as follows:
  - Uninterruptible disk sleep -> red
  - Idle -> Yellow
  - Parked -> Orange
  - Running -> Green
  - Sleeping -> Blue
  - Stopped -> Cyan
  - Tracing stop -> Brown
  - Dead -> Magenta
  - Zombie -> Purple
- Plugin will also write into the rectangle, if it is wide enough, the full name and abbreviation of the previous state of the task. The text will be either black or white, chosen via color intensity of the background color.
- If couplebreak is OFF in the loaded stream, Plugin will change the owner PID of a *sched/sched_waking* entry to allow it to be displayed in the task plot - otherwise there wouldn't be a waking event entry to connect the switch to, as waking entries by default belong to the task waking another (it is its work). This may be a major source of incompatiblity with other plugins and **using couplebreak is suggested**.
- Plugin may be built to work with unmodified KernelShark, but compatibility with other plugins, especially
  sched_events, will be sacrificed.
- Plugin offers an experimental coloring version, which will change the top and bottom outline of nap rectangles,
to be of the same color as the task owning said rectangle. This option has to be turned on in the configuration
upon each plugin load, as it is not persistent. It couldn't be hardwired into the code due to issues with session
loads.

# Project layout
- Naps (root directory, this directory)
  - build
    - if present, houses *build files*
    - if present, might contain bin directory
      - houses *built binaries*
  - doc
    - technical
    - user
      - *user-manual.md*
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

# Building & installing

## Compatibility

Plugin should is compatible with KernelShark's **custom** version *2.4.0-couplebreak* and higher.
Unmodified KernelShark usage is achievable through a build argument, but **discouraged**, as the
custom versions improve compatibility with other plugins.

With couplebreak on, the plugin is fully compatible with every default KernelShark plugin. With couplebreak off,
sched_events plugin is incompatible. In general, if any plugin changes event's PID data during data stream loading,
this plugin will be incompatible with it.

## Build and install only this plugin

1. Change directory to the build directory (best created in the root directory, if not already present).
2. Run `cmake ..` command.
   - If using an unmodified KernelShark copy, add `-D_UNMODIFIED_KSHARK` to the command.
   - If **Doxygen documentation** is desired, include `-D_DOXYGEN_DOC=1` in the command.
   - If **trace-cmd header files** aren't in `/usr/include`, specify so via `-D_TRACECMD_INCLUDE_DIR=[PATH]`, where
    `[PATH]` is replaced by the path to the header files.
   - If **trace-cmd shared libraries** aren't in `/usr/lib64`, specify so via `-D_TRACECMD_LIBS_DIR=[PATH]`, where
    `[PATH]` is replaced by the path to the shared libraries.
   - By default, the **build type** will be `RelWithDebInfo` - to change this, e.g. to `Release`, use the option `-DCMAKE_BUILD_TYPE=Release`.
   - If **Qt6 files** aren't in `/usr/include/qt6`, use the option `-D_QT6_INCLUDE_DIR=[PATH]`, where `[PATH]` is replaced by the path to the Qt6 files.
     - Build instructions still expect that the specified directory has same inner structure as the default case (i.e. it contains `QtCore`, `QtWidgets`, etc.).
   - If **KernelShark source files** aren't in the relative path `../KS_fork/src` from this directory, use
     the option `-D_KS_INCLUDE_DIR=[PATH]`, where `[PATH]` is replaced by the path to KernelShark source files.
   - If **KernelShark's shared libraries** (.so files) aren't in `/usr/local/lib64`, use the option `-D_KS_SHARED_LIBS_DIR=[PATH]`, where `[PATH]` is replaced by the path to KernelShark shared libraries.
3. Run `make` while still in the `build` directory.
   - If only a part of building is necessary, select a target of your choice.
   - Just running `make` builds: **the plugin** (target `naps`), **symlink** to the plugin SO (target `naps_symlink`) and, if specified, the **Doxygen documentation** (target `docs`).
4. Plug in the plugin into KernelShark - either via KernelShark's GUI or when starting it via the CLI with the `-p` 
   option and location of the symlink or the SO itself.

Instructions will remove the binary on `make clean`, but won't remove the symlink.

## Building KernelShark from source and this plugin with it

1. Ensure all source files (`.c`, `.cpp`, `.h`) of Naps are in the `src/plugins` subdirectory of your KernelShark project directory.
2. Ensure the `CMakeLists.txt` file in said subdirectory contains instructions for building the plugin (copy the style of other Qt-using GUI plugins).
3. Build KernelShark (plugins are built automatically).
4. Start KernelShark. Plugins built this way will be loaded automatically. If that for some reason failed, look for the SO as for any other default-built KernelShark plugin, again in GUI or via the CLI.

# Usage & documentation

Code is heavily commented, up to every private function. For detailed understanding of how Plugin works,
refer to the source files in `src` directory.

For a design document, please refer to the `doc/technical` directory, as this is included in the technical documentation,
or read it unprocessed in file `doc/design.doxygen`.

For API documentation, please refer to the `doc/technical` directory.

For user documentation refer to the [user manual](./doc/user/user-manual.md).

# Acknowledgments

- The idea of this plugin was suggested by **Jan Kára RNDr.**, who also serves as project's **supervisor**.
- **Yordan Karadzhov** - maintains KernelShark, inspiration in already written plugins was of immense help
- ChatGPT deserves the "greatest evolution of the rubber ducky" award - not very helpful, but great to just unload bad ideas onto
- **Doxygen Awesome CSS** - [beautiful CSS theme](https://jothepro.github.io/doxygen-awesome-css/index.html)
  that makes Doxygen documentation look visually pleasing

# About author

David Jaromír Šebánek (e-mail: `djsebofficial@gmail.com`) Direct all inquiries about this plugin there.

# License

This plugin uses [MIT licensing](./LICENSE). KernelShark uses LGPL-2.1.