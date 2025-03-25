# TODO: !!!CHANGE THIS, MOST OF THIS SHOULD BE MOVED, SOME OF THIS CAN STAY!!!

<!-- TODO: Move stuff below -->

<!--# Stacklook

_Plugin for visualizing stacktraces in KernelShark._
-->

### Author

David Jaromír Šebánek

### Supervisor

Jan Kára RNDr.

<!--TODO: Move paragraph below. -->

<!--
## (WIP) Repository layout

You are in **root**.

- **root**
  - Directory `Stacklook` <= All files related to the plugin
    - Directory `src` <= All source code of the plugin
    - Directory `doc` <= Doxygen configuration and documentation files
    - Other files include: `CMakeLists.txt`
  - Directory `KS_fork` <= Modified copy of the main KernelShark repository with changes
  - File `CHANGELOG.md` <= Brief change summaries to the program, starting from the first version that basically worked
  - File `ProjectSpecification.md` <= Outlines how the project should look like
  - Other files include: `LICENSE`, `.gitignore`, `.gitmodules`, `README.md` _(you are reading this \:D)_.
-->

## Project status

<!--TODO: Move line below -->

<!--At version **1.4.3**.-->

Semester project part is finished.

Bachelor thesis project part WIP.

### NPRG045 general tasks

(Stacklook plugin and small changes to KernelShark.)

- [x] Brief project description
- [x] Specification
- [x] Basic working version\*
- [x] Final version

\* The most important things work.

### Bachelor thesis project general tasks

- [x] Brief project descrption
- [x] Specification
- [ ] Basic working version
  - [x] Split events involving two processes
  - [ ] NUMA topology visualisation
  - [ ] sched_events interoperability with created plugin(s)
- [ ] Final version

\* The most important things work.

<!--TODO: Move stuff below. -->

<!--### Changelog

[Here.](./CHANGELOG.md)
-->

<!--TODO: Revise this.-->

### Roadmap

Things not in specification aren't mandatory, but would be pretty useful.

- [x] KernelShark Record adjustment for stacktracing
- [x] Stacklook
  - [x] Get scheduler event for when the task was about to go to sleep and the stacktracing event
  - [x] Add clickable shapes above each sleep event
    - \[ABANDONED\] _(Not in specification) Add such clickable shapes into entry viewer_
      - NOTE: It is highly unlikely that this is easily doable as a plugin, unless a lot of KernelShark is rewritten.
  - [x] Show a dialogue/window with detailed stack information
  - [x] Display preview of the stack in the top info bar
  - [x] Create CMake build instructions
    - NOTE: Could be circumvented if the plugin files were put into KernelShark's `src/plugins` directory
  - [x] Create plugin documentation (user & technical)
    - [x] Create build instructions for the documentation
    - [x] Technical documentation (Doxygen)
    - [x] User documentation (Markdown or HTML)
  - [x] _(Not in specification) Add a settings menu for the plugin_
  - [x] _(Not in specification, but requested)_ Visualization of prev_states after a sched/sched_switch
  - [x] _(implied)_ Make it work for CPU and task plots

* [x] Create an example `trace.dat` file for demonstration
  - [x] Write a program that will do some stack shenanigans (`recursive_syscall_with_sleep`)
  - _NOTE:_ It isn't great, but it does show nanosceond sleep :\)
  - Will also include a very small trace-file to show some simple stuff (`simple_trace.dat`)

Below are bachelor thesis part's appended requirements:

- [x] Events involving two processes shall be split into initiators and targets
  - E.g. sched_waking should be split into "awaker" and "awakened" events, one
    belonging to each respective process
  - [x] At the very least sched_switch and sched_waking should be splitted.
  - \[ABANDONED\] Try to keep the logic inside the plugin (minimizing KernelShark changes)
    - NOTE: Not doable, insufficeint API, KernelShark source code changes necessary.
- [ ] NUMA topology visualization in KernelShark (either as a plugin or KernelShark modification)
  - [ ] Parse data from Istopo (XML format)
  - [ ] Visualize said data on the screen
    - [ ] If no Istopo data are given, use default KShark visualization
    - [ ] CPU reordering according to topology (NUMA nodes, hyperthread siblings)
    - [ ] Tree grouping of CPUs
      - [ ] May be collapsible (preferred)
      - NOTE: Other grouping visualizations are possible, if
        CPUs are sorted by topology, but tree-like is the one
        that will be implemented as basis
    - [ ] Colorful differentiation of different groups
  - [ ] Configurable display method via some option in KShark's menu
- [x] Allow `sched_events` plugin compatibility
  - NOTE: XOR choice
  - \[ABANDONED\] Either integrate Stacklook into sched_events plugin
    - Would violate the holy Single Responsibility principle
    - Direct approach
    - Ensured success
    - NOTE: Statement above is false.
  - [x] Or figure out how to allow compatibility while keeping plugins separate
    - Less direct
    - Keeps SRP
    - Unknown probability of success
    - Ultimately chosen due to KernelShark's insufficient API for creating new entries.

General SW goal:

- [ ] Debug the plugin even more, stabilise performance where necessary
- [ ] Revise README and other non-code parts of repository to reflect project extensions
- [ ] _("Optional")_ Create proper design document \* Would be quite good to include in technical documentation


