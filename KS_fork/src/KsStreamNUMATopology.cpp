//NOTE: Changed here. (NUMA TV) (2025-06-19)

/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

/**
 *  @file    KsStreamNUMATopology.hpp
 *  @brief   Implementation of th widget for visualising NUMA topology
 *           of a stream as a block tree.
*/

// KernelShark
#include "KsGLWidget.hpp"
#include "KsNUMATopologyViews.hpp"
#include "KsStreamNUMATopology.hpp"
#include "KsTraceGraph.hpp"

//NOTE: Changed here. (NUMA TV) (2025-04-17)
/**
 * @brief Explicit constructor for the stream topology widget.
 * 
 * @param stream_id Identifier of the stream.
 * @param brief_topo Brief topology necessary for creation of the
 * topology widget.
 * @param trace_graph Pointer to the trace graph.
 * @param parent Parent widget.
 */
KsStreamNUMATopology::KsStreamNUMATopology(int stream_id, const TopoNodeCorePU& brief_topo,
	const KsTraceGraph* trace_graph, QWidget* parent)
: QWidget(parent),
  _mainLayout(this),
  _topo(this),
  _topoLayout(&_topo),
  _nodes(&_topo),
  _nodesLayout(&_nodes),
  _cores(&_topo),
  _coresLayout(&_cores)
{
	KsGLWidget* gl_widget = const_cast<KsTraceGraph *>(trace_graph)->glPtr();
	int v_spacing = gl_widget->vSpacing();
	this->setContentsMargins(0, 0, 0, 0);
	
	_setupWidgetStructure(v_spacing);
	_setupWidgetLayouts();
	_setupTopologyTree(stream_id, v_spacing, brief_topo, gl_widget);
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-17)
/**
 * @brief Hides or shows the topology widget.
 * 
 * @param hide Whether to hide or show the topology widget.
 */
void KsStreamNUMATopology::hideTopology(bool hide)
{ _topo.setHidden(hide); }
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-20)
/**
 * @brief Resizes the topology widget to a new fixed height.
 * 
 * @param new_height The new height for the topology widget.
 */
void KsStreamNUMATopology::resizeTopologyWidget(int new_height)
{ this->setFixedHeight(std::max(new_height, 0)); }
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-19)
/**
 * @brief Sets up Spacings, margins and alignements for the
 * wiget an its layouts.
 * 
 * @param v_spacing Vertical spacing between the widgets used in
 * KernelShark's GL widget.
 */
void KsStreamNUMATopology::_setupWidgetStructure(int v_spacing) {
	_mainLayout.setContentsMargins(0, 0, 0, 0);
	_mainLayout.setSpacing(0);
	_mainLayout.setAlignment(Qt::AlignTop);

	_topo.setContentsMargins(0, 0, 0, 0);
	_topo.setMinimumHeight(0);
	
	_topoLayout.setContentsMargins(0, 0, 0, 0);
	_topoLayout.setSpacing(0);
	
	_nodes.setContentsMargins(0, 0, 0, 0);
	
	_nodesLayout.setContentsMargins(0, 0, 0, 0);
	_nodesLayout.setSpacing(v_spacing);
	
	_cores.setContentsMargins(0, 0, 0, 0);

	_coresLayout.setContentsMargins(0, 0, 0, 0);
	_coresLayout.setSpacing(v_spacing);
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-19)
/**
 * @brief Sets up layouts and their items for the topology widget.
 */
void KsStreamNUMATopology::_setupWidgetLayouts() {
	this->setLayout(&_mainLayout);
	_topo.setLayout(&_topoLayout);
	_nodes.setLayout(&_nodesLayout);
	_cores.setLayout(&_coresLayout);

	_topoLayout.addWidget(&_nodes);
	_topoLayout.addWidget(&_cores);

	_mainLayout.addWidget(&_topo);
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-19)
/**
 * @brief Creates a stylesheet for the topology tree items.
 * Expected stylesheet is an item with a black 1px solid border,
 * a background color specified in the argument and a text color
 * which is determined by the background color to be black or white
 * for readability.
 * 
 * @param color The color to be used in the stylesheet.
 * @return The stylesheet as a QString.
 */
