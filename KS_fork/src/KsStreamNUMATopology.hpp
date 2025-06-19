//NOTE: Changed here. (NUMA TV) (2025-06-19)

/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    KsStreamNUMATopology.hpp
 *  @brief   Widget for visualising NUMA topology of a stream
 *           as a block tree.
*/

#ifndef _KS_STREAM_NUMA_TOPOLOGY_HPP
#define _KS_STREAM_NUMA_TOPOLOGY_HPP

// Qt
#include <QtWidgets>
#include <QVector>

// KernelShark
#include "KsNUMATopologyViews.hpp"
#include "KsTraceGraph.hpp"

//NOTE: Changed here. (NUMA TV) (2025-04-17)
/**
 * @brief Widget for displaying NUMA topology of a stream as
 * a block tree.
 */
class KsStreamNUMATopology : public QWidget {
private: // Qt parts
	/// @brief Main layout of the widget.
	QVBoxLayout _mainLayout;

	/// @brief Container of the topology tree.
	QWidget _topo;
	
	/// @brief Layout of the topology tree.
	QHBoxLayout _topoLayout;

	/// @brief Container of the nodes for NUMA nodes.
	QWidget _nodes;

	/// @brief Layout of the nodes.
	QVBoxLayout _nodesLayout;

	/// @brief Container of the cores.
	QWidget _cores;

	/// @brief Layout of the cores.
	QVBoxLayout _coresLayout;
public:
	explicit KsStreamNUMATopology(int stream_id, const TopoNodeCorePU& brief_topo,
		const KsTraceGraph* trace_graph, QWidget* parent = nullptr);

	void hideTopology(bool hide);

	void resizeTopologyWidget(int new_height);

private:
	void _setupWidgetStructure(int v_spacing);

	void _setupWidgetLayouts();

	int _setupTopologyTreeCore(int core_lid, int node_lid, int v_spacing,
		bool more_numas, const TopoPUIds& PUs, const KsGLWidget* gl_widget,
		QWidget* node_parent, unsigned int& node_reds,
		unsigned int& node_greens, unsigned int& node_blues);

	void _setupTopologyTreeNode(int node_lid, int v_spacing, bool more_numas,
		const TopoCorePU& cores, const KsGLWidget* gl_widget);

	void _setupTopologyTree(int stream_id, int v_spacing,
		const TopoNodeCorePU& brief_topo, KsGLWidget* gl_widget);
};

#endif // _KS_STREAM_NUMA_TOPOLOGY_HPP
// END of change