# Stacklook
*Plugin for visualizing stacktraces in KernelShark.*

### Author

David Jaromír Šebánek

### Supervisor

Jan Kára RNDr.

## Repository layout

You are in **root**.

- **root**
    - Directory `Stacklook` <= All files related to the plugin
        - Directory `src` <= All source code of the plugin
        - Directory `doc` <= Doxygen configuration and documentation files
        - Other files include: `CMakeLists.txt`
    - Directory `KS_fork` <= Modified copy of the main KernelShark repository with changes
    - File `CHANGELOG.md` <= Brief change summaries to the program, starting from the first version that basically worked
    - File `ProjectSpecification.md` <= Outlines how the project should look like
    - Other files include: `LICENSE`, `.gitignore`, `.gitmodules`, `README.md` *(you are reading this \:D)*.


## Project status

At version **1.4.3**.

Semester project part is finished.

Bachelor thesis project part WIP.

### NPRG045 general tasks

- [x] Brief project description
- [x] Specification
- [x] Basic working version*
- [x] Final version

\* The most important things work.

### Changelog

[Here.](./CHANGELOG.md)

### Roadmap

Things not in specification aren't mandatory, but would be pretty useful.

- [x] KernelShark Record adjustment for stacktracing
- [x] Stacklook
    * [x] Get scheduler event for when the task was about to go to sleep and the stacktracing event
    * [x] Add clickable shapes above each sleep event
        * [ABANDONED] *(Not in specification) Add such clickable shapes into entry viewer*
            * NOTE: It is highly unlikely that this is easily doable as a plugin, unless a lot of KernelShark is rewritten.
    * [x] Show a dialogue/window with detailed stack information
    * [x] Display preview of the stack in the top info bar
    * [x] Create CMake build instructions
        * NOTE: Could be circumvented if the plugin files were put into KernelShark's `src/plugins` directory
    * [x] Create plugin documentation (user & technical)
        <!-- NOTE: Limit thyself, author -->
        * [x] Create build instructions for the documentation
        * [x] Technical documentation (Doxygen)
        * [x] User documentation (Markdown or HTML)
    * [x] *(Not in specification) Add a settings menu for the plugin*
    * [x] *(Not in specification, but requested)* Visualization of prev_states after a sched/sched_switch
    * [x] *(implied)* Make it work for CPU and task plots
* [x] Create an example `trace.dat` file for demonstration
    * [x] Write a program that will do some stack shenanigans (`recursive_syscall_with_sleep`)
    * *NOTE:* It isn't great, but it does show nanosceond sleep :\)
    * Will also include a very small trace-file to show some simple stuff (`simple_trace.dat`)

Below are bachelor thesis part's appended requirements:
* [ ] Events involving two processes shall be split into initiators and targets
    * E.g. sched_waking should be split into "awaker" and "awakened" events, one
      belonging to each respective process
    * [ ] Try to keep the logic inside the plugin (minimizing KernelShark changes)
* [ ] NUMA topology visualization in KernelShark (either as a plugin or KernelShark modification)
    * [ ] Parse data from Istopo (XML format)
    * [ ] Visualize said data on the screen
        * [ ] If no Istopo data are given, use default KShark visualization
        * [ ] CPU reordering according to topology (NUMA nodes, hyperthread siblings)
        * [ ] Tree grouping of CPUs
            * [ ] May be collapsible (preferred)
            * NOTE: Other grouping visualizations are possible, if
              CPUs are sorted by topology, but tree-like is the one
              that will be implemented as basis
        * [ ] Colorful differentiation of different groups
    * [ ] Configurable display method via some option in KShark's menu
* [ ] Allow `sched_events` plugin compatibility
    * XOR choice
    * [ ] Either integrate Stacklook into sched_events plugin
        * Would violate the holy Single Responsibility principle
        * Direct approach
        * Ensured success
    * [ ] Or figure out how to allow compatibility while keeping plugins separate
        * Less direct
        * Keeps SRP
        * Unknown probability of success
General SW goal:
* [ ] Debug the plugin even more, stabilise performance where necessary
    * This requirement will likely grow in the future
    * BUG: Changing trace files in opned KShark seg-faults.
        * Environment: WSL2 openSUSE Tumbleweed
        * Cause: Unknown
    * PERF: many entries might produce too heavy a load for mouse hover functionality
* [ ] Refactor README and other non-code parts of repository to reflect project extensions 

## Requirements

Modified KernelShark (preview functionality is unusable otherwise).

It is possible to create a plugin that would work with unomdified
KernelShark version 2.3.1 (code that would need to be removed
contains a comment about this).

