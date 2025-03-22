/** Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    naps.h
 * @brief   For plugin integration with KernelShark. Includes plugin context
 *          and a few global functions either used in C++ or in C parts of the
 *          plugin.
 * 
 * @note    Definitions in `NapRects.cpp` and `naps.c`.
*/

#ifndef _KS_PLUGIN_NAPS_H
#define _KS_PLUGIN_NAPS_H

// traceevent
#include <traceevent/event-parse.h>

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
struct plugin_naps_context {
    // Event IDs

    /**
     * @brief Numerical id of sched_switch event.
    */
   int sswitch_event_id;
   
   /**
    * @brief Numerical id of sched_waking event.
   */
   int waking_event_id;

    // Tep processing
    
    /**
     * @brief Page handle used to parse the trace event data.
    */
   struct tep_handle* tep;

   /**
    * @brief Pointer to the sched_waking_event object.
   */
   struct tep_event* tep_waking;
   
   /**
    * @brief Pointer to the sched_waking_pid_field format descriptor.
   */
   struct tep_format_field* sched_waking_pid_field;

    /** 
     * @brief Collected switch or wakeup events.
    */
   struct kshark_data_container* collected_events;
};

// Some magic by KernelShark that makes it simpler to integrate the plugin.
KS_DECLARE_PLUGIN_CONTEXT_METHODS(struct plugin_naps_context)

// Global functions, defined in C

struct ksplot_font* get_font_ptr();
struct ksplot_font* get_bold_font_ptr();

// Global functions, defined in C++

const std::string get_switch_prev_state(const kshark_entry* entry);
void draw_nap_rectangles(struct kshark_cpp_argv* argv_c, int sd,
    int val, int draw_action);
void* plugin_set_gui_ptr(void* gui_ptr);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // _KS_PLUGIN_NAPS_H