Non-software goal - write survey paper:

- [ ] Survey paper
  - [ ] Stack tracing (in Linux)
    - Mostly basics, some more info about KernelShark's stack tracer
    - Mainly to set context for the project
  - [ ] Basic visualizations of trace data
    - Not eclusive to stack tracing, but mostly about it
  - [ ] Theoretical part of KernelShark
    - [ ] Basic overview of design
    - [ ] How is it visualizing the data?
    - [ ] What are the limitations?
      - I.e. why make the project in the first place?
    - [ ] What is the job of plugins? / Removing limitations
  - [ ] Theoretical part of Stacklook & KShark modifications
    - [ ] Brief overview of plugin's design
      - Shouldn't be too long, that's what the documentations are for
      - Mainly to get a rough idea if only reading the paper
    - [ ] Purpose & limits
    - [ ] Results of implementation

<!--TODO: Revise and move below-->
<!--
## (WIP) Requirements

Modified KernelShark (preview functionality is unusable otherwise).

It is possible to create a plugin that would work with unomdified
KernelShark version 2.3.1 (code that would need to be removed
contains a comment about this).

**Newly from 1.3.1**: By specifying CMake variable `_UNMODIFED_KSHARK`,
CMake will be able to add a definition of a preprocessor variable of the
same name and compile the plugin SO file appropriately.

## (WIP) Compatibility

The user is **REQUIRED** to turn off "sched_events" plugin, which causes issues
because of reassigning "next" fields of entries. Plugin will **NOT** work
correctly with "sched_events" enabled.

## (WIP) Documentation

[User manual](./Stacklook/doc/user/Manual.md)

Technical (Doxygen HTML) - build it via Doxygen (build instructions are below).

- Or do not generate it and read the comments in code and .doxygen files in
  the `docs` folder.

## (WIP) Building

_(Default)_ If using this plugin's build method:

1. Create a `build` directory in the `Stacklook` folder and go into it.
2. Start CMake and use the provided `CMakeLists.txt` in the `Stacklook` directory (i.e. `cmake ..`).
   - If using **unmodified KernelShark**, specify so via `-D_UNMODIFIED_KSHARK=1` to build a binary without unnecessary code.
   - By default, the **build type** will be `RelWithDebInfo` - to change this, e.g. to `Release`, use the option `-DCMAKE_BUILD_TYPE=Release`.
   - If **Qt6 files** aren't in `/usr/include/qt6`, use the option `-D_QT6_INCLUDE_DIR=[PATH]`, where `[PATH]` is replaced by the path to the Qt6 files.
     - Build instructions still expect that the specified directory has same inner structure as the default case (i.e. it contains `QtCore`, `QtWidgets`, etc.).
   - If **KernelShark source files** aren't in the relative path `../kernelshark/src` from inside `Stacklook`, use the option `-D_KS_INCLUDE_DIR=[PATH]`, where `[PATH]` is replaced by the path to KernelShark source files.
   - If **KernelShark's shared libraries** (.so files) aren't in `/usr/local/lib64`, use the option `-D_KS_SHARED_LIBS_DIR=[PATH]`, where `[PATH]` is replaced by the path to KernelShark shared libraries.
   - If **documentation** is wanted, use the option `-D_DOXYGEN_DOC=1`.
3. Run `make` in the `build` directory.
   - If only a part of building is necessary, select a target of your choice.
   - Just running `make` builds: **the plugin** (target `stacklook`), **symlink** to the plugin SO (target `stacklook_symlink`) and **documentation** (target `docs`), if it was specified.
4. Plug in the plugin into KernelShark (after starting it or in CLI).

If using KernelShark build method:

1. Ensure all source files (`.c`, `.cpp`, `.h`) of Stacklook are in the `src/plugins` subdirectory of your KernelShark project directory.
2. Ensure the `CMakeLists.txt` file in said subdirectory contains instructions for building the plugin (copy the style of other Qt-using GUI plugins).
   - You may need to modify them a bit further regarding the option `_UNMODIFIED_KSHARK`.
3. Build KernelShark (plugins are built automatically).
4. Start KernelShark (plugins built this way will be loaded automatically).

- Documentation has to be built manually

Do note that the instructions won't remove previous versions. For that, just use `rm` in the directory
where the SO files are to clean what you need.
-->

## (WIP) Contributions & acknowledgments

**Yordan Karadzhov** - maintains KernelShark, author (me) took inspiration from his examples and plugins
**Google Lens** - extracted text from images when no sources were available
**Geeksforgeeks** - [Check if a point is inside a triangle](https://www.geeksforgeeks.org/check-whether-a-given-point-lies-inside-a-triangle-or-not/)
**Markdown PDF** - VS Code extension, allowed me to export the manual with pictures easily
**ChatGPT**:

- perfect for finding ideas that will never work
- good for simple prototype brainstorming
- helped with configuration dialog when designing the layouts

**Doxygen Awesome CSS** - [beautiful CSS theme](https://jothepro.github.io/doxygen-awesome-css/index.html) that makes even Doxygen documentation look good

## Support

Use the e-mail `djsebofficial@gmail.com` for questions.

## License

[MIT License](./LICENSE)
