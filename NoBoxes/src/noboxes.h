/** Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> **/

/**
 * @file    noboxes.h
 * @brief   Plugin for KernelShark to disable drawing of taskboxes from
 *          certain events' bins or ending at certain bins.
 * 
 * @note    Definitions in `noboxes.c`.
*/

#ifndef _KS_PLUGIN_NOBOXES_H
#define _KS_PLUGIN_NOBOXES_H

// KernelShark
#include "libkshark.h"
#include "libkshark-plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Context for the plugin, basically structured
 * globally shared data. It's inclusion here is mostly
 * required by the API, since all we need is the id of a
 * kernel stack event.
 */
struct plugin_noboxes_context {
    /// @brief Event ID of the kernel stack event.
    int kstack_event_id;
};

// Macro'd declarations by KernelShark which makes it simpler to integrate the plugin.
KS_DECLARE_PLUGIN_CONTEXT_METHODS(struct plugin_noboxes_context)

#ifdef __cplusplus
}
#endif

#endif // _KS_PLUGIN_NOBOXES_H