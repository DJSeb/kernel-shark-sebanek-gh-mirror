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
// C++
#include <map>
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

//NOTE: Changed here. (NUMA TV) (2025-04-15)
/**
 * @brief Reimplemented scroll area for topology view,
 * so that mouse wheel events are completely ignored.
 * 
 */
class KsTopologyScrollArea : public QScrollArea {
public:
	/**
	 * @brief Construct a new Ks Topology Scroll Area object, just
	 * like a normal scroll area.
	 * 
	 * @param parent Parent widget.
	 */
	explicit KsTopologyScrollArea(QWidget *parent = nullptr)
	: QScrollArea(parent) {}

	/**
	 * @brief Reimplemented handler for mouse wheel events. All mouse wheel
	 * events will be ignored.
	 * 
	 * @param evt Event to (not) handle.
	 */
	void wheelEvent([[maybe_unused]] QWheelEvent *evt) override {}
};
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-17)
// Forward declaration of the KsStreamTopology class, so
// that it can be used in the KsTraceGraph class.
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

	void cpuReDraw(int sd, QVector<int> cpus);

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
	
	//NOTE: Changed here. (NUMA TV) (2025-04-22)
	/**
	 * @brief Getter of the streams' topology configrations' context.
	 * 
	 * @return Reference to the NUMA TV context.
	 */
	KsNUMATVContext& getNUMATVContext() { return _numaTvCtx; }
	// END of change

	void numatvHideTopologyWidget(bool hide);

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
	
	void _setup_numatv_topo_widget();

	void _numatv_insert_topology_widget(int stream_id,
		const NodeCorePU& brief_topo);

	void _numatv_remove_topology_widget(int stream_id);

	void _numatv_existing_topology_action(int stream_id,
		QVector<int>& cpusToDraw);

	void _numatv_redraw_topo_widgets(int stream_id,
		QVector<int>& cpusToDraw);
	
	void _numatv_adjust_topo_task_padding(int stream_id);

	void _numatv_hide_stream_topo(int stream_id, bool hide);
	// END of change

	QString _t2str(uint64_t sec, uint64_t usec);

	QToolBar	_pointerBar, _navigationBar;

	QPushButton	_zoomInButton, _quickZoomInButton;

	QPushButton	_zoomOutButton, _quickZoomOutButton;

	QPushButton	_scrollLeftButton, _scrollRightButton;

	QLabel	_labelP1, _labelP2,				  // Pointer
		_labelI1, _labelI2, _labelI3, _labelI4, _labelI5; // Proc. info

	//NOTE: Changed here. (NUMA TV) (2025-04-12)
	/**
	 * @brief Configuration object for topology data of all streams. 
	 */
	KsNUMATVContext _numaTvCtx;

	/**
	 * @brief Wrapper for the topology widget and the GL widget.
	 */
	QWidget _topoGlWrapper;

	/**
	 * @brief Layout for the topology widget and the GL widget.
	 */
	QHBoxLayout _topoGlLayout;

	/**
	 * @brief Scroll area for the wrapper topology widget.
	 * 
	 * This scroll area is used to allow scrolling of the topology
	 * widget when it is larger than the available space and to
	 * be synchronised with the GL widget's scroll area.
	 * 
	 * It itself cannot be scrolled, except horizontally.
	 */
	KsTopologyScrollArea _topoScrollArea;
	
	/**
	 * @brief Button to hide/show the wrapper topology widget.
	 */
	QPushButton _hideTopoBtn;
	
	/**
	 * @brief Widget which is the parent to all topology widgets
	 * belonging to streams. It is used as the space where they will
	 * be displayed - the wrapper topology widget.
	 */
	QWidget	_topoSpace;
	
	/**
	 * @brief Layout for KsStreamTopology widgets.
	 */
	QVBoxLayout _topoLayout;

	/**
	 * @brief What topology widgets KernelShark should display.
	 * Key is the stream id, value is the topology widget.
	 * 
	 * @note This is a map of pointers - yet they aren't
	 * explicitly deleted when this object is destroyed.
	 * This is because the topology widgets are deleted
	 * automatically when the parent widget is deleted, so
	 * additionally deleting them would be redundant (and
	 * produce faults). Every topology widget has a parent,
	 * that being _topoSpace.
	 */
	std::map<int, KsStreamTopology*> _topoWidgets;
	// END of change

	KsGraphScrollArea	_scrollArea;

	KsGLWidget	_glWindow;

	QVBoxLayout	_layout;

	KsDualMarkerSM	*_mState;

	KsDataStore 	*_data;

	bool		 _keyPressed;
};

//NOTE: Changed here. (NUMA TV) (2025-04-17)
/**
 * @brief Widget for displaying a topology of a stream as
 * a block tree. While any kind of Node-Core-PU topology can be
 * displayed, where Core holds PUs (CPUs in KShark) and Node
 * holds Cores, currently only NUMA topology is supported.
 */
class KsStreamTopology : public QWidget {
private: // Qt parts
	/// @brief Main layout of the widget.
	QVBoxLayout _main_layout;

	/// @brief Container of the topology tree.
	QWidget _topo;
	
	/// @brief Layout of the topology tree.
	QHBoxLayout _topo_layout;

	/// @brief Label for the machine (stream) name and a root
	/// of the topology tree.
	QLabel _machine;

	/// @brief Container of the nodes.
	QWidget _nodes;

	/// @brief Layout of the nodes.
	QVBoxLayout _nodes_layout;

	/// @brief Container of the cores.
	QWidget _cores;

	/// @brief Layout of the cores.
	QVBoxLayout _cores_layout;
public:
	explicit KsStreamTopology(int stream_id, const NodeCorePU& brief_topo,
		const KsTraceGraph* trace_graph, QWidget* parent = nullptr);

	void hide_topology(bool hide);

	void resize_topology_widget(int new_height);

private:
	void _setup_widget_structure(int v_spacing);

	void _setup_widget_layouts();

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
