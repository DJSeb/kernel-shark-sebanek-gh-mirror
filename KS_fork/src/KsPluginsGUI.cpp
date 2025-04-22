// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright 2022 VMware Inc, Yordan Karadzhov (VMware) <y.karadz@gmail.com>
 */

/**
  *  @file    KsPluginsGUI.cpp
  *  @brief   KernelShark C++ plugin declarations.
  */

// KernelShark
#include "KsPluginsGUI.hpp"
#include "KsMainWindow.hpp"
#include "KsDualMarker.hpp"

/**
 * @brief Marks an entry with the A marker.
 * 
 * @param ks_ptr Pointer to the KernelShark main window.
 * @param e Pointer to the entry to be marked.
 */
void markEntryA(void *ks_ptr, const kshark_entry *e)
{
	KsMainWindow *ks = static_cast<KsMainWindow *>(ks_ptr);
	ks->markEntry(e, DualMarkerState::A);
}

/**
 * @brief Marks an entry with the B marker.
 * 
 * @param ks_ptr Pointer to the KernelShark main window.
 * @param e Pointer to the entry to be marked.
 */
void markEntryB(void *ks_ptr, const kshark_entry *e)
{
	KsMainWindow *ks = static_cast<KsMainWindow *>(ks_ptr);
	ks->markEntry(e, DualMarkerState::B);
}
