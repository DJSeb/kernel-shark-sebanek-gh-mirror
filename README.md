# Repository overview

## Repository owner

David Jaromír Šebánek (contact e-mail: `djsebofficial@gmail.com`)

## Supervisor

Jan Kára RNDr.

## About

This is the main repository for the semester project & bachelor thesis for Faculty of Mathematics and Physics of
Charles University. It is, in general, concerned with improving existing software **KernelShark** with features
explained below.

It is composed of four main directories, each representing a portion of the thesis project.

- **Stacklook** - plugin for visualising stack traces via a more GUI-centric way
- **Naps** - plugin for visualising timeslices between a task's switch and next waking
- **NoBoxes** - plugin which disables some plugin's ability to partake in drawing taskboxes
  - This plugin doesn't work perfectly, as it seems there's a lot of seemingly random pop-in upon GL Widget updates,
    as the bins are a little unruly when it comes to being consistent with their visibilites.
    But unless a pretty big guttting of KernelShark's visualisation is done, this is currently possibly the best approach.
  - This was a requested bug fix, but is presented as a plugin for easy on/off changes - flickering may be too much for some.
    It is not a necessity for the project, hence it's presented as "best effort" solution.
- **KS_fork** - modified copy of KernelShark's source code with multiple additions, what many parts across different
  documentations will call "custom KernelShark"
  - _Couplebreak_ functionality is a new ability of KernelShark to split some events (chosen in code) into two
  - _NUMA Topology Views_ functionality gives KernelShark the ability to show CPU plots with respect to the NUMA
    topology given by the user to the program
  - _Other smaller additions to KernelShark's abilities_
- **SurveyPaper** - directory containing the survey paper/bachelor thesis paper **in Czech** about the plugins & enhancements
  above, along with `ExampleData` directory in which files for showcasing above's abilities may be found (namely four topology
  XML files and two trace files with DAT extension).

These five components' requirements are defined in the [project specification document](./ProjectSpecification.md). Each plugin has
their own README and documentations, **KS_fork** includes a document detailing changes made there.

See each directory for more details about that part of the project.

IMPORTANT NOTE: SurveyPaper contains the most up-to-date information, English documentations are slightly outdated,
which becomes obvious upon comparison of runnning software with some images in English docs.

## Basic general setup and usage

Potential users may either allow the script `build_all.sh` to build release builds of everything,
that being modified KernelShark from KS_fork, plugins Stacklook and Naps for both modified and
unmodified KernelShark and plugin NoBoxes (for modified KernelShark only). Other possibility is following
the list below.

1. Build modified KernelShark in KS_fork directory (CMakeLists.txt files are provided).
   Instructions are in the README in KS_fork.
2. Build plugins in their directories (CMakeLists.txt files are provided). Instructions are in user documentations
   (Czech in SurveyPaper thesis.pdf file, English in English user documentations).
   Czech documentation is more up to date, so reading the thesis file is suggested.

As for usage:

3. Launch KernelShark (from here to `KS_fork/bin`, binary name `kernelshark`).
   You can add plugins to be loaded on the terminal or later via GUI.
   Each plugin's SO or symlink to it is in `[PLUGIN_NAME]/build/bin/plugin-[PLUGIN_NAME_LOWERCASE]`.
   If the plugin was built for unmodified KernelShark, just look for `unmodif_build` subdirectory
   instead of the `build` subdirectory.
4. For example data, navigate to SurveyPaper/ExampleData.
5. Observe the modifications and plugins at work.

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

Bachelor thesis project part WIP.

(Stacklook revision, Naps plugin, Couplebreak, NUMA Topology Views.)

- [x] Brief project descrption
- [x] Specification
- [x] Basic working version\*
  - [x] Split events involving two processes
  - [x] NUMA topology visualisation
  - [x] sched_events interoperability with created plugin(s)
- [x] Final version

\* The most important things work.
