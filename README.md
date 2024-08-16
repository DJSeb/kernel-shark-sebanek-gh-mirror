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
    - Submodule `KS_fork` <= Link to a GitHub repository hosting a GitHub fork of the main KernelShark repository
    - File `CHANGELOG.md` <= Brief change summaries to the program, starting from the first version that basically worked
    - File `ProjectSpecification.md` <= Outlines how the project should look like
    - Other files include: `LICENSE`, `.gitignore`, `.gitmodules`, `README.md` *(you are reading this \:D)*.


## Project status

Making user documentation.

### NPRG045 general tasks

- [x] Brief project description
- [x] Specification
- [x] Basic working version*
- [ ] Final version

\* The most important things work.

### Changelog
[Here.](./CHANGELOG.md)

### Roadmap

Things not in specification aren't mandatory, but would be pretty useful.

- [x] KernelShark Record adjustment for stacktracing
- [ ] Stacklook
    * [x] Get scheduler event for when the task was about to go to sleep and the stacktracing event
    * [x] Add clickable shapes above each sleep event
        * [ABANDONED] *(Not in specification) Add such clickable shapes into entry viewer*
            * NOTE: It is highly unlikely that this is easily doable as a plugin, unless a lot of KernelShark is rewritten.
    * [x] Show a dialogue/window with detailed stack information
    * [x] Display preview of the stack in the top info bar
    * [x] Create CMake build instructions
        * NOTE: Could be circumvented if the plugin files were put into KernelShark's `src/plugins` directory
    * [ ] Create plugin documentation (user & technical)
        <!-- NOTE: Limit thyself, author -->
        * [x] Create build instructions for the documentation
        * [x] Technical documentation (Doxygen)
        * [ ] User documentation (Markdown or HTML)
    * [x] *(Not in specification) Add a settings menu for the plugin*
    * [x] *(implied)* Make it work for CPU and task plots
* [ ] Create an example `trace.dat` file for demonstration
    * [ ] Write a program that will do some stack shenanigans

## Requirements

Modified KernelShark (preview functionality is unusable otherwise).

It is possible to create a plugin that would work with unomdified
KernelShark version 2.3.1 (code that would need to be removed
contains a comment about this).

## Documentation

[User manual](./Stacklook/doc/user/Manual.md)

Technical (Doxygen HTML) - build it via Doxygen (build instructions are below).
- Or do not generate it and read the comments in code and .doxygen files in
  the `docs` folder.

## Building

*(Default)* If using this plugin's build method:

1) Create a `build` directory in the `Stacklook` folder and go into it.
2) Start CMake and use the provided `CMakeLists.txt` in the `Stacklook` directory (i.e. `cmake ..`).
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
3) Build KernelShark (plugins are built automatically).
4) Start KernelShark (plugins built this way will be loaded automatically).
- Documentation has to be built manually

Do note that the instructions won't remove previous versions. For that, just use `rm` in the directory
where the SO files are to clean what you need.

## Contributions & acknowledgments

**Yordan Karadzhov** - maintains KernelShark, author (me) took inspiration from his examples and plugins
**Google Lens** - extracted text from images when no sources were available
**Geeksforgeeks** - [Check if a point is inside a triangle](https://www.geeksforgeeks.org/check-whether-a-given-point-lies-inside-a-triangle-or-not/)

**ChatGPT**:
- perfect for finding ideas that will never work
- good for simple prototype brainstorming
- helped with configuration dialog when designing the layouts

## Support

Use the e-mail `djsebofficial@gmail.com` for questions.

## License

[MIT License](./LICENSE)
