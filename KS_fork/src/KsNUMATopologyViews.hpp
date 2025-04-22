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

// Enum classes

/// @brief Differentiable view types, mainly used by the radio buttons per each stream.
// (Possible EXTENSION)
enum class TopoViewType { DEFAULT = 0, NUMATREE };

// Usings

/// @brief Simpler name to package a view type and chosen topology file.
using ViewTopologyPair = std::pair<TopoViewType, QString>;

/// @brief Mapping of PU logical IDs to their OS IDs.
using TopoPUIds = std::map<int, int>;
/// @brief Mapping of core logical IDs to their owned PUs.
using TopoCorePU = std::map<int, TopoPUIds>;
/// @brief Mapping of node logical IDs to their owned cores.
using TopoNodeCorePU = std::map<int, TopoCorePU>;

// Classes

/**
 * @brief Configuration of NUMA Topology Views for a stream. Owns
 * a view type to use in KernelShark's main window and a path to the
 * topology file describing a topology as determined by hwloc.
 */
class StreamNUMATopologyConfig {
private:
    /// @brief Type of the view to be used in KernelShark's main window.
    TopoViewType _applied_view;
    /// @brief Path to the topology file describing a topology as determined by hwloc.
    // Currently only XML files are supported.
    std::string _topo_fpath;
    /// @brief Mapping of node logical IDs to their owned cores and PUs.
    /// Instead of holding a pointer to the full hwloc topology,
    /// the maps are used to store the topology in a more compact way.
    TopoNodeCorePU _brief_topo;
public: // Creation, destruction, assigns
    explicit StreamNUMATopologyConfig(TopoViewType view, const std::string& fpath);
    StreamNUMATopologyConfig();
    StreamNUMATopologyConfig(const StreamNUMATopologyConfig&);
    StreamNUMATopologyConfig& operator=(const StreamNUMATopologyConfig&);
    StreamNUMATopologyConfig(StreamNUMATopologyConfig&&) noexcept;
    StreamNUMATopologyConfig& operator=(StreamNUMATopologyConfig&&) noexcept;
    ~StreamNUMATopologyConfig() = default;
public: // Business
    const std::string& get_topo_fpath() const;

    TopoViewType get_view_type() const;
    
    void set_view_type(TopoViewType new_view);
    
    const TopoNodeCorePU& get_brief_topo() const;
    
    QVector<int> rearrangeCPUs(const QVector<int>& cpu_ids) const;
    
    QVector<int> rearrangeCPUsWithBriefTopo(const QVector<int>& cpu_ids,
        const TopoNodeCorePU& brief_topo) const;
    
    int get_topo_npus() const;
};

/**
 * @brief Class that manages the NUMA Topology Views
 * configuration for all streams. Through it, topology configurations
 * can be added, updated, observed, deleted and cleared (delete all).
 */
class KsNUMATVContext {
private:
    /**
     * @brief Unordered map of stream IDs to their topology configurations.
     * The stream ID is the key, the value is a StreamNUMATopologyConfig object.
     */
    std::unordered_map<int, StreamNUMATopologyConfig> _active_numatvs;
public:
    bool exists_for(int stream_id) const;

    int add_config(int stream_id, TopoViewType view, const std::string& topology_file);
    
    int update_cfg(int stream_id, TopoViewType view, const std::string& topology_file);
    
    const StreamNUMATopologyConfig* observe_cfg(int stream_id) const;
    
    int delete_cfg(int stream_id);
    
    void clear();
};

// Global functions

int numatv_count_PUs(const TopoNodeCorePU& brief_topo);

int numatv_count_cores(const TopoNodeCorePU& brief_topo);

TopoNodeCorePU numatv_filter_by_PUs(const TopoNodeCorePU& brief_topo,
    const QVector<int>& PUs);

bool numatv_stream_wants_topology_widget(int stream_id,
    const KsNUMATVContext& numatv_ctx);

#endif // _KS_NUMA_TV_HPP
// END of change
