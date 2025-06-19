//NOTE: Changed here. (NUMA TV) (2025-04-04)

/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    KsNUMATopologyViews.cpp
 *  @brief   Implementations for working with NUMA topologies.
*/

// hwloc
#include "hwloc.h"

// C++
#include <string>
#include <iostream>

// Qt
#include <QVector>

// KernelShark
#include "KsNUMATopologyViews.hpp"
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
{ return _active_numatvs.count(stream_id) > 0; }

/**
 * @brief Checks whether a given stream wants to show a topology widget or
 * use the default view (no topology widget).
 * 
 * @param stream_id ID of the stream to check for.
 * @return True if the stream wants to show a topology widget, false otherwise.
 */
bool KsTopoViewsContext::streamWantsTopoWidget(int stream_id) const {
	bool show_this_topo = false;
	if (existsFor(stream_id)) {
		const StreamNUMATopologyConfig* cfg_observer = observeConfig(stream_id);
		show_this_topo = (cfg_observer->getViewType() != TopoViewType::DEFAULT);
	}
		
	return show_this_topo;
}

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
    
    auto new_topo_cfg = StreamNUMATopologyConfig(view, topology_file);
    int topo_npus = new_topo_cfg.getTopologyNPUs();
    
    if (topo_npus == stream_ncpus) {
        _active_numatvs[stream_id] = std::move(new_topo_cfg);
        retval = 0;
    }

    return retval;
}

/**
 * @brief Attempts to update an existing topology configuration for a given stream ID.
 * If the topology file has changed, a new configuration is created. If only the view type
 * has changed, the existing configuration is updated.
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
        StreamNUMATopologyConfig& topo_cfg = _active_numatvs.at(stream_id);

        if (topology_file != topo_cfg.getTopoFilepath()) {
            // Topology file changed - create new topology
            retval = addConfig(stream_id, view, topology_file);    
        } else if (topo_cfg.getViewType() != view) {
            // View type changed, but not the topology file
            // Just update the view type and request redraw
            topo_cfg.setViewType(view);
            retval = 0;
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
const StreamNUMATopologyConfig* KsTopoViewsContext::observeConfig(int stream_id) const {
    if (existsFor(stream_id)) {
        const StreamNUMATopologyConfig* topo_cfg = &(_active_numatvs.at(stream_id));
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
{ return _active_numatvs.erase(stream_id); }

/**
 * @brief Deletes all topology configurations from the context.
 */
void KsTopoViewsContext::clear()
{ _active_numatvs.clear(); }

// StreamNUMATopologyConfig

/**
 * @brief Explicit constructor for a stream's topology configuration. Loads
 * up hwloc topology from the given XML file and creates the configuration
 * with data from the hwloc topology inspection. Other part of the
 * configuration is which view to use for a stream (currently either NUMATREE,
 * which visualises the NUMA topology in a topology tree in the main KernelShark
 * window, or DEFAULT - no topology information shall be shown).
 * 
 * @param view Type of view to be used for the topology configuration.
 * @param fpath Path to the topology file to be used for the configuration.
 */
StreamNUMATopologyConfig::StreamNUMATopologyConfig(TopoViewType view, const std::string& fpath)
: _applied_view(view), _topo_fpath(fpath), _brief_topo({}) {
    hwloc_topology_t topology = get_hwloc_topology(_topo_fpath);
    
    if (topology == nullptr) {
        // Cannot load topology to fill out the brief topology
        return;
    }

    hwloc_obj_t machine = hwloc_get_root_obj(topology);
    unsigned int id = 0;
	hwloc_bitmap_foreach_begin(id, machine->cpuset)
        hwloc_obj_t pu = hwloc_get_pu_obj_by_os_index(topology, id);
        hwloc_obj_t core_of_pu = hwloc_get_ancestor_obj_by_type(topology, HWLOC_OBJ_CORE, pu);
        int pu_nnode_osid = hwloc_bitmap_first(pu->nodeset);
        hwloc_obj_t node = hwloc_get_numanode_obj_by_os_index(topology, pu_nnode_osid);
        _brief_topo[node->logical_index][core_of_pu->logical_index][pu->logical_index] = pu->os_index;   
    hwloc_bitmap_foreach_end();
    hwloc_topology_destroy(topology);
    topology = nullptr;
}

