//NOTE: Changed here. (TOPOVIEWS) (2025-04-04)

/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    KsTopologyViews.cpp
 *  @brief   Implementations for working with topologies.
*/

// hwloc
#include "hwloc.h"

// C++
#include <string>
#include <iostream>

// Qt
#include <QVector>

// KernelShark
#include "KsTopologyViews.hpp"
#include "libkshark.h"

// Statics

/**
 * @brief Get the hwloc topology object's pointer. Topology is loaded
 * from the given XML filepath. The user is responsible for destroying
 * the topology object after use.
 * 
 * @param topo_fpath File path to the topology XML file.
 * @return Pointer to the hwloc topology object loaded from the XML file.
 * If a problem occured during allocation or loading, nullptr is returned
 * and an error message is printed to the standard error output.
 */
static hwloc_topology_t get_hwloc_topology(const std::string& topo_fpath) {
    int hwloc_load_result;
    hwloc_topology_t topology;
    hwloc_load_result = hwloc_topology_init(&topology);
    if (hwloc_load_result != 0) {
        std::cerr << "[FAIL] Not enough memory for allocation of topology..."
            << std::endl;
        topology = nullptr;
        return nullptr;
    }
    
    hwloc_load_result = hwloc_topology_set_xml(topology, topo_fpath.c_str());
    if (hwloc_load_result != 0) {
        std::cerr << "[FAIL] Couldn't set topology to be loaded by XML file..."
            << std::endl;
        hwloc_topology_destroy(topology);
        topology = nullptr;
        return nullptr;
    }

    hwloc_load_result = hwloc_topology_load(topology);
    if (hwloc_load_result != 0) {
        std::cerr << "[FAIL] Couldn't load topology of file '"+ topo_fpath +"'..."
            << std::endl;
        hwloc_topology_destroy(topology);
        topology = nullptr;
        return nullptr;
    }
    return topology;
}

// KsTopoViewsContext

/**
 * @brief Checks for existence of a topology configuration for a given stream ID.
 * 
 * @param stream_id ID of the stream to check for.
 * @return True if the topology configuration exists for the stream ID, false otherwise.
 */
bool KsTopoViewsContext::existsFor(int stream_id) const
{ return _active_topoviews.count(stream_id) > 0; }

/**
 * @brief Attempts to add a new topology configuration for a given stream ID.
 * Returns an exit code indicating success or failures.
 * 
 * @param stream_id ID of the stream to add the configuration for.
 * @param view Type of view to be used for the topology configuration.
 * @param topology_file Path to the topology file to be used for the configuration.
 * @return Exit code indicating success or failure:
 * `-2` if KernelShark context is not available,
 * `-1` if the topology file is not valid or
 * `0` if the configuration was added successfully.
 */
int KsTopoViewsContext::addConfig(int stream_id, TopoViewType view, const std::string& topology_file) {
    kshark_context *kshark_ctx(nullptr);
    int retval = -1;
    
    if (!kshark_instance(&kshark_ctx)) {
        return -2;
    }
    
    kshark_data_stream* stream = kshark_get_data_stream(kshark_ctx, stream_id);
    int stream_ncpus = stream->n_cpus;
    
    TopoViewConfig* new_topo_cfg;
    if (view == TopoViewType::NUMATREE) {
        new_topo_cfg = new HwlocNUMATopoViewConfig{topology_file};
    } else {
        new_topo_cfg = new TopoViewConfig{topology_file};
    }
    
    int topo_npus = new_topo_cfg->getTopologyNPUs();
    
    if (topo_npus == stream_ncpus) {
        _active_topoviews[stream_id] = new_topo_cfg;
        retval = 0;
    }

    return retval;
}

/**
 * @brief Attempts to update an existing topology configuration for a given stream ID.
 * If the topology file or the view type have changed, a new configuration is created.
 * Returns an exit code indicating success or failure.
 * 
 * @param stream_id ID of the stream to update the configuration for.
 * @param view Type of view to be used for the topology configuration.
 * @param topology_file Path to the topology file to be used for the configuration.
 * @return Exit code indicating success or failure:
 * `-3` if there wasn't a topology configuration to update,
 * `-2` if KernelShark context is not available,
 * `-1` if the topology file is not valid,
 * `0` if the configuration was updated successfully or
 * `1` if the configuration was not changed (success).
 */
