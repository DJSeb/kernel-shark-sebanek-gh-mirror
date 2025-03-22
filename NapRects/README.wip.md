 (after line 178)
 
 - If you wish to disable **visualization of prev_states** after a sched/sched_switch event up until the next sched/sched_waking event via nap rectangles in the plot, use `-D_NO_NAPS=1`.
     - _Nap_ is just a quick name for the time between a sched_switch and sched_waking, when the process doesn't do anything noteworthy.
     - These need the _trace-cmd_ header files and libraries to work, so:
       - If **trace-cmd header files** aren't in `/usr/include`,
         specify so via `-D_TRACECMD_INCLUDE_DIR=[PATH]`, where
         `[PATH]` is replaced by the path to the header files.
       - If **trace-cmd shared libraries** aren't in `/usr/lib64`,
         specify so via `-D_TRACECMD_LIBS_DIR=[PATH]`, where
         `[PATH]` is replaced by the path to the shared libraries.

If using KernelShark build method:

1. Ensure all source files (`.c`, `.cpp`, `.h`) of Naps are in the `src/plugins` subdirectory of your KernelShark project directory.
2. Ensure the `CMakeLists.txt` file in said subdirectory contains instructions for building the plugin (copy the style of other Qt-using GUI plugins).
3. Build KernelShark (plugins are built automatically).
4. Start KernelShark. Plugins built this way will be loaded automatically. If that for some reason failed, look for the SO
as for any other default-built KernelShark plugin.

- Documentation has to be built manually