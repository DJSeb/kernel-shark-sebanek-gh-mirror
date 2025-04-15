//NOTE: Changed here. (NUMA TV) (2025-04-04)

/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    KsNUMATopologyViews.cpp
 *  @brief   Implementations for working with NUMA topologies.
*/

// C++
#include <filesystem>

// KernelShark
#include "KsNUMATopologyViews.hpp"

// NUMATVContext

NUMATVContext& NUMATVContext::get_instance() {
    static NUMATVContext instance;
    return instance;
}

NUMATVContext::NUMATVContext() {
    _active_numatvs = {};
}

void NUMATVContext::add_config(int stream_id, ViewType view, const std::string& topology_file) {
    if (!std::filesystem::exists(topology_file) ||
        (_active_numatvs.count(stream_id) > 0 &&
            topology_file == _active_numatvs[stream_id].topo_fpath))
    {
        return;
    }

    _active_numatvs[stream_id] = StreamTopologyConfig(view, topology_file);
}

int NUMATVContext::get_stream_topo_depth(int stream_id) const {
    if (_active_numatvs.count(stream_id) > 0) {
        return hwloc_topology_get_depth(_active_numatvs.at(stream_id).topology);
    }
    return -1;
}

bool NUMATVContext::exists_for(int stream_id) const {
    return _active_numatvs.count(stream_id) > 0;
}

const StreamTopologyConfig* NUMATVContext::observe_cfg(int stream_id) const {
    if (_active_numatvs.count(stream_id) > 0) {
        const StreamTopologyConfig* topo_cfg = &(_active_numatvs.at(stream_id));
        return topo_cfg;
    }
    return nullptr;
}

void NUMATVContext::delete_cfg(int stream_id) {
    _active_numatvs.erase(stream_id);
}

void NUMATVContext::update_cfg(int stream_id, ViewType view, const std::string& topology_file) {
    if (_active_numatvs.count(stream_id) > 0) {
        StreamTopologyConfig& topo_cfg = _active_numatvs.at(stream_id);
        if (topology_file != topo_cfg.topo_fpath) {
            _active_numatvs[stream_id] = StreamTopologyConfig(view, topology_file);
        } else {
            topo_cfg.applied_view = view;
        }
    }
}

// StreamTopologyConfig
const std::string& StreamTopologyConfig::get_topo_fpath() const
{ return topo_fpath; }

ViewType StreamTopologyConfig::get_view_type() const
{ return applied_view; }

StreamTopologyConfig::StreamTopologyConfig()
: applied_view(ViewType::DEFAULT), topo_fpath("-"), topology(nullptr) {}

StreamTopologyConfig::StreamTopologyConfig(ViewType view, const std::string& fpath) {
    applied_view = view;
    topo_fpath = fpath;
    int result;

    result = hwloc_topology_init(&topology);
    if (result != 0) {
        printf("[FAIL] Not enough memory for allocation of topology...\n");
        topology = nullptr;
        return;
    }
    result = hwloc_topology_set_xml(topology, fpath.c_str());

    if (result != 0) {
        printf("[FAIL] Couldn't set topology to be loaded by XML file...\n");
        hwloc_topology_destroy(topology);
        topology = nullptr;
        return;
    }

    result = hwloc_topology_load(topology);
    if (result != 0) {
        printf("[FAIL] Couldn't load topology...\n");
        hwloc_topology_destroy(topology);
        topology = nullptr;
        return;
    }
}

StreamTopologyConfig::StreamTopologyConfig(const StreamTopologyConfig& other) {
    applied_view = other.applied_view;
    topo_fpath = other.topo_fpath;
    topology = nullptr;

    if (other.topology != nullptr) {
        int result = hwloc_topology_dup(&topology, other.topology);
        if (result != 0) {
            printf("[FAIL] Not enough memory for allocation of topology...\n");
            topology = nullptr;
        }
    }
}

StreamTopologyConfig& StreamTopologyConfig::operator=(const StreamTopologyConfig& other) {
    if (this != &other) {
        applied_view = other.applied_view;
        topo_fpath = other.topo_fpath;

        if (topology != nullptr) {
            hwloc_topology_destroy(topology);
        }

        topology = nullptr;
        if (other.topology != nullptr) {
            int result = hwloc_topology_dup(&topology, other.topology);
            if (result != 0) {
                printf("[FAIL] Not enough memory for allocation of topology...\n");
                topology = nullptr;
            }
        }
    }
    return *this;
}

StreamTopologyConfig::StreamTopologyConfig(StreamTopologyConfig&& other) noexcept
: applied_view(other.applied_view),
  topo_fpath(std::move(other.topo_fpath)), topology(other.topology) {
    other.topology = nullptr;
}

StreamTopologyConfig& StreamTopologyConfig::operator=(StreamTopologyConfig&& other) noexcept {
    if (this != &other) {
        applied_view = other.applied_view;
        topo_fpath = std::move(other.topo_fpath);
        topology = other.topology;
        other.topology = nullptr;
    }
    return *this;
}

StreamTopologyConfig::~StreamTopologyConfig() {
    if (topology != nullptr) {
        hwloc_topology_destroy(topology);
        topology = nullptr;
    }
}

// END of change