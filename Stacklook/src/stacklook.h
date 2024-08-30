/** TODO: Copyright? **/

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
     * @brief Numerical id of sched_wakeup event.
    */
    int swake_event_id;

    /** 
     * @brief Collected switch or wakeup events.
    */
    struct kshark_data_container* collected_events;

    /**
     * @brief Container of Stacklook window pointers.
    */
    void* cpp_views_container;
};

// Some magic by KernelShark that makes it simpler to integrate the plugin.
KS_DECLARE_PLUGIN_CONTEXT_METHODS(struct plugin_stacklook_ctx)

// Global functions, defined in C

struct ksplot_font* get_font_ptr();
struct ksplot_font* get_bold_font_ptr();

// Global functions, defined in C++

void draw_stacklook_objects(struct kshark_cpp_argv* argv_c, int sd,
                            int val, int draw_action);
void* plugin_set_gui_ptr(void* gui_ptr);
void clean_opened_views(void* view_container);
void* init_views();
#ifndef _NO_NAPS
void wakeup_evt_tep_processing(struct plugin_stacklook_ctx* ctx, 
                               struct kshark_data_stream* stream,
                               void* rec,
                               struct kshark_entry* entry);
#endif

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // _KS_PLUGIN_STACKLOOK_H