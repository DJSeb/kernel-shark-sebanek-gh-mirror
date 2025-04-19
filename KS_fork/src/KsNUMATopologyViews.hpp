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
#include <map>

// Qt
#include <QtWidgets>
#include <QVector>

// hwloc
#include "hwloc.h"

// Enum classes

/// @brief Differentiable view types, mainly used by the radio buttons per each stream.
enum class ViewType { DEFAULT = 0, NUMATREE };

// Usings

/// @brief Simpler name to package a view type and chosen topology file.
using ViewTopologyPair = std::pair<ViewType, QString>;

using PUIds = std::map<int, int>;
using CorePU = std::map<int, PUIds>;
using NodeCorePU = std::map<int, CorePU>;

// Classes

/// @brief
struct StreamTopologyConfig {
public:
    ViewType applied_view;
    std::string topo_fpath;
    hwloc_topology_t topology;
public: // Creation, destruction, assigns
    StreamTopologyConfig();
    StreamTopologyConfig(ViewType view, const std::string& fpath);
    StreamTopologyConfig(const StreamTopologyConfig&);
    StreamTopologyConfig& operator=(const StreamTopologyConfig&);
    StreamTopologyConfig(StreamTopologyConfig&&) noexcept;
    StreamTopologyConfig& operator=(StreamTopologyConfig&&) noexcept;
    ~StreamTopologyConfig();
public: // Business
    const std::string& get_topo_fpath() const;
    ViewType get_view_type() const;
    const NodeCorePU get_brief_topo() const;
    QVector<int> rearrangeCPUs(const QVector<int>& cpu_ids) const;
    QVector<int> rearrangeCPUsWithBriefTopo
    (const QVector<int>& cpu_ids, const NodeCorePU& brief_topo) const;
};

class NUMATVContext {
public:
    static NUMATVContext& get_instance();
private:
    using ActiveNUMATVs_t = std::unordered_map<int, StreamTopologyConfig>;
    ActiveNUMATVs_t _active_numatvs;
public:
    bool exists_for(int stream_id) const;
    int add_config(int stream_id, ViewType view, const std::string& topology_file);
    int update_cfg(int stream_id, ViewType view, const std::string& topology_file);
    const StreamTopologyConfig* observe_cfg(int stream_id) const;
    int delete_cfg(int stream_id);
    void clear();
private:
    NUMATVContext();
    NUMATVContext(const NUMATVContext&) = delete;
    NUMATVContext& operator=(const NUMATVContext&) = delete;
    NUMATVContext(NUMATVContext&&) = delete;
    NUMATVContext& operator=(NUMATVContext&&) = delete;
    ~NUMATVContext() = default;
};

// Global functions
int numatv_count_PUs(const NodeCorePU& brief_topo);
int numatv_count_cores(const NodeCorePU& brief_topo);
NodeCorePU numatv_filter_by_PUs(const NodeCorePU& brief_topo, QVector<int> PUs);

#endif // _KS_NUMA_TV_HPP
// END of change