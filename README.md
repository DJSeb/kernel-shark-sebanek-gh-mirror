# Stacklook
*Plugin for visualizing stacktraces in KernelShark.*

## Author

David Jaromír Šebánek

## Supervisor

Jan Kára RNDr.

## Repository layout

You are in `root`.

- `root`
    - `Stacklook` <= All files related to the plugin
        - `src` <= All source code of the plugin
        - `doc` <= Doxygen configuration and documentation files
        - Other files include: `CMakeLists.txt`
    - `KS_fork` <= Link to a GitHub repository hosting a GitHub fork of the main KernelShark repository


## Project status

Developing Stacklook plugin's basics.

### NPRG045 general tasks

- [x] Brief project description
- [x] Specification
- [ ] Basic working version
- [ ] Final version

### Roadmap

- [x] KernelShark Record adjustment for stacktracing
- [ ] Stacklook
    * [ ] Get scheduler event for when the task was about to go to sleep and the stacktracing event
    * [ ] Add clickable shapes above each sleep event
        * [ ] *(Not in specification) Add such clickable shapes into entry viewer*
    * [ ] Show a dialogue/window with detailed stack information
    * [ ] Display preview of the stack in the top info bar
    * [x] Create CMake build instructions
        * NOTE: Could be circumvented if the plugin files were put into KernelShark's `src/plugins` directory
    * [ ] Create plugin documentation (user & technical)
        * NOTE: Limit thyself, author
        * [ ] Create build instructions for the documentation
        * [ ] Technical documentation (Doxygen)
        * [ ] User documentation (Markdown or HTML)
    * [ ] Create an example `trace.dat` file for demonstration
        * [ ] Write a program that will do some stack shenanigans 

## Requirements

KernelShark v2.3.1 (currently not tested outside of this version).

*Build file requires this version implicitly.*

## Installation

If using plugin's build method:

1) Create a build directory and go into it.
2) Start CMake and use the provided `CMakeLists.txt` in the `Stacklook` directory.
3) Run `make` in the build directory.
4) Plug in the plugin into KernelShark (after starting it or in CLI).

If using KernelShark build method:

1) Ensure all source files (`.c`, `.cpp`, `.h`) of Stacklook are in the `src/plugins` subdirectory of your KernelShark project directory.
2) Ensure the `CMakeLists.txt` file in said subdirectory contains instructions for building the plugin (copy the style of other Qt-using GUI plugins).
3) Build KernelShark (plugins are built automatically).
4) Start KernelShark (plugins built this way will be loaded automatically).


## Usage

**TODO**

## Documentation

[Look here.](./Stacklook/doc)

## Examples of use

**TODO**

## Contributions & acknowledgments

Yordan Karadzhov - maintains KernelShark, took inspiration from his examples and plugins
Google Lens - extracted text from images when no sources were available

## Support

Use the e-mail `djsebofficial@gmail.com` for questions.

## License

[MIT License](./LICENSE)
