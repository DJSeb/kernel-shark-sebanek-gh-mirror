/* SPDX-License-Identifier: LGPL-2.1 */

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsTraceGraph.hpp
 *  @brief   KernelShark Trace Graph widget.
 */
#ifndef _KS_TRACEGRAPH_H
#define _KS_TRACEGRAPH_H

//NOTE: Changed here. (NUMA TV) (2025-04-17)
// Qt
#include <QMap>
// END of change

// KernelShark
#include "KsWidgetsLib.hpp"
#include "KsGLWidget.hpp"
//NOTE: Changed here. (NUMA TV) (2025-04-17)
#include "KsNUMATopologyViews.hpp"
// END of change

/**
 * Scroll Area class, needed in order to reimplemented the handler for mouse
 * wheel events.
 */
class KsGraphScrollArea : public QScrollArea {
public:
	/** Create a default Scroll Area. */
	explicit KsGraphScrollArea(QWidget *parent = nullptr)
	: QScrollArea(parent) {}

	/**
	 * Reimplemented handler for mouse wheel events. All mouse wheel
	 * events will be ignored.
	 */
	void wheelEvent(QWheelEvent *evt) {
		if (QApplication::keyboardModifiers() != Qt::ControlModifier)
			QScrollArea::wheelEvent(evt);
	}
};

//NOTE: Changed here. !!!!!!! (NUMA TV) (2025-04-15)
class KsTopologyScrollArea : public QScrollArea {
public:
	explicit KsTopologyScrollArea(QWidget *parent = nullptr)
	: QScrollArea(parent) {}

	void wheelEvent([[maybe_unused]] QWheelEvent *evt) override {}
};
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-17)
class KsStreamTopology;
// END of change

/**
 * The KsTraceViewer class provides a widget for interactive visualization of
 * trace data shown as time-series.
 */
class KsTraceGraph : public KsWidgetsLib::KsDataWidget
{
	Q_OBJECT
public:
	explicit KsTraceGraph(QWidget *parent = nullptr);

	void loadData(KsDataStore *data, bool resetPlots);

	void setMarkerSM(KsDualMarkerSM *m);

	void reset();

	/** Get the KsGLWidget object. */
	KsGLWidget *glPtr() {return &_glWindow;}

	void markEntry(size_t);

	void cpuReDraw(int sd, QVector<int> cpus);  // NUMA TV TODO: Modify this

	void taskReDraw(int sd, QVector<int> pids);

	void comboReDraw(int sd, QVector<int> v);

	void addCPUPlot(int sd, int cpu);

	void addTaskPlot(int sd, int pid);

	void removeCPUPlot(int sd, int cpu);

	void removeTaskPlot(int sd, int pid);

	void update(KsDataStore *data);

	void updateGeom();

	void resizeEvent(QResizeEvent* event) override;

	bool eventFilter(QObject* obj, QEvent* evt) override;

	//NOTE: Changed here. (PREVIEW LABELS CHANGEABLE) (2024-07-26)
	
	void setPreviewLabels(const QString& label1 = "",
						  const QString& label2 = "",
						  const QString& label3 = "",
						  const QString& label4 = "",
						  const QString& label5 = "");
	// END of change
	
	//NOTE: Changed here. (NUMA TV) (2025-04-15)
	void numatvHideTopologyWidget(bool hide);
	// END of change

	void numatvClearTopologyWidgets();
	// END of change

signals:
	/**
	 * This signal is emitted in the case of a right mouse button click or
	 * in the case of a double click over an empty area (no visible
	 * KernelShark entries).
	 */
	void deselect();

private:

	void _zoomIn();

	void _zoomOut();

	void _quickZoomIn();

	void _quickZoomOut();

	void _scrollLeft();

	void _scrollRight();

	void _stopUpdating();

	void _resetPointer(int64_t ts, int sd, int cpu, int pid);

	void _setPointerInfo(size_t);

	void _selfUpdate();

	void _markerReDraw();

	void _updateGraphs(KsWidgetsLib::KsDataWork action);

	void _onCustomContextMenu(const QPoint &point);

	//NOTE: Changed here. (NUMA TV) (2025-04-18)
	void _numatv_insert_topology_widget(int stream_id,
		const NodeCorePU& brief_topo);

	void _numatv_remove_topology_widget(int stream_id);

	void _numatv_existing_topology_action(int stream_id, bool widget_exists,
		QVector<int>& cpusToDraw, const NUMATVContext& numa_ctx);

	void _numatv_no_topology_action(int stream_id, bool widget_exists);
	
	void _numatv_hide_stream_topo(int stream_id, bool hide);

	void _numatv_tree_view_action(int stream_id,
		QVector<int>& cpusToDraw, const StreamTopologyConfig* stream_cfg);

	void _numatv_redraw_topo_widgets(int stream_id, QVector<int>& cpusToDraw);
	// END of change

	QString _t2str(uint64_t sec, uint64_t usec);