/**
 * @brief Empty constructor for a stream's topology configuration.
 */
StreamNUMATopologyConfig::StreamNUMATopologyConfig()
: _applied_view(TopoViewType::DEFAULT), _topo_fpath(""), _brief_topo({}) {}

/**
 * @brief Copy constructor for a stream's topology configuration.
 * 
 * @param other Other stream's topology configuration to copy from.
 */
StreamNUMATopologyConfig::StreamNUMATopologyConfig(const StreamNUMATopologyConfig& other)
: _applied_view(other._applied_view), _topo_fpath(other._topo_fpath), _brief_topo(other._brief_topo) {}

/**
 * @brief Copy assignment operator for a stream's topology configuration.
 * 
 * @param other Other stream's topology configuration to copy from.
 * @return Reference to the current object.
 */
StreamNUMATopologyConfig& StreamNUMATopologyConfig::operator=(const StreamNUMATopologyConfig& other) {
    if (this != &other) {
        _applied_view = other._applied_view;
        _topo_fpath = other._topo_fpath;
        _brief_topo = other._brief_topo;
    }
    return *this;
}

/**
 * @brief Move constructor for a stream's topology configuration.
 * 
 * @param other Other stream's topology configuration to move data from.
 */
StreamNUMATopologyConfig::StreamNUMATopologyConfig(StreamNUMATopologyConfig&& other) noexcept
: _applied_view(other._applied_view), _topo_fpath(std::move(other._topo_fpath)),
  _brief_topo(std::move(other._brief_topo)) {}

/**
 * @brief Move assignment operator for a stream's topology configuration.
 * 
 * @param other Other stream's topology configuration to move data from.
 * @return Reference to the current object.
 */
StreamNUMATopologyConfig& StreamNUMATopologyConfig::operator=(StreamNUMATopologyConfig&& other) noexcept {
    if (this != &other) {
        _applied_view = other._applied_view;
        _topo_fpath = std::move(other._topo_fpath);
        _brief_topo = std::move(other._brief_topo);
    }
    return *this;
}

/**
 * @brief Getter for the topology file path.
 * 
 * @return File path to the topology file.
 */
const std::string& StreamNUMATopologyConfig::getTopoFilepath() const
{ return _topo_fpath; }

/**
 * @brief Getter for the view type.
 * 
 * @return Type of the view to be used when visualising the stream's
 * topology.
 */
TopoViewType StreamNUMATopologyConfig::getViewType() const
{ return _applied_view; }

/**
 * @brief Sets a new view type for the configuration.
 * 
 * @param new_view New view type to be used.
 */
void StreamNUMATopologyConfig::setViewType(TopoViewType new_view)
{ _applied_view = new_view; }
/**
 * @brief Getter for the brief topology from hwloc.
 * 
 * @return Map of nodes (their logical IDs) to their cores (map of their logical IDs
 * to their PUs (map of their logical IDs to their OS IDs)).
 */
const TopoNodeCorePU& StreamNUMATopologyConfig::getBriefTopology() const
{ return _brief_topo; }

/**
 * @brief Rearranges the given CPU IDs according to the brief topology
 * stored in the configuration. The rearranged CPU IDs are returned
 * in a new vector.
 * 
 * @param cpu_ids Vector of CPU IDs to be rearranged.
 * @return Rearranged vector of CPU IDs.
 */
QVector<int> StreamNUMATopologyConfig::rearrangeCPUs(const QVector<int>& cpu_ids) const
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
QVector<int> StreamNUMATopologyConfig::rearrangeCPUsWithBriefTopo
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
int StreamNUMATopologyConfig::getTopologyNPUs() const {
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
int numatv_count_PUs(const TopoNodeCorePU& brief_topo) {
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
int numatv_count_cores(const TopoNodeCorePU& brief_topo) {
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
TopoNodeCorePU numatv_filter_by_PUs(const TopoNodeCorePU& brief_topo, const QVector<int>& PUs) {
    TopoNodeCorePU filtered_topo{};

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

// END of change