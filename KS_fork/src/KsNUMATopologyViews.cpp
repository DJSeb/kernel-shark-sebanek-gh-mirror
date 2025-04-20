//NOTE: Changed here. (NUMA TV) (2025-04-04)

/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    KsNUMATopologyViews.cpp
 *  @brief   Implementations for working with NUMA topologies.
*/

// hwloc
#include "hwloc.h"

// Qt
#include <QVector>

// KernelShark
#include "KsNUMATopologyViews.hpp"
#include "libkshark.h"


// Statics
static hwloc_topology_t get_hwloc_topology(const std::string& topo_fpath) {
    int hwloc_load_result;
    hwloc_topology_t topology;
    hwloc_load_result = hwloc_topology_init(&topology);
    if (hwloc_load_result != 0) {
        printf("[FAIL] Not enough memory for allocation of topology...\n");
        topology = nullptr;
        return nullptr;
    }
    hwloc_load_result = hwloc_topology_set_xml(topology, topo_fpath.c_str());

    if (hwloc_load_result != 0) {
        printf("[FAIL] Couldn't set topology to be loaded by XML file...\n");
        hwloc_topology_destroy(topology);
        topology = nullptr;
        return nullptr;
    }

    hwloc_load_result = hwloc_topology_load(topology);
    if (hwloc_load_result != 0) {
        printf("[FAIL] Couldn't load topology...\n");
        hwloc_topology_destroy(topology);
        topology = nullptr;
        return nullptr;
    }
    return topology;
}

int StreamTopologyConfig::get_topo_npus() const {
    hwloc_topology_t topology = get_hwloc_topology(topo_fpath);
    int topo_npus = hwloc_get_nbobjs_by_type(topology, HWLOC_OBJ_PU);
    hwloc_topology_destroy(topology);
    topology = nullptr;

    return topo_npus;
}

NUMATVContext& NUMATVContext::get_instance() {
    static NUMATVContext instance;
    return instance;
}

NUMATVContext::NUMATVContext() : _active_numatvs({}) {}

bool NUMATVContext::exists_for(int stream_id) const
{ return _active_numatvs.count(stream_id) > 0; }

int NUMATVContext::add_config(int stream_id, ViewType view, const std::string& topology_file) {
    kshark_context *kshark_ctx(nullptr);
    int retval = -1;
    
    if (!kshark_instance(&kshark_ctx)) {
        return -2;
    }

    kshark_data_stream* stream = kshark_get_data_stream(kshark_ctx, stream_id);
    int stream_ncpus = stream->n_cpus;
    
    auto new_topo_cfg = StreamTopologyConfig(view, topology_file);

    int topo_npus = new_topo_cfg.get_topo_npus();
    
    if (topo_npus == stream_ncpus) {
        _active_numatvs[stream_id] = std::move(new_topo_cfg);
        retval = 0;
    }

    return retval;
}

int NUMATVContext::update_cfg(int stream_id, ViewType view, const std::string& topology_file) {
    kshark_context *kshark_ctx(nullptr);
    bool retval = -1;
    
    if (_active_numatvs.count(stream_id) > 0) {
        StreamTopologyConfig& topo_cfg = _active_numatvs.at(stream_id);
        if (topology_file != topo_cfg.topo_fpath || topo_cfg.get_view_type() != view) {
            if (!kshark_instance(&kshark_ctx)) {
                return -2;
            }

            kshark_data_stream* stream = kshark_get_data_stream(kshark_ctx, stream_id);
            int stream_ncpus = stream->n_cpus;
            
            auto new_topo_cfg = StreamTopologyConfig(view, topology_file);
            int topo_npus = new_topo_cfg.get_topo_npus();
            
            if (topo_npus == stream_ncpus) {
                _active_numatvs[stream_id] = std::move(new_topo_cfg);
                retval = 0;
            }
        } else {
            retval = 1;
        }
    }

    return retval;
}

const StreamTopologyConfig* NUMATVContext::observe_cfg(int stream_id) const {
    if (_active_numatvs.count(stream_id) > 0) {
        const StreamTopologyConfig* topo_cfg = &(_active_numatvs.at(stream_id));
        return topo_cfg;
    }
    return nullptr;
}

int NUMATVContext::delete_cfg(int stream_id) {
    kshark_context *kshark_ctx(nullptr);
    if (!kshark_instance(&kshark_ctx)) {
        return 0;
    }

    kshark_data_stream* stream = kshark_get_data_stream(kshark_ctx, stream_id);

    return _active_numatvs.erase(stream_id);
}

void NUMATVContext::clear()
{ _active_numatvs.clear(); }

// StreamTopologyConfig
StreamTopologyConfig::StreamTopologyConfig()
: applied_view(ViewType::DEFAULT), topo_fpath(""), _brief_topo({}) {}

StreamTopologyConfig::StreamTopologyConfig(ViewType view, const std::string& fpath)
: applied_view(view), topo_fpath(fpath), _brief_topo({}) {
    hwloc_topology_t topology = get_hwloc_topology(topo_fpath);
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

StreamTopologyConfig::StreamTopologyConfig(const StreamTopologyConfig& other)
: applied_view(other.applied_view), topo_fpath(other.topo_fpath), _brief_topo(other._brief_topo) {}

StreamTopologyConfig& StreamTopologyConfig::operator=(const StreamTopologyConfig& other) {
    if (this != &other) {
        applied_view = other.applied_view;
        topo_fpath = other.topo_fpath;
        _brief_topo = other._brief_topo;
    }
    return *this;
}

StreamTopologyConfig::StreamTopologyConfig(StreamTopologyConfig&& other) noexcept
: applied_view(other.applied_view), topo_fpath(std::move(other.topo_fpath)),
  _brief_topo(std::move(other._brief_topo)) {}

StreamTopologyConfig& StreamTopologyConfig::operator=(StreamTopologyConfig&& other) noexcept {
    if (this != &other) {
        applied_view = other.applied_view;
        topo_fpath = std::move(other.topo_fpath);
        _brief_topo = std::move(other._brief_topo);
    }
    return *this;
}

StreamTopologyConfig::~StreamTopologyConfig() {}

const std::string& StreamTopologyConfig::get_topo_fpath() const
{ return topo_fpath; }

ViewType StreamTopologyConfig::get_view_type() const
{ return applied_view; }

const NodeCorePU& StreamTopologyConfig::get_brief_topo() const
{ return _brief_topo; }

QVector<int> StreamTopologyConfig::rearrangeCPUs(const QVector<int>& cpu_ids) const {
	NodeCorePU brief_topo = get_brief_topo();
    return rearrangeCPUsWithBriefTopo(cpu_ids, brief_topo);
}

QVector<int> StreamTopologyConfig::rearrangeCPUsWithBriefTopo
(const QVector<int>& cpu_ids, const NodeCorePU& brief_topo) const {
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

// Global functions

int numatv_count_PUs(const NodeCorePU& brief_topo) {
    int count = 0;
    for (const auto& [node_lid, cores]: brief_topo) {
        for (const auto& [core_lid, PUs]: cores) {
            count += PUs.size();
        }
    }
    return count;
}

int numatv_count_cores(const NodeCorePU& brief_topo) {
    int count = 0;
    for (const auto& [node_lid, cores]: brief_topo) {
        count += cores.size();
    }
    return count;
}

NodeCorePU numatv_filter_by_PUs(const NodeCorePU& brief_topo, QVector<int> PUs) {
    NodeCorePU filtered_topo{};

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