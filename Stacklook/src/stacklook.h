/** Copyright (C) 2024, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    stacklook.h
 * @brief   For plugin integration with KernelShark. Includes plugin context
 *          and a few global functions either used in C++ or in C parts of the
 *          plugin.
 * 
 * @note    Definitions in `Stacklook.cpp` and `stacklook.c`.
*/

#ifndef _KS_PLUGIN_STACKLOOK_H
#define _KS_PLUGIN_STACKLOOK_H

// C
#include <stdbool.h>

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

///
/// @brief Chosen font size for plugin's font.
#define FONT_SIZE 8

/**
 * @brief Context for the plugin, basically structured
 * globally shared data.
*/
struct plugin_stacklook_ctx {
    /**
     * @brief Numerical id of sched_switch event.
    */
    int sswitch_event_id;
    /** 
     * @brief Numerical id of kernel_stack event.
    */
    int kstack_event_id;
    /**
     * @brief Flag indicating presence of kernel_stack
     * entries in the trace. Set upon first drawing attempt,
     * as the selected events aren't fully loaded until then.
     * 
     * By default false.
     */
    bool kstacks_exist;
    /**
     * @brief Flag indicating whether the plugin has already
     * searched for kernel stacks in the trace.
     */
    bool searched_for_kstacks;
    /**
     * @brief Numerical id of sched/sched_waking or
     * couplebreak/sched_waking[target] event.
    */
    int swaking_event_id;

    /** 
     * @brief Collected switch or wakeup events.
    */
    struct kshark_data_container* collected_events;
};

// Some magic by KernelShark that makes it simpler to integrate the plugin.
KS_DECLARE_PLUGIN_CONTEXT_METHODS(struct plugin_stacklook_ctx)

// Global functions, defined in C

struct ksplot_font* get_font_ptr();
struct ksplot_font* get_bold_font_ptr();

// Global functions, defined in C++

const struct kshark_entry* get_kstack_entry(
    const struct kshark_entry* kstack_owner);
void draw_stacklook_objects(struct kshark_cpp_argv* argv_c, int sd,
                            int val, int draw_action);
void* plugin_set_gui_ptr(void* gui_ptr);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _KS_PLUGIN_STACKLOOK_H