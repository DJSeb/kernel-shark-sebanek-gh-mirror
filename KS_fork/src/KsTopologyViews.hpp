//NOTE: Changed here. (TOPOVIEWS) (2025-04-04)

/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    KsTopologyViews.hpp
 *  @brief   Interface for working with topologies.
*/

#ifndef _KS_TOPOVIEWS_HPP
#define _KS_TOPOVIEWS_HPP

// C++
#include <vector>
#include <unordered_map>
#include <map>

// Qt
#include <QtWidgets>
#include <QVector>

// Enum classes

/// @brief Differentiable view types, mainly used by the radio buttons per each stream.
enum class TopoViewType { DEFAULT = 0, NUMATREE };

// Usings

/// @brief Simpler name to package a view type and chosen topology file.
using ViewTopologyPair = std::pair<TopoViewType, QString>;

// Classes

/**
 * @brief Default topology configuration.
 */
class TopoViewConfig {
protected:
    /// @brief Path to a file describing a topology.
    std::string _topo_fpath;
public:
    explicit TopoViewConfig(const std::string& fpath);
    TopoViewConfig() : _topo_fpath("") {};
    virtual ~TopoViewConfig() = default;

    const std::string& getTopoFilepath() const;

    int getTopologyNPUs() const;

    /**
     * @brief Runtime indicator of the type of topology view and configuration.
     * 
     * @return Type of the topology view & configuration.
     */
    virtual TopoViewType getViewType() const { return TopoViewType::DEFAULT; };
};

/**
 * @brief Configuration of Topology Views for a NUMA topology. Accepts
 * files from hwloc XML format. The topology is stored in a more compact way
 * than the full hwloc topology after construction.
 */
class HwlocNUMATopoViewConfig: public TopoViewConfig {
public:
    /// @brief Mapping of PU logical IDs to their OS IDs.
    using TopoPUIds = std::map<int, int>;
    /// @brief Mapping of core logical IDs to their owned PUs.
    using TopoCorePU = std::map<int, TopoPUIds>;
    /// @brief Mapping of node logical IDs to their owned cores.
    using TopoNodeCorePU = std::map<int, TopoCorePU>;
private:
    /// @brief Mapping of NUMA node logical IDs to their owned cores and PUs.
    /// Instead of holding a pointer to the full hwloc topology,
    /// the maps are used to store the topology in a more compact way.
    TopoNodeCorePU _brief_topo;
public: // Creation, destruction, assigns
    explicit HwlocNUMATopoViewConfig(const std::string& fpath);
    HwlocNUMATopoViewConfig();
    ~HwlocNUMATopoViewConfig() override = default;
public: // Business
    const TopoNodeCorePU& getBriefTopology() const;
    
    QVector<int> rearrangeCPUs(const QVector<int>& cpu_ids) const;
    
    QVector<int> rearrangeCPUsWithBriefTopo(const QVector<int>& cpu_ids,
        const TopoNodeCorePU& brief_topo) const;

    /**
     * @brief Runtime indicator of the type of topology view and configuration.
     * 
     * @return Type of the topology view & configuration.
     */
    TopoViewType getViewType() const override { return TopoViewType::NUMATREE; }
};

/**
 * @brief Class that manages the Topology Views
 * configuration for all streams. Through it, topology configurations
 * can be added, updated, observed, deleted and cleared (delete all).
 */
class KsTopoViewsContext {
private:
    /**
     * @brief Unordered map of stream IDs to their topology configurations.
     * The stream ID is the key, the value is a pointer to a TopoViewConfig object.
     */
    std::unordered_map<int, TopoViewConfig*> _active_topoviews;
public:
    bool existsFor(int stream_id) const;

    int addConfig(int stream_id, TopoViewType view, const std::string& topology_file);
    
    int updateConfig(int stream_id, TopoViewType view, const std::string& topology_file);
    
    const TopoViewConfig* observeConfig(int stream_id) const;
    
    int deleteConfig(int stream_id);
    
    void clear();

    ~KsTopoViewsContext();
};

// Global functions

int numatv_count_PUs(const HwlocNUMATopoViewConfig::TopoNodeCorePU& brief_topo);

int numatv_count_cores(const HwlocNUMATopoViewConfig::TopoNodeCorePU& brief_topo);

HwlocNUMATopoViewConfig::TopoNodeCorePU numatv_filter_by_PUs(
    const HwlocNUMATopoViewConfig::TopoNodeCorePU& brief_topo,
    const QVector<int>& PUs);

bool stream_wants_topology_widget(int stream_id,
    const KsTopoViewsContext& topoviews_ctx);

#endif // _KS_TOPOVIEWS_HPP
// END of change