static QString make_topo_item_stylesheet(const KsPlot::Color& color) {
	QString bg_color_str = QString{"%1, %2, %3"}
		.arg(color.r())
		.arg(color.g())
		.arg(color.b()
	);

	float bg_intensity = KsPlot::getColorIntensity(color);
	KsPlot::Color text_color = KsPlot::blackOrWhite(bg_intensity);
	QString text_color_str = QString{"%1, %2, %3"}
		.arg(text_color.r())
		.arg(text_color.g())
		.arg(text_color.b()
	);

	QString stylesheet = QString{
		"QLabel {border: 1px solid black;"
		"background-color: rgb(" + bg_color_str + ");"
		"color: rgb(" + text_color_str + ");}"};
	
	return stylesheet;
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-19)
/**
 * @brief Sets up a Core node of the topology tree - its height,
 * its color, its text, its tooltip and puts it in the cores' layout.
 * 
 * @param core_lid Logical index of the core in the topology.
 * @param node_lid Logical index of the owner node in the topology.
 * @param v_spacing Spacing between the graphs in KernelShark's GL widget.
 * @param more_numas Whether the topology includes multiple NUMA nodes.
 * @param PUs Collection of PUs in the core.
 * @param gl_widget Pointer to the GL widget from which to get
 * CPU colors.
 * @param node_parent Parent widget of the core node.
 * @param node_reds Output accumulator for the red color component
 * of core's PUs.
 * @param node_greens Output accumulator for the green color component
 * of core's PUs.
 * @param node_blues Output accumulator for the blue color component
 * of core's PUs.
 * @return Height of the created core node.
 */
int KsStreamNUMATopology::_setupTopologyTreeCore(int core_lid, int node_lid,
	int v_spacing, bool more_numas, const TopoPUIds& PUs,
	const KsGLWidget* gl_widget, QWidget* node_parent,
	unsigned int& node_reds, unsigned int& node_greens,
	unsigned int& node_blues)
{
	QLabel* core = new QLabel(node_parent);

	// Appropriately create the label and tooltip for the core node
	// with regards to amount of NUMA nodes in the topology.
	QString core_node_label;
	QString core_node_tooltip;
	if (more_numas) {
		core_node_label = QString{"(NN %1) C %2"}.arg(node_lid).arg(core_lid);
		core_node_tooltip = QString{"Core %1 in NUMA Node %2"}.arg(core_lid)
			.arg(node_lid);
	} else {
		core_node_label = QString{"C %1"}.arg(core_lid);
		core_node_tooltip = QString{"Core %1"}.arg(core_lid);
	}
	core->setText(core_node_label);
	core->setToolTip(core_node_tooltip);

	core->setAlignment(Qt::AlignCenter);
	_coresLayout.addWidget(core);

	unsigned int pu_reds = 0;
	unsigned int pu_greens = 0;
	unsigned int pu_blues = 0;

	const KsPlot::ColorTable& cpu_cols = gl_widget->getCPUColors();
	for (const auto& [pu_lid, pu_osid] : PUs) {
		const KsPlot::Color& pu_color = cpu_cols.at(pu_osid);
		pu_reds += pu_color.r();
		pu_greens += pu_color.g();
		pu_blues += pu_color.b();
	}

	int cpus_in_core = int(PUs.size());
	int n_spacings = (cpus_in_core - 1);
	int graph_heights = (KS_GRAPH_HEIGHT * cpus_in_core);
	int spacings_heights = (n_spacings * v_spacing);
	int core_height = graph_heights + spacings_heights;
	
	core->setFixedHeight(core_height);
	
	auto bg_color = KsPlot::Color{
		uint8_t(pu_reds / cpus_in_core),
		uint8_t(pu_greens / cpus_in_core),
		uint8_t(pu_blues / cpus_in_core)
	};
	core->setStyleSheet(make_topo_item_stylesheet(bg_color));

	node_reds += bg_color.r();
	node_greens += bg_color.g();
	node_blues += bg_color.b();

	return core_height;
}
// END of change