	QToolBar	_pointerBar, _navigationBar;

	QPushButton	_zoomInButton, _quickZoomInButton;

	QPushButton	_zoomOutButton, _quickZoomOutButton;

	QPushButton	_scrollLeftButton, _scrollRightButton;

	QLabel	_labelP1, _labelP2,				  // Pointer
		_labelI1, _labelI2, _labelI3, _labelI4, _labelI5; // Proc. info

	//NOTE: Changed here. (NUMA TV) (2025-04-12)
	QWidget _topoGlWrapper;

	QHBoxLayout _topoGlLayout;

	KsTopologyScrollArea _topoScrollArea;
	
	QPushButton _hideTopoBtn;
	
	QWidget	_topoSpace;
	
	QVBoxLayout _topoLayout;

	QMap<int, KsStreamTopology*> _topoWidgets;
	// END of change

	KsGraphScrollArea	_scrollArea;

	KsGLWidget	_glWindow;

	QVBoxLayout	_layout;

	KsDualMarkerSM	*_mState;

	KsDataStore 	*_data;

	bool		 _keyPressed;
};

//NOTE: Changed here. (NUMA TV) (2025-04-17)
/*
Tree-like layout of the topology view with NUMA nodes.
It shall be constructed right to left, first matching CPUs
in KernelShark's GL window to PUs in the topology. They shall
have the same height as each CPU graph (their heights do not
change, which allows to use this invariant).
NUMA TV rearranges the CPUs, which allows construction of
such UI, that nodes and cores can be sorted by their logical
indices, as specified by hwloc. By creating the rightmost
column first, the core column can then create cores with the
height of the PU(s) it owns - analogously for NUMA nodes.
By doing this, if some cores have more or less PUs, their
height will adjust accordingly. Similarly for NUMA nodes
and their cores. It also comes with a bonus reactivity to 
hidden & visible CPUs as specified by KernelShark - construction
right to left also allows to show only the relevant parts of the
topology.

Total height is then used for the machine column, which
also denotes the height of the stream's graph. It shall use
the stream's color (if there are more streams open).

Caveats: Some exotic topologies won't work, e.g. nested NUMA nodes,
PUs shared across cores or cores shared across NUMA nodes.

Technically, it should be a visualisation of the NodeCorePU
mappings.

To take task graphs into account, spacing is added at the bottom of
the topology 

Example look:
```
______________________________________________________________
|------------------------------------------------------------|
||              |               |               |   PU P1   ||
||				|				|	core L1		|-----------||
||				|				|				|	PU P8	||
||				|	Nnode L1	|---------------|-----------||
||				|				|				|	PU P2	||
||				|				|	core L2		|-----------||
||				|				|				|	PU P7	||
||	machine		|---------------|---------------|-----------||
||	(stream X)	|				|				|	PU P3	||
|| (topo info)	|				|	core L3		|-----------||
||				|				|				|	PU P6	||
||				|	Nnode L2	|---------------|-----------||
||				|				|				|	PU P4	||
||				|				|	core L4		|-----------||
||				|				|				|	PU P5	||
|------------------------------------------------------------|
|[							SPACING							]|
--------------------------------------------------------------
```
*/
class KsStreamTopology : public QWidget {
	Q_OBJECT
private: // Qt parts
	QVBoxLayout _main_layout;
	QWidget _topo;
	QHBoxLayout _topo_layout;
	QLabel _machine;
	QWidget _nodes;
	QVBoxLayout _nodes_layout;
	QWidget _cores;
	QVBoxLayout _cores_layout;
	/*
	QWidget _PUs;
	QVBoxLayout _PUs_layout;
	*/
	QWidget _tasks_padding;
public:
	explicit KsStreamTopology(int stream_id, const NodeCorePU& brief_topo,
		const KsTraceGraph* trace_graph, QWidget* parent = nullptr);
	void hide_topology(bool hide);
	void change_task_padding(int height);
	void hide_task_padding(bool hide);
private:
	void _setup_widget_structure(int v_spacing);
	void _setup_widget_layouts();
	/*
	KsPlot::Color _setup_topology_tree_pu(int pu_lid, int pu_osid,
		int node_lid, int core_lid, const KsGLWidget* gl_widget,
		QLabel* core_parent);
	*/
	int _setup_topology_tree_core(int core_lid, int node_lid,
		int v_spacing, const PUIds& PUs, const KsGLWidget* gl_widget,
		QLabel* node_parent, unsigned int& node_reds,
		unsigned int& node_greens, unsigned int& node_blues);
	int _setup_topology_tree_node(int node_lid, int v_spacing,
		const CorePU& cores, const KsGLWidget* gl_widget);
	void _setup_topology_tree(int stream_id, int v_spacing,
		const NodeCorePU& brief_topo, KsGLWidget* gl_widget);
};
// END of change

#endif // _KS_TRACEGRAPH_H