int KsTopoViewsContext::updateConfig(int stream_id, TopoViewType view, const std::string& topology_file) {
    bool retval = -3;
    
    if (existsFor(stream_id)) {
        // Get old configuration
        TopoViewConfig* topo_cfg = _active_topoviews.at(stream_id);
        bool file_changed = topology_file != topo_cfg->getTopoFilepath();
        bool view_changed = topo_cfg->getViewType() != view;

        if (file_changed || view_changed) {
            // Something hanged - create new topology configuration
            retval = addConfig(stream_id, view, topology_file);
        } else {
            // Nothing changed, request no action to be taken
            retval = 1;
        }
    }

    // If there wasn't anything to update, returns -3.

    return retval;
}

/**
 * @brief Gets an observer pointer to the topology configuration for a given stream ID.
 * If the configuration does not exist, returns nullptr. Such should NOT be used for any
 * modifications of the topology configuration, hence the name and their constness.
 * 
 * @param stream_id ID of the stream to get the configuration for.
 * @return Observer pointer to the topology configuration or nullptr if it doesn't exist.
 */
const TopoViewConfig* KsTopoViewsContext::observeConfig(int stream_id) const {
    if (existsFor(stream_id)) {
        const TopoViewConfig* topo_cfg = _active_topoviews.at(stream_id);
        return topo_cfg;
    }
    return nullptr;
}

/**
 * @brief Deletes the topology configuration for a given stream ID.
 * 
 * @param stream_id ID of the stream to delete the configuration for.
 * @return Number of configurations deleted (0 or 1).
 */
int KsTopoViewsContext::deleteConfig(int stream_id)
{ return _active_topoviews.erase(stream_id); }

/**
 * @brief Deletes all topology configurations from the context.
 */
void KsTopoViewsContext::clear() {
    for (auto& topo_cfg : _active_topoviews) {
        delete topo_cfg.second;
    }
    _active_topoviews.clear();
}

/**
 * @brief Destructor of the topology views context. Deletes all
 * topology configurations in the context.
 */
KsTopoViewsContext::~KsTopoViewsContext() {
    for (auto& topo_cfg : _active_topoviews) {
        delete topo_cfg.second;
    }
    _active_topoviews.clear();
}

// TopoViewConfig

/**
 * @brief Constructor for a stream's default topology configuration, with
 * a file path to the topology XML file. The file is unused, except for
 * navigating to topology file location in the TopoViews configuration dialog.
 * 
 * @param fpath Path to the topology file housing topology.
 */
TopoViewConfig::TopoViewConfig(const std::string& fpath)
: _topo_fpath(fpath) {}

/**
 * @brief Getter for the topology file path.
 * 
 * @return File path to the topology file.
 */
const std::string& TopoViewConfig::getTopoFilepath() const
{ return _topo_fpath; }

// HwlocNUMATopoViewConfig

/**
 * @brief Explicit constructor for a stream's topology configuration. Loads
 * up hwloc topology from the given XML file and creates the configuration
 * with data from the hwloc topology inspection.
 * 
 * @param fpath Path to the topology file to be used for the configuration.
 */
HwlocNUMATopoViewConfig::HwlocNUMATopoViewConfig(const std::string& fpath)
: TopoViewConfig(fpath), _brief_topo({}) {
    hwloc_topology_t topology = get_hwloc_topology(_topo_fpath);
    
    if (topology == nullptr) {
        // Cannot load topology to fill out the brief topology
        return;
    }

    int n_numa_nodes = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_NUMANODE);
	for (unsigned int i = 0; i < (unsigned int)n_numa_nodes; i++) {
		hwloc_obj_t node = hwloc_get_obj_by_type(topology, HWLOC_OBJ_NUMANODE, i);
		// Iterate over the PUs in the cpuset
		unsigned int id = 0;
		hwloc_bitmap_foreach_begin(id, node->cpuset)
			hwloc_obj_t pu = hwloc_get_pu_obj_by_os_index(topology, id);
			hwloc_obj_t core_of_pu = hwloc_get_ancestor_obj_by_type(topology, HWLOC_OBJ_CORE, pu);
			_brief_topo[node->logical_index][core_of_pu->logical_index][pu->logical_index] = pu->os_index;
		hwloc_bitmap_foreach_end();
	}
    hwloc_topology_destroy(topology);
    topology = nullptr;
}

/**
 * @brief Empty constructor for a stream's topology configuration.
 */
HwlocNUMATopoViewConfig::HwlocNUMATopoViewConfig()
: TopoViewConfig(""), _brief_topo({}) {}

/**
 * @brief Getter for the brief topology from hwloc.
 * 
 * @return Map of nodes (their logical IDs) to their cores (map of their logical IDs
 * to their PUs (map of their logical IDs to their OS IDs)).
 */
const HwlocNUMATopoViewConfig::TopoNodeCorePU&
HwlocNUMATopoViewConfig::getBriefTopology() const
{ return _brief_topo; }