**Newly from 1.3.1**: By specifying CMake variable `_UNMODIFED_KSHARK`,
CMake will be able to add a definition of a preprocessor variable of the
same name and compile the plugin SO file appropriately.

## Compatibility

The user is **REQUIRED** to turn off "sched_events" plugin, which causes issues
because of reassigning "next" fields of entries. Plugin will **NOT** work
correctly with "sched_events" enabled.

## Documentation

[User manual](./Stacklook/doc/user/Manual.md)

Technical (Doxygen HTML) - build it via Doxygen (build instructions are below).
- Or do not generate it and read the comments in code and .doxygen files in
  the `docs` folder.

## Building

*(Default)* If using this plugin's build method:

1) Create a `build` directory in the `Stacklook` folder and go into it.
2) Start CMake and use the provided `CMakeLists.txt` in the `Stacklook` directory (i.e. `cmake ..`).
    - If using **unmodified KernelShark**, specify so via `-D_UNMODIFIED_KSHARK=1` to build a binary without unnecessary code.
    - If you wish to disable **visualization of prev_states** after a sched/sched_switch event up until the next sched/sched_waking event via nap rectangles in the plot, use `-D_NO_NAPS=1`.
        * *Nap* is just a quick name for the time between a sched_switch and sched_waking, when the process doesn't do anything noteworthy.
        * These need the *trace-cmd* header files and libraries to work, so:
            - If **trace-cmd header files** aren't in `/usr/include`, 
              specify so via `-D_TRACECMD_INCLUDE_DIR=[PATH]`, where
              `[PATH]` is replaced by the path to the header files.
            - If **trace-cmd shared libraries** aren't in `/usr/lib64`,
              specify so via `-D_TRACECMD_LIBS_DIR=[PATH]`, where
              `[PATH]` is replaced by the path to the shared libraries.
    - By default, the **build type** will be `RelWithDebInfo` - to change this, e.g. to `Release`, use the option `-DCMAKE_BUILD_TYPE=Release`.
    - If **Qt6 files** aren't in `/usr/include/qt6`, use the option `-D_QT6_INCLUDE_DIR=[PATH]`, where `[PATH]` is replaced by the path to the Qt6 files.
        - Build instructions still expect that the specified directory has same inner structure as the default case (i.e. it contains `QtCore`, `QtWidgets`, etc.).
    - If **KernelShark source files** aren't in the relative path `../kernelshark/src` from inside `Stacklook`, use the option `-D_KS_INCLUDE_DIR=[PATH]`, where `[PATH]` is replaced by the path to KernelShark source files.
    - If **KernelShark's shared libraries** (.so files) aren't in `/usr/local/lib64`, use the option `-D_KS_SHARED_LIBS_DIR=[PATH]`, where `[PATH]` is replaced by the path to KernelShark shared libraries.
    - If **documentation** is wanted, use the option `-D_DOXYGEN_DOC=1`.
3) Run `make` in the `build` directory.
    - If only a part of building is necessary, select a target of your choice.
    - Just running `make` builds: **documentation** (target `docs`), **the plugin** (target `stacklook`), **symlink** to the plugin SO (target `stacklook_symlink`).
4) Plug in the plugin into KernelShark (after starting it or in CLI).

If using KernelShark build method:

1) Ensure all source files (`.c`, `.cpp`, `.h`) of Stacklook are in the `src/plugins` subdirectory of your KernelShark project directory.
2) Ensure the `CMakeLists.txt` file in said subdirectory contains instructions for building the plugin (copy the style of other Qt-using GUI plugins).
    - You may need to modify them a bit further regarding options `_NO_NAPS` and `_UNMODIFIED_KSHARK`.
3) Build KernelShark (plugins are built automatically).
4) Start KernelShark (plugins built this way will be loaded automatically).
- Documentation has to be built manually

Do note that the instructions won't remove previous versions. For that, just use `rm` in the directory
where the SO files are to clean what you need.

## Contributions & acknowledgments

**Yordan Karadzhov** - maintains KernelShark, author (me) took inspiration from his examples and plugins
**Google Lens** - extracted text from images when no sources were available
**Geeksforgeeks** - [Check if a point is inside a triangle](https://www.geeksforgeeks.org/check-whether-a-given-point-lies-inside-a-triangle-or-not/)
**Markdown PDF** - VS Code extension, allowed me to export the manual with pictures easily
**ChatGPT**:
- perfect for finding ideas that will never work
- good for simple prototype brainstorming
- helped with configuration dialog when designing the layouts

## Support

Use the e-mail `djsebofficial@gmail.com` for questions.

## License

[MIT License](./LICENSE)