//NOTE: Changed here. (NUMA TV) (2025-04-19)
/**
 * @brief Sets up a Node node of the topology tree - its height,
 * its color, its text, its tooltip and puts it in the nodes' layout.
 * Also calls setup of the cores in the node. If given topology has only
 * one NUMA node, the node is not created and containing widget is
 * hidden.
 * 
 * @param node_lid Logical index of the node in the topology.
 * @param v_spacing Spacing between the graphs in KernelShark's GL widget.
 * @param more_numas Whether the topology includes multiple NUMA nodes.
 * @param cores Collection of cores in the node.
 * @param gl_widget Pointer to the GL widget from which to get
 * CPU colors.
 */
void KsStreamNUMATopology::_setupTopologyTreeNode(int node_lid, int v_spacing,
	bool more_numas, const TopoCorePU& cores, const KsGLWidget* gl_widget)
{
	unsigned int core_reds = 0;
	unsigned int core_greens = 0;
	unsigned int core_blues = 0;

	if (more_numas) {
		QLabel* node = new QLabel(&_nodes);
		node->setText(QString{"NN %1"}.arg(node_lid));
		node->setAlignment(Qt::AlignCenter);
		node->setToolTip(QString{"NUMA Node %1"}.arg(node_lid));
		_nodesLayout.addWidget(node);

		int node_height = 0;

		for (const auto& [core_lid, PUs] : cores) {
			int core_height = _setupTopologyTreeCore(core_lid,
				node_lid, v_spacing, more_numas, PUs, gl_widget,
				node, core_reds, core_greens, core_blues);
			node_height += core_height + v_spacing;
		}

		node_height -= v_spacing; // One less core spacing
		node->setFixedHeight(std::max(node_height, 0));

		int cores_in_node = int(cores.size());
		auto node_color = KsPlot::Color{
			uint8_t(core_reds / cores_in_node),
			uint8_t(core_greens / cores_in_node),
			uint8_t(core_blues / cores_in_node)
		};

		node->setStyleSheet(make_topo_item_stylesheet(node_color));
	} else {
		// Forego any node manipulations and just create cores
		// core_[color]s will be ignored, but are used because
		// of _setupTopologyTreeCore's signature.
		for (const auto& [core_lid, PUs] : cores) {
			_setupTopologyTreeCore(core_lid,
				node_lid, v_spacing, more_numas, PUs, gl_widget,
				&_topo, core_reds, core_greens, core_blues);
		}
		_nodes.setHidden(true);
	}
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-19)
/**
 * @brief Sets up the topology tree - sets up topology widgets's height and
 * asks each NUMA node to be set up.
 * 
 * @param stream_id Identifier of the stream.
 * @param v_spacing Spacing between the graphs in KernelShark's GL widget.
 * @param brief_topo Brief topology necessary for creation of the
 * topology tree.
 * @param gl_widget Pointer to the GL widget from which to get
 * stream colors for the machine node. 
 */
void KsStreamNUMATopology::_setupTopologyTree(int stream_id, int v_spacing, 
	const TopoNodeCorePU& brief_topo, KsGLWidget* gl_widget)
{
	// Set height of the widget itself
	int all_graphs = gl_widget->graphCount(stream_id);
	int all_graphs_height = KS_GRAPH_HEIGHT * all_graphs;
	int all_graphs_spacings = (all_graphs - 1) * v_spacing;
	int full_stream_height = all_graphs_height + all_graphs_spacings;
	int machine_height_or_gl_height = std::max(0, full_stream_height);
	resizeTopologyWidget(machine_height_or_gl_height);

	// Setup nodes
	bool more_numas = brief_topo.size() != 1;
	for (const auto& [node_lid, cores] : brief_topo) {
		_setupTopologyTreeNode(node_lid, v_spacing, more_numas,
			cores, gl_widget);
	}
}
// END of change