/**
 * @brief Rearranges the given CPU IDs according to the brief topology
 * stored in the configuration. The rearranged CPU IDs are returned
 * in a new vector.
 * 
 * @param cpu_ids Vector of CPU IDs to be rearranged.
 * @return Rearranged vector of CPU IDs.
 */
QVector<int> HwlocNUMATopoViewConfig::rearrangeCPUs(const QVector<int>& cpu_ids) const
{ return rearrangeCPUsWithBriefTopo(cpu_ids, _brief_topo); }

/**
 * @brief Rearranges the given CPU IDs according to the brief topology
 * passed as an argument. The rearranged CPU IDs are returned
 * in a new vector.
 * 
 * @param cpu_ids Vector of CPU IDs to be rearranged.
 * @param brief_topo Brief topology to be used for rearranging the CPU IDs.
 * @return Rearranged vector of CPU IDs.
 */
QVector<int> HwlocNUMATopoViewConfig::rearrangeCPUsWithBriefTopo
(const QVector<int>& cpu_ids, const TopoNodeCorePU& brief_topo) const {
    QVector<int> rearranged{};

    // Since maps are sorted, the resulting vector will be sorted as well
	for (const auto& [node_lid, cores]: brief_topo) {
		for (const auto& [core_lid, PUs]: cores) {
			for (const auto& [pu_lid, pu_osid]: PUs) {
                if (cpu_ids.contains(pu_osid)) {
				    rearranged.append(pu_osid);
                }
			}
		}
	}

	return rearranged;
}

/**
 * @brief Gets the number of PUs in the topology.
 * 
 * @return Number of PUs in the topology or
 * -1 if the topology could not be loaded.
 */
int TopoViewConfig::getTopologyNPUs() const {
    hwloc_topology_t topology = get_hwloc_topology(_topo_fpath);
    if (topology == nullptr) {
        return -1;
    }
    int topo_npus = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU);
    hwloc_topology_destroy(topology);
    topology = nullptr;

    return topo_npus;
}

// Global functions

/**
 * @brief Counts the number of PUs in the given brief topology.
 * 
 * @param brief_topo Brief topology to count the PUs in.
 * @return Number of PUs in the brief topology.
 */
int numatv_count_PUs(const HwlocNUMATopoViewConfig::TopoNodeCorePU& brief_topo) {
    int count = 0;
    for (const auto& [node_lid, cores]: brief_topo) {
        for (const auto& [core_lid, PUs]: cores) {
            count += PUs.size();
        }
    }
    return count;
}

/**
 * @brief Counts the number of cores in the given brief topology.
 * 
 * @param brief_topo Brief topology to count the cores in.
 * @return Number of cores in the brief topology.
 */
int numatv_count_cores(const HwlocNUMATopoViewConfig::TopoNodeCorePU& brief_topo) {
    int count = 0;
    for (const auto& [node_lid, cores]: brief_topo) {
        count += cores.size();
    }
    return count;
}

/**
 * @brief Returns a filtered brief topology based on the given PUs. The filtered topology
 * contains only the PUs that are present in the given list of PUs.
 * 
 * @param brief_topo Brief topology to be filtered.
 * @param PUs List of PUs to filter the brief topology by.
 * @return New brief topology containing only the PUs that are present in the given list.
 */
HwlocNUMATopoViewConfig::TopoNodeCorePU numatv_filter_by_PUs(
    const HwlocNUMATopoViewConfig::TopoNodeCorePU& brief_topo,
    const QVector<int>& PUs)
{
        HwlocNUMATopoViewConfig::TopoNodeCorePU filtered_topo{};

    for (const auto& [node_lid, cores]: brief_topo) {
        for (const auto& [core_lid, PUs_map]: cores) {
            for (const auto& [pu_lid, pu_osid]: PUs_map) {
                if (PUs.contains(pu_osid)) {
                    filtered_topo[node_lid][core_lid][pu_lid] = pu_osid;
                }
            }
        }
    }

    return filtered_topo;
}

/**
 * @brief Checks whether a given stream wants to show a topology widget or
 * use the default view (no topology widget).
 * 
 * @param stream_id ID of the stream to check for.
 * @param topoviews_ctx Topology Views context, containing the topology configurations.
 * @return True if the stream wants to show a topology widget, false otherwise.
 */
bool stream_wants_topology_widget(int stream_id, const KsTopoViewsContext& topoviews_ctx)
{
	bool show_this_topo = false;
	if (topoviews_ctx.existsFor(stream_id)) {
		const TopoViewConfig* cfg_observer = topoviews_ctx.observeConfig(stream_id);
		show_this_topo = (cfg_observer->getViewType() != TopoViewType::DEFAULT);
	}
		
	return show_this_topo;
}

// END of change