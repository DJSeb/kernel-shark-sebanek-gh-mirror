# Repository overview

## Mirrors

This repository has two mirrors in case the primary repository is unavailable. Development expects that one of
the mirrors will become the primary repository in the future.

_GitLab mirror_: https://gitlab.mff.cuni.cz/sebaned/kernel-shark-sebanek-mirror

_GitHub mirror_: https://github.com/DJSeb/kernel-shark-sebanek-gh-mirror

## Repository owner

David Jaromír Šebánek (contact e-mail: `djsebofficial@gmail.com`)

## Supervisor

Jan Kára RNDr.

## About

This is the main repository for the semester project & bachelor thesis for Faculty of Mathematics and Physics of
Charles University. It is, in general, concerned with improving existing software **KernelShark** with features
explained below.

It is composed of five main directories, each representing a portion of the thesis project.

- **Stacklook** - plugin for visualising stack traces via a more GUI-centric way
- **Naps** - plugin for visualising timeslices between a task's switch and next waking
- **NoBoxes** - plugin which disables some plugin's ability to partake in drawing taskboxes
  - This plugin doesn't work perfectly, as it seems there's a lot of seemingly random pop-in upon GL Widget updates,
    as the bins are a little unruly when it comes to being consistent with their visibilites.
    But unless a pretty big guttting of KernelShark's visualisation is done, this is currently possibly the best approach.
  - This was a requested bug fix, but is presented as a plugin for easy on/off changes - flickering may be too much for some.
    It is not a necessity for the project, hence it's presented as "best effort" solution.
- **KS_fork** - modified copy of KernelShark's source code with multiple additions, what many parts across different
  documentations will call "custom/modified KernelShark"
  - _Couplebreak_ functionality is a new ability of KernelShark to split some events (chosen in code) into two
  - _NUMA Topology Views_ functionality gives KernelShark the ability to show CPU plots with respect to the NUMA
    topology given by the user to the program
  - _Other smaller additions to KernelShark's abilities_
- **SurveyPaper** - directory containing the survey paper/bachelor thesis paper **in Czech** about the plugins & enhancements
  above, along with **ExampleData** directory in which files for showcasing above's abilities may be found (namely four topology
  XML files and three trace files with DAT extension).

These five components' requirements are somewhat defined in the [project specification document](./ProjectSpecification.md). Each plugin has
their own README and documentations, **KS_fork** includes a document detailing changes made there.

See each directory for more details about that part of the project.

### (IMPORTANT) Documentations

SurveyPaper contains the most up-to-date documentation, albeit in Czech only, English documentations are slightly outdated,
which becomes obvious upon comparison of runnning software with some images in English documents. English documentation also
has a slightly different format, is more compact and its design sections badly need a revision. It is strongly recommended
to read the paper first or read it while working with the software (assuming you can understand Czech or have the means
to translate it).

English documentation WILL be updated in the future as the project will be approaching a state when it can be submitted as a
contribution to KernelShark.

## Basic general setup and usage

To use anything, you have to first `git clone` this repository. Second, make sure you have all dependencies listed
in [KernelShark's README](./KS_fork/README), including the ones introduced by modifications (i.e. Qt of version at
least 6.7 and Hwloc of version at least 2.11).

Potential users may either run the script `build_all.sh` to build release builds of everything,
that being modified KernelShark from KS_fork, plugins Stacklook and Naps for both modified and
unmodified KernelShark and plugin NoBoxes (for modified KernelShark only). Other possibility is following
the list below.

1. Build modified KernelShark in KS_fork directory (CMakeLists.txt files are provided).
   Instructions are in the README in KS_fork.
2. Build plugins in their directories (CMakeLists.txt files are provided). Instructions are in user documentations
   (Czech in SurveyPaper/thesis.pdf file, English in English user documentations, which are either in
   KS_fork/Modifications or in plugins' directories).
   Czech documentation is more up to date, so reading the thesis file is suggested.

As for usage:

3. Launch KernelShark (from here to `KS_fork/bin`, binary name `kernelshark`).
   You can add plugins to be loaded on the terminal or later via GUI.
   Each plugin's SO or symlink to it is at `[PLUGIN_NAME]/build/bin/plugin-[PLUGIN_NAME_LOWERCASE].so`.
   If the plugin was built for unmodified KernelShark, just look for `unmodif_build` subdirectory
   instead of the `build` subdirectory.
4. For example data, navigate to SurveyPaper/ExampleData.
5. Observe the modifications and plugins at work.

If unmodified KernelShark is desired, it is recommended to install a version from its official repository on the internet.

## Projects' status

### Semester project (NPRG045) general tasks

Semester project part is finished.

(Stacklook plugin and small changes to KernelShark.)

- [x] Brief project description
- [x] Specification
- [x] Basic working version\*
- [x] Final version

\* The most important things work.

### Bachelor thesis project general tasks

Bachelor thesis project part.

(Stacklook revision, Naps plugin, Couplebreak, NUMA Topology Views.)

- [x] Brief project descrption
- [x] Specification
- [x] Basic working version\*
  - [x] Split events involving two processes (couplebreak)
  - [x] NUMA topology visualisation
  - [x] sched_events interoperability with created plugin(s) (couplebreak)
- [x] Final version

\* The most important things work.
