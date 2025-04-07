//NOTE: Changed here. (NUMA TV) (2025-04-04)

/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    KsNUMATopologyViews.hpp
 *  @brief   Interface for working with NUMA topologies.
*/

#ifndef _KS_NUMA_TV_HPP
#define _KS_NUMA_TV_HPP

// C++
#include <vector>

// Qt
#include <QtWidgets>

// hwloc
#include "hwloc.h"

// Enum classes

/// @brief Differentiable view types, mainly used by the radio buttons per each stream.
enum class ViewType { DEFAULT = 0, TREE };

// Usings

/// @brief Simpler name to package a view type and chosen topology file.
using ViewTopologyPair = std::pair<ViewType, QString>;

/// @brief 
class StreamTopologyConfig {
private:
    ViewType _applied_view;
    std::string _topology_fpath;
    hwloc_topology_t _topology;
};

#endif
// END of change