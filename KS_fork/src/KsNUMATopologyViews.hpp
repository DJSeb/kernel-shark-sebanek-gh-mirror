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
#include <unordered_map>
#include <memory>

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

// Classes

/// @brief
struct StreamTopologyConfig {
public:
    ViewType applied_view;
    std::string topo_fpath;
    hwloc_topology_t topology;
public:
    StreamTopologyConfig();
    StreamTopologyConfig(ViewType view, const std::string& fpath);
    StreamTopologyConfig(const StreamTopologyConfig&);
    StreamTopologyConfig& operator=(const StreamTopologyConfig&);
    StreamTopologyConfig(StreamTopologyConfig&&);
    StreamTopologyConfig& operator=(StreamTopologyConfig&&);
    ~StreamTopologyConfig();
};

class NUMATVContext {
public:
    static NUMATVContext& get_instance();
private:
    using ActiveNUMATVs_t = std::unordered_map<int, StreamTopologyConfig>;
    ActiveNUMATVs_t _active_numatvs;
public:
    void add_config(int stream_id, ViewType view, const QString& topology_file);
private:
    NUMATVContext();
    NUMATVContext(const NUMATVContext&) = delete;
    NUMATVContext& operator=(const NUMATVContext&) = delete;
    ~NUMATVContext() = default;
};

#endif
// END of change