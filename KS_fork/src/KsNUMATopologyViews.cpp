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

void NUMATVContext::add_config(int stream_id, ViewType view, const QString& topology_file) {
    if (view == ViewType::DEFAULT ||
        !std::filesystem::exists(topology_file.toStdString()) ||
        (_active_numatvs.count(stream_id) > 0 &&
            topology_file.toStdString() == _active_numatvs[stream_id].topo_fpath))
    {
        return;
    }

    _active_numatvs[stream_id] = StreamTopologyConfig(view, topology_file.toStdString());
}

// StreamTopologyConfig
StreamTopologyConfig::StreamTopologyConfig()
: applied_view(ViewType::DEFAULT), topo_fpath("-"), topology(nullptr) {}

StreamTopologyConfig::StreamTopologyConfig(ViewType view, const std::string& fpath) {
    applied_view = view;
    topo_fpath = fpath;
    int depth;
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

    printf("[SUCCESS] View mode: %d\n", static_cast<int>(view));
    printf("[SUCCESS] Topology file: %s\n", topo_fpath.c_str());
    
    depth = hwloc_topology_get_depth(topology);
    printf("[SUCCESS] Depth of topology: %d\n", depth);
}

StreamTopologyConfig::StreamTopologyConfig(const StreamTopologyConfig& other) {
    printf("[DEBUG] Copy constructor called.\n");
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
    printf("[DEBUG] Copy assignment operator called.\n");
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

StreamTopologyConfig::StreamTopologyConfig(StreamTopologyConfig&& other)
: applied_view(other.applied_view),
  topo_fpath(std::move(other.topo_fpath)), topology(other.topology) {
    printf("[DEBUG] Move constructor called.\n");
    other.topology = nullptr;
}

StreamTopologyConfig& StreamTopologyConfig::operator=(StreamTopologyConfig&& other) {
    printf("[DEBUG] Move assignment operator called.\n");
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
        printf("Destroying topology for '%s' ...\n", topo_fpath.c_str());
        hwloc_topology_destroy(topology);
        topology = nullptr;
    } else {
        printf("Topology for '%s' already destroyed.\n", topo_fpath.c_str());
    }
}

// END of change