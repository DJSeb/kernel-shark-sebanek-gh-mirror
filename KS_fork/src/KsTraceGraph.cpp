// SPDX-License-Identifier: LGPL-2.1

/*
 * Copyright (C) 2017 VMware Inc, Yordan Karadzhov <ykaradzhov@vmware.com>
 */

/**
 *  @file    KsTraceGraph.cpp
 *  @brief   KernelShark Trace Graph widget.
 */

// KernelShark
#include "KsUtils.hpp"
#include "KsDualMarker.hpp"
#include "KsTraceGraph.hpp"
#include "KsQuickContextMenu.hpp"
//NOTE: Changed here. (NUMA TV) (2025-04-17)
#include "KsNUMATopologyViews.hpp"
#include "KsPlotTools.hpp"
// END of change

/** Create a default (empty) Trace graph widget. */
KsTraceGraph::KsTraceGraph(QWidget *parent)
: KsWidgetsLib::KsDataWidget(parent),
  _pointerBar(this),
  _navigationBar(this),
  _zoomInButton("+", this),
  _quickZoomInButton("++", this),
  _zoomOutButton("-", this),
  _quickZoomOutButton("- -", this),
  _scrollLeftButton("<", this),
  _scrollRightButton(">", this),
  _labelP1("Pointer: ", this),
  _labelP2("", this),
  _labelI1("", this),
  _labelI2("", this),
  _labelI3("", this),
  _labelI4("", this),
  _labelI5("", this),
//NOTE: Changed here. (NUMA TV) (2025-04-16)
  _numaTvCtx(),
  _topoGlWrapper(this),
  _topoGlLayout(&_topoGlWrapper),
  _topoScrollArea(&_topoGlWrapper),
  _hideTopoBtn(">", &_topoGlWrapper),
  _topoSpace(&_topoScrollArea),
  _topoLayout(&_topoSpace),
  _topoWidgets({}),
// END of change
  _scrollArea(this),
  _glWindow(&_scrollArea),
  _mState(nullptr),
  _data(nullptr),
  _keyPressed(false)
{
	auto lamMakeNavButton = [&](QPushButton *b) {
		b->setMaximumWidth(FONT_WIDTH * 5);

		connect(b,	&QPushButton::released,
			this,	&KsTraceGraph::_stopUpdating);
		_navigationBar.addWidget(b);
	};

	_pointerBar.setMaximumHeight(FONT_HEIGHT * 1.75);
	_pointerBar.setOrientation(Qt::Horizontal);

	_navigationBar.setMaximumHeight(FONT_HEIGHT * 1.75);
	_navigationBar.setMinimumWidth(FONT_WIDTH * 90);
	_navigationBar.setOrientation(Qt::Horizontal);

	_pointerBar.addWidget(&_labelP1);
	_labelP2.setFrameStyle(QFrame::Panel | QFrame::Sunken);
	_labelP2.setStyleSheet("QLabel {background-color : white; color: black}");
	_labelP2.setTextInteractionFlags(Qt::TextSelectableByMouse);
	_labelP2.setFixedWidth(FONT_WIDTH * 16);
	_pointerBar.addWidget(&_labelP2);
	_pointerBar.addSeparator();

	_labelI1.setStyleSheet("QLabel {color : blue;}");
	_labelI2.setStyleSheet("QLabel {color : green;}");
	_labelI3.setStyleSheet("QLabel {color : red;}");
	_labelI4.setStyleSheet("QLabel {color : blue;}");
	_labelI5.setStyleSheet("QLabel {color : green;}");

	_pointerBar.addWidget(&_labelI1);
	_pointerBar.addSeparator();
	_pointerBar.addWidget(&_labelI2);
	_pointerBar.addSeparator();
	_pointerBar.addWidget(&_labelI3);
	_pointerBar.addSeparator();
	_pointerBar.addWidget(&_labelI4);
	_pointerBar.addSeparator();
	_pointerBar.addWidget(&_labelI5);

	_glWindow.installEventFilter(this);

	connect(&_glWindow,	&KsGLWidget::select,
		this,		&KsTraceGraph::markEntry);

	connect(&_glWindow,	&KsGLWidget::found,
		this,		&KsTraceGraph::_setPointerInfo);

	connect(&_glWindow,	&KsGLWidget::notFound,
		this,		&KsTraceGraph::_resetPointer);

	connect(&_glWindow,	&KsGLWidget::zoomIn,
		this,		&KsTraceGraph::_zoomIn);

	connect(&_glWindow,	&KsGLWidget::zoomOut,
		this,		&KsTraceGraph::_zoomOut);

	connect(&_glWindow,	&KsGLWidget::scrollLeft,
		this,		&KsTraceGraph::_scrollLeft);

	connect(&_glWindow,	&KsGLWidget::scrollRight,
		this,		&KsTraceGraph::_scrollRight);

	connect(&_glWindow,	&KsGLWidget::stopUpdating,
		this,		&KsTraceGraph::_stopUpdating);

	_glWindow.setContextMenuPolicy(Qt::CustomContextMenu);
	connect(&_glWindow,	&QWidget::customContextMenuRequested,
		this,		&KsTraceGraph::_onCustomContextMenu);

	//NOTE: Changed here. (NUMA TV) (2025-04-12)
	// To not bloat this constructor definition as it is
	// already long, NUMA TV-related construction is moved
	// to a separate setup function :)
	_setupNumatvTopoWidget();
	// END of change
	
	_scrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_scrollArea.setWidget(&_glWindow);

	lamMakeNavButton(&_scrollLeftButton);
	connect(&_scrollLeftButton,	&QPushButton::pressed,
		this,			&KsTraceGraph::_scrollLeft);

	lamMakeNavButton(&_zoomInButton);
	connect(&_zoomInButton,		&QPushButton::pressed,
		this,			&KsTraceGraph::_zoomIn);

	lamMakeNavButton(&_zoomOutButton);
	connect(&_zoomOutButton,	&QPushButton::pressed,
		this,			&KsTraceGraph::_zoomOut);

	lamMakeNavButton(&_scrollRightButton);
	connect(&_scrollRightButton,	&QPushButton::pressed,
		this,			&KsTraceGraph::_scrollRight);

	_navigationBar.addSeparator();

	lamMakeNavButton(&_quickZoomInButton);
	connect(&_quickZoomInButton,	&QPushButton::pressed,
		this,			&KsTraceGraph::_quickZoomIn);

	lamMakeNavButton(&_quickZoomOutButton);
	connect(&_quickZoomOutButton,	&QPushButton::pressed,
		this,			&KsTraceGraph::_quickZoomOut);

	_layout.addWidget(&_pointerBar);
	_layout.addWidget(&_navigationBar);
	//NOTE: Changed here. (NUMA TV) (2025-04-19)
	// To preserve the original layout, we add the topology widget
	// now wrapping both the GL widget and the topology widget.
	_layout.addWidget(&_topoGlWrapper);
	// END of change
	this->setLayout(&_layout);
	updateGeom();
}

//NOTE: Changed here. (NUMA TV) (2025-04-21)
/**
 * @brief Wrapper for construction of NUMA TV-related widgets and
 * layouts in the constructor.
 */
void KsTraceGraph::_setupNumatvTopoWidget() {
	_topoGlWrapper.setContentsMargins(0, 0, 0, 0);
	_topoGlWrapper.setLayout(&_topoGlLayout);

	_topoGlLayout.setContentsMargins(0, 0, 0, 0);
	_topoGlLayout.setSpacing(0);
	_topoGlLayout.addWidget(&_hideTopoBtn);
	_topoGlLayout.addWidget(&_topoScrollArea);
	_topoGlLayout.addWidget(&_scrollArea);

	_topoScrollArea.setContentsMargins(0, 0, 0, 0);
	_topoScrollArea.setWidgetResizable(true);
	_topoScrollArea.setHidden(true);
	_topoScrollArea.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_topoScrollArea.setWidget(&_topoSpace);

	_hideTopoBtn.setFixedWidth(FONT_WIDTH * 3);
	_hideTopoBtn.setFixedHeight(_topoScrollArea.height());
	_hideTopoBtn.setHidden(true);
	_hideTopoBtn.setStyleSheet("QPushButton {background-color : green;}");

	_topoSpace.setStyleSheet("QWidget {background-color : white;}");
	_topoSpace.setLayout(&_topoLayout);
	
	int v_margin = _glWindow.vMargin();
	int v_spacing = _glWindow.vSpacing();
	_topoLayout.setAlignment(Qt::AlignTop);
	// Interestingly the bottom margin is actually just a
	// the spacing between graphs.
	_topoLayout.setContentsMargins(0, 2*v_margin, 0, v_spacing);
	_topoLayout.setSpacing(v_spacing);

	// Connections
	connect(&_hideTopoBtn, &QPushButton::pressed,
		this,	[this]() {
			if (_topoScrollArea.isHidden()) {
				_topoScrollArea.show();
				_hideTopoBtn.setText("<");
			} else {
				_topoScrollArea.hide();
				_topoScrollArea.setFixedWidth(0);
				_hideTopoBtn.setText(">");
			}
			updateGeom();
		}
	);
	
	connect(_scrollArea.verticalScrollBar(), &QScrollBar::valueChanged,
		_topoScrollArea.verticalScrollBar(), &QScrollBar::setValue);
}
// END of change

/**
 * @brief Load and show trace data.
 *
 * @param data: Input location for the KsDataStore object.
 *	  	KsDataStore::loadDataFile() must be called first.
 * @param resetPlots: If true, all existing graphs are closed
 *		      and a default configuration of graphs is displayed
 *		      (all CPU plots). If false, the current set of graphs
 *		      is preserved.
 */
void KsTraceGraph::loadData(KsDataStore *data, bool resetPlots)
{
	_data = data;
	_glWindow.loadData(data, resetPlots);
	updateGeom();
}

/** Connect the KsGLWidget widget and the State machine of the Dual marker. */
void KsTraceGraph::setMarkerSM(KsDualMarkerSM *m)
{
	_mState = m;
	_navigationBar.addSeparator();
	_mState->placeInToolBar(&_navigationBar);
	_glWindow.setMarkerSM(m);
}

/** Reset (empty) the widget. */
void KsTraceGraph::reset()
{
	/* Reset (empty) the OpenGL widget. */
	_glWindow.reset();

	_labelP2.setText("");
	for (auto l1: {&_labelI1, &_labelI2, &_labelI3, &_labelI4, &_labelI5})
		l1->setText("");

	_selfUpdate();
}

void KsTraceGraph::_selfUpdate()
{
	_markerReDraw();
	_glWindow.model()->update();
	updateGeom();
}

void KsTraceGraph::_zoomIn()
{
	KsWidgetsLib::KsDataWork action = KsWidgetsLib::KsDataWork::ZoomIn;

	startOfWork(action);
	_updateGraphs(action);
	endOfWork(action);
}

void KsTraceGraph::_zoomOut()
{
	KsWidgetsLib::KsDataWork action = KsWidgetsLib::KsDataWork::ZoomOut;

	startOfWork(action);
	_updateGraphs(action);
	endOfWork(action);
}

void KsTraceGraph::_quickZoomIn()
{
	if (_glWindow.isEmpty())
		return;

	startOfWork(KsWidgetsLib::KsDataWork::QuickZoomIn);

	/* Bin size will be 100 ns. */
	_glWindow.model()->quickZoomIn(100);
	if (_mState->activeMarker()._isSet &&
	    _mState->activeMarker().isVisible()) {
		/*
		 * Use the position of the active marker as
		 * a focus point of the zoom.
		 */
		uint64_t ts = _mState->activeMarker()._ts;
		_glWindow.model()->jumpTo(ts);
		_glWindow.render();
	}

	endOfWork(KsWidgetsLib::KsDataWork::QuickZoomIn);
}

void KsTraceGraph::_quickZoomOut()
{
	if (_glWindow.isEmpty())
		return;

	startOfWork(KsWidgetsLib::KsDataWork::QuickZoomOut);
	_glWindow.model()->quickZoomOut();
	_glWindow.render();
	endOfWork(KsWidgetsLib::KsDataWork::QuickZoomOut);
}

void KsTraceGraph::_scrollLeft()
{
	KsWidgetsLib::KsDataWork action = KsWidgetsLib::KsDataWork::ScrollLeft;

	startOfWork(action);
	_updateGraphs(action);
	endOfWork(action);
}

void KsTraceGraph::_scrollRight()
{
	KsWidgetsLib::KsDataWork action = KsWidgetsLib::KsDataWork::ScrollRight;

	startOfWork(action);
	_updateGraphs(action);
	endOfWork(action);
}

void KsTraceGraph::_stopUpdating()
{
	/*
	 * The user is no longer pressing the action button. Reset the
	 * "Key Pressed" flag. This will stop the ongoing user action.
	 */
	_keyPressed = false;
}

QString KsTraceGraph::_t2str(uint64_t sec, uint64_t usec) {
	QString usecStr;
	QTextStream ts(&usecStr);

	ts.setFieldAlignment(QTextStream::AlignRight);
	ts.setFieldWidth(6);
	ts.setPadChar('0');

	ts << usec;

	return QString::number(sec) + "." + usecStr;
}

void KsTraceGraph::_resetPointer(int64_t ts, int sd, int cpu, int pid)
{
	kshark_entry entry;
	uint64_t sec, usec;

	entry.cpu = cpu;
	entry.pid = pid;
	entry.stream_id = sd;

	if (ts < 0)
		ts = 0;

	kshark_convert_nano(ts, &sec, &usec);
	_labelP2.setText(_t2str(sec, usec));

	if (pid > 0 && cpu >= 0) {
		struct kshark_context *kshark_ctx(NULL);

		if (!kshark_instance(&kshark_ctx))
			return;

		QString comm(kshark_get_task(&entry));
		comm.append("-");
		comm.append(QString("%1").arg(pid));
		_labelI1.setText(comm);
		_labelI2.setText(QString("CPU %1").arg(cpu));
	} else {
		_labelI1.setText("");
		_labelI2.setText("");
	}

	for (auto const &l: {&_labelI3, &_labelI4, &_labelI5}) {
		l->setText("");
	}
}

void KsTraceGraph::_setPointerInfo(size_t i)
{
	kshark_entry *e = _data->rows()[i];
	auto lanMakeString = [] (char *buffer) {
		QString str(buffer);
		free(buffer);
		return str;
	};

	QString event(lanMakeString(kshark_get_event_name(e)));
	QString aux(lanMakeString(kshark_get_aux_info(e)));
	QString info(lanMakeString(kshark_get_info(e)));
	QString comm(lanMakeString(kshark_get_task(e)));
	int labelWidth;
	uint64_t sec, usec;
	char *pointer;

	kshark_convert_nano(e->ts, &sec, &usec);
	labelWidth = asprintf(&pointer, "%" PRIu64 ".%06" PRIu64 "", sec, usec);
	if (labelWidth <= 0)
		return;

	_labelP2.setText(pointer);
	free(pointer);

	comm.append("-");
	comm.append(QString("%1").arg(kshark_get_pid(e)));

	_labelI1.setText(comm);
	_labelI2.setText(QString("CPU %1").arg(e->cpu));
	_labelI3.setText(aux);
	_labelI4.setText(event);
	_labelI5.setText(info);
	QCoreApplication::processEvents();

	labelWidth =
		_pointerBar.geometry().right() - _labelI4.geometry().right();
	if (labelWidth > STRING_WIDTH(info) + FONT_WIDTH * 5)
		return;

	/*
	 * The Info string is too long and cannot be displayed on the toolbar.
	 * Try to fit the text in the available space.
	 */
	KsUtils::setElidedText(&_labelI5, info, Qt::ElideRight, labelWidth);
	_labelI5.setVisible(true);
	QCoreApplication::processEvents();
}

/**
 * @brief Use the active marker to select particular entry.
 *
 * @param row: The index of the entry to be selected by the marker.
 */
void KsTraceGraph::markEntry(size_t row)
{
	int yPosVis(-1);

	_glWindow.model()->jumpTo(_data->rows()[row]->ts);
	_mState->activeMarker().set(*_data, _glWindow.model()->histo(),
				    row, _data->rows()[row]->stream_id);

	_mState->updateMarkers(*_data, &_glWindow);

	/*
	 * If a Combo graph has been found, this Combo graph will be visible.
	 * Else the Task graph will be shown. If no Combo and no Task graph
	 * has been found, make visible the corresponding CPU graph.
	 */
	if (_mState->activeMarker()._mark.comboIsVisible())
		yPosVis = _mState->activeMarker()._mark.comboY();
	else if (_mState->activeMarker()._mark.taskIsVisible())
		yPosVis = _mState->activeMarker()._mark.taskY();
	else if (_mState->activeMarker()._mark.cpuIsVisible())
		yPosVis = _mState->activeMarker()._mark.cpuY();

	if (yPosVis > 0)
		_scrollArea.ensureVisible(0, yPosVis);
}

void KsTraceGraph::_markerReDraw()
{
	size_t row;

	if (_mState->markerA()._isSet) {
		row = _mState->markerA()._pos;
		_mState->markerA().set(*_data, _glWindow.model()->histo(),
				       row, _data->rows()[row]->stream_id);
	}

	if (_mState->markerB()._isSet) {
		row = _mState->markerB()._pos;
		_mState->markerB().set(*_data, _glWindow.model()->histo(),
				       row, _data->rows()[row]->stream_id);
	}
}

// Function was renamed to match the new API and functionality.
/**
 * @brief Redraw all CPU graphs & redraw topology widgets.
 *
 * @param sd: Data stream identifier.
 * @param v: CPU ids to be plotted.
 */
void KsTraceGraph::cpuReDraw(int sd, QVector<int> v)
{
	startOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
	if (_glWindow._streamPlots.contains(sd)) {
		//NOTE: Changed here. (NUMA TV) (2025-04-18)
		// CPUs being redrawn goes hand in hand with redrawing
		// the topology widget, as CPUs may need to be reordered,
		// or some CPUs were hidden and the topology wiget must adjust
		// its own parts.
		_numatvRedrawTopoWidgets(sd, v);
		// END of change
		_glWindow._streamPlots[sd]._cpuList = v;
	}

	_selfUpdate();
	endOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
}

/**
 * @brief Redreaw all Task graphs.
 *
 * @param sd: Data stream identifier.
 * @param v: Process ids of the tasks to be plotted.
 */
void KsTraceGraph::taskReDraw(int sd, QVector<int> v)
{
	startOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
	if (_glWindow._streamPlots.contains(sd)) {
		_glWindow._streamPlots[sd]._taskList = v;
		//NOTE: Changed here. (NUMA TV) (2025-04-18)
		// Task redraw means that the topology widget needs padding
		// at the bottom, so that the topology widget of another stream
		// below does not overlap with the task graph of the current stream.
		_numatvAdjustTopoTaskPadding(sd);
		// END of change
	}

	_selfUpdate();
	endOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
}

/**
 * @brief Redreaw all virtCombo graphs.
 *
 * @param nCombos: Numver of Combo plots.
 * @param v: Descriptor of the Combo to be plotted.
 */
void KsTraceGraph::comboReDraw(int nCombos, QVector<int> v)
{
	KsComboPlot combo;

	startOfWork(KsWidgetsLib::KsDataWork::EditPlotList);

	_glWindow._comboPlots.clear();

	for (int i = 0; i < nCombos; ++i) {
		combo.resize(v.takeFirst());
		for (auto &p: combo)
			p << v;

		_glWindow._comboPlots.append(combo);
	}

	_selfUpdate();
	endOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
}

/** Add (and plot) a CPU graph to the existing list of CPU graphs. */
void KsTraceGraph::addCPUPlot(int sd, int cpu)
{
	startOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
	QVector<int> &list = _glWindow._streamPlots[sd]._cpuList;
	if (list.contains(cpu))
		return;

	list.append(cpu);

	std::sort(list.begin(), list.end());

	_selfUpdate();
	endOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
}

/** Add (and plot) a Task graph to the existing list of Task graphs. */
void KsTraceGraph::addTaskPlot(int sd, int pid)
{
	startOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
	QVector<int> &list = _glWindow._streamPlots[sd]._taskList;
	if (list.contains(pid))
		return;

	list.append(pid);
	std::sort(list.begin(), list.end());

	_selfUpdate();
	endOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
}

/** Remove a CPU graph from the existing list of CPU graphs. */
void KsTraceGraph::removeCPUPlot(int sd, int cpu)
{
	startOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
	if (!_glWindow._streamPlots[sd]._cpuList.contains(cpu))
		return;

	_glWindow._streamPlots[sd]._cpuList.removeAll(cpu);
	_selfUpdate();
	endOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
}

/** Remove a Task graph from the existing list of Task graphs. */
void KsTraceGraph::removeTaskPlot(int sd, int pid)
{
	startOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
	if (!_glWindow._streamPlots[sd]._taskList.contains(pid))
		return;

	_glWindow._streamPlots[sd]._taskList.removeAll(pid);
	_selfUpdate();
	endOfWork(KsWidgetsLib::KsDataWork::EditPlotList);
}

/** Update the content of all graphs. */
void KsTraceGraph::update(KsDataStore *data)
{
	kshark_context *kshark_ctx(nullptr);
	QVector<int> streamIds;

	if (!kshark_instance(&kshark_ctx))
		return;

	streamIds = KsUtils::getStreamIdList(kshark_ctx);
	for (auto const &sd: streamIds)
		for (auto &pid: _glWindow._streamPlots[sd]._taskList) {
			kshark_unregister_data_collection(&kshark_ctx->collections,
							  kshark_match_pid,
							  sd, &pid, 1);
		}

	_selfUpdate();

	streamIds = KsUtils::getStreamIdList(kshark_ctx);
	for (auto const &sd: streamIds)
		for (auto &pid: _glWindow._streamPlots[sd]._taskList) {
			kshark_register_data_collection(kshark_ctx,
							data->rows(),
							data->size(),
							kshark_match_pid,
							sd, &pid, 1,
							25);
		}
}

/** Update the geometry of the widget. */
void KsTraceGraph::updateGeom()
{
	int saWidth, saHeight, dwWidth, hMin;

	/* Set the size of the Scroll Area. */
	saWidth = width() - _layout.contentsMargins().left() -
				_layout.contentsMargins().right();

	saHeight = height() - _pointerBar.height() -
			      _navigationBar.height() -
			      _layout.spacing() * 2 -
			      _layout.contentsMargins().top() -
			      _layout.contentsMargins().bottom();

	//NOTE: Changed here. (NUMA TV) (2025-04-16)
	// Resize the wrapper for wrapper topology widget and the GL widget
	// to be the size of the GL scroll area if NUMA TV was disabled.
	_topoGlWrapper.resize(saWidth, saHeight);
	// END of change

	//NOTE: Changed here. (NUMA TV) (2025-04-15)
	// If the hide button is shown, set the width of the scroll area
	// to be the width of the scroll area minus the width of the
	// hide button, to allow both to be shown.
	int hideBtnCorrection = _hideTopoBtn.isHidden() ?
		0 : _hideTopoBtn.width();
	// END of change

	//NOTE: Changed here. (NUMA TV) (2025-04-15)
	// Shorten the width of the scroll area by the width of the
	// hide button, if it is shown.
	saWidth -= hideBtnCorrection;
	// END of change

	//NOTE: Changed here. (NUMA TV) (2025-04-15)
	// If the wrapper topology widget is NOT hidden, set it to be a fifth
	// of the width of the scroll area otherwise shown. Make the scroll
	// area to be 4/5 of the width of itself to make space for the
	// topology scroll area as well.
	if (!_topoScrollArea.isHidden()) {
		_topoScrollArea.setFixedWidth(saWidth / 5);
		saWidth -= _topoScrollArea.width();
	}
	// END of change

	_scrollArea.resize(saWidth, saHeight);
	//NOTE: Changed here. (NUMA TV) (2025-04-15)
	// Synchronize the height of the hide button and the scroll area
	// with the other GL widget's scroll area.
	_hideTopoBtn.setFixedHeight(std::max(saHeight, 0));
	_topoScrollArea.setFixedHeight(std::max(saHeight, 0));
	// END of change

	/*
	 * Calculate the width of the Draw Window, taking into account the size
	 * of the scroll bar.
	 */
	dwWidth = _scrollArea.width();
	if (_glWindow.height() > _scrollArea.height())
		dwWidth -=
			qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

	_glWindow.resize(dwWidth, _glWindow.height());
	//NOTE: Changed here. (NUMA TV) (2025-04-15)
	// Synchronize the height of the wrapper topology widget with the height of the
	// GL widget.
	_topoSpace.setFixedHeight(std::max(0, _glWindow.height()));
	// END of change

	/* Set the minimum height of the Graph widget. */
	hMin = _glWindow.height() +
	       _pointerBar.height() +
	       _navigationBar.height() +
	       _layout.contentsMargins().top() +
	       _layout.contentsMargins().bottom();

	if (hMin > KS_GRAPH_HEIGHT * 6)
		hMin = KS_GRAPH_HEIGHT * 6;

	setMinimumHeight(hMin);

	/*
	 * Now use the height of the Draw Window to fix the maximum height
	 * of the Graph widget.
	 */
	setMaximumHeight(_glWindow.height() +
			 _pointerBar.height() +
			 _navigationBar.height() +
			 _layout.spacing() * 2 +
			 _layout.contentsMargins().top() +
			 _layout.contentsMargins().bottom() +
			 2);  /* Just a little bit of extra space. This will
			       * allow the scroll bar to disappear when the
			       * widget is extended to maximum.
			       */

	_glWindow.updateGeom();
}

/**
 * Reimplemented event handler used to update the geometry of the widget on
 * resize events.
 */
void KsTraceGraph::resizeEvent([[maybe_unused]] QResizeEvent* event)
{
	updateGeom();
}

/**
 * Reimplemented event handler (overriding a virtual function from QObject)
 * used to detect the position of the mouse with respect to the Draw window and
 * according to this position to grab / release the focus of the keyboard. The
 * function has nothing to do with the filtering of the trace events.
 */
bool KsTraceGraph::eventFilter(QObject* obj, QEvent* evt)
{
	/* Desable all mouse events for the OpenGL wiget when busy. */
	if (obj == &_glWindow && this->isBusy() &&
	    (evt->type() == QEvent::MouseButtonDblClick ||
	     evt->type() == QEvent::MouseButtonPress ||
	     evt->type() == QEvent::MouseButtonRelease ||
	     evt->type() == QEvent::MouseMove)
	)
		return true;

	if (obj == &_glWindow && evt->type() == QEvent::Enter)
		_glWindow.setFocus();

	if (obj == &_glWindow && evt->type() == QEvent::Leave)
		_glWindow.clearFocus();

	return QWidget::eventFilter(obj, evt);
}

void KsTraceGraph::_updateGraphs(KsWidgetsLib::KsDataWork action)
{
	double k;
	int bin;

	if (_glWindow.isEmpty())
		return;

	/*
	 * Set the "Key Pressed" flag. The flag will stay set as long as the user
	 * keeps the corresponding action button pressed.
	 */
	_keyPressed = true;

	/* Initialize the zooming factor with a small value. */
	k = .01;
	while (_keyPressed) {
		switch (action) {
		case KsWidgetsLib::KsDataWork::ZoomIn:
			if (_mState->activeMarker()._isSet &&
			    _mState->activeMarker().isVisible()) {
				/*
				 * Use the position of the active marker as
				 * a focus point of the zoom.
				 */
				bin = _mState->activeMarker()._bin;
				_glWindow.model()->zoomIn(k, bin);
			} else {
				/*
				 * The default focus point is the center of the
				 * range interval of the model.
				 */
				_glWindow.model()->zoomIn(k);
			}

			break;

		case KsWidgetsLib::KsDataWork::ZoomOut:
			if (_mState->activeMarker()._isSet &&
			    _mState->activeMarker().isVisible()) {
				/*
				 * Use the position of the active marker as
				 * a focus point of the zoom.
				 */
				bin = _mState->activeMarker()._bin;
				_glWindow.model()->zoomOut(k, bin);
			} else {
				/*
				 * The default focus point is the center of the
				 * range interval of the model.
				 */
				_glWindow.model()->zoomOut(k);
			}

			break;

		case KsWidgetsLib::KsDataWork::ScrollLeft:
			_glWindow.model()->shiftBackward(10);
			break;

		case KsWidgetsLib::KsDataWork::ScrollRight:
			_glWindow.model()->shiftForward(10);
			break;

		default:
			return;
		}

		/*
		 * As long as the action button is pressed, the zooming factor
		 * will grow smoothly until it reaches a maximum value. This
		 * will have a visible effect of an accelerating zoom.
		 */
		if (k < .25)
			k  *= 1.02;

		_mState->updateMarkers(*_data, &_glWindow);
		_glWindow.render();
		QCoreApplication::processEvents();
	}
}

void KsTraceGraph::_onCustomContextMenu(const QPoint &point)
{
	KsQuickMarkerMenu *menu(nullptr);
	int sd, cpu, pid;
	size_t row;
	bool found;

	found = _glWindow.find(point, 20, true, &row);
	if (found) {
		/* KernelShark entry has been found under the cursor. */
		KsQuickContextMenu *entryMenu;
		menu = entryMenu = new KsQuickContextMenu(_mState, _data, row,
							  this);

		connect(entryMenu,	&KsQuickContextMenu::addTaskPlot,
			this,		&KsTraceGraph::addTaskPlot);

		connect(entryMenu,	&KsQuickContextMenu::addCPUPlot,
			this,		&KsTraceGraph::addCPUPlot);

		connect(entryMenu,	&KsQuickContextMenu::removeTaskPlot,
			this,		&KsTraceGraph::removeTaskPlot);

		connect(entryMenu,	&KsQuickContextMenu::removeCPUPlot,
			this,		&KsTraceGraph::removeCPUPlot);
	} else {
		if (!_glWindow.getPlotInfo(point, &sd, &cpu, &pid))
			return;

		if (cpu >= 0) {
			/*
			 * This is a CPU plot, but we do not have an entry
			 * under the cursor.
			 */
			KsRmCPUPlotMenu *rmMenu;
			menu = rmMenu = new KsRmCPUPlotMenu(_mState, sd, cpu, this);

			auto lamRmPlot = [&sd, &cpu, this] () {
				removeCPUPlot(sd, cpu);
			};

			connect(rmMenu, &KsRmPlotContextMenu::removePlot,
				lamRmPlot);
		}

		if (pid >= 0) {
			/*
			 * This is a Task plot, but we do not have an entry
			 * under the cursor.
			 */
			KsRmTaskPlotMenu *rmMenu;
			menu = rmMenu = new KsRmTaskPlotMenu(_mState, sd, pid, this);

			auto lamRmPlot = [&sd, &pid, this] () {
				removeTaskPlot(sd, pid);
			};

			connect(rmMenu, &KsRmPlotContextMenu::removePlot,
				lamRmPlot);
		}
	}

	if (menu) {
		connect(menu,	&KsQuickMarkerMenu::deselect,
			this,	&KsTraceGraph::deselect);

		/*
		 * Note that this slot was connected to the
		 * customContextMenuRequested signal of the OpenGL widget.
		 * Because of this the coordinates of the point are given with
		 * respect to the frame of this widget.
		 */
		QPoint global = _glWindow.mapToGlobal(point);
		global.ry() -= menu->sizeHint().height() / 2;

		/*
		 * Shift the menu so that it is not positioned under the mouse.
		 * This will prevent from an accidental selection of the menu
		 * item under the mouse.
		 */
		global.rx() += FONT_WIDTH;

		menu->exec(global);
	}
}

//NOTE: Changed here. (PREVIEW LABELS CHANGEABLE) (2024-07-26)
/**
 * @brief Set the labels of the preview area. By default, each
 * argument is an empty string.
 * 
 * @param label1 Text to be put into the first label (leftmost).
 * @param label2 Text to be put into the second label.
 * @param label3 Text to be put into the third label.
 * @param label4 Text to be put into the fourth label.
 * @param label5 Text to be put into the fifth label (rightmost).
 */
void KsTraceGraph::setPreviewLabels(const QString& label1,
									const QString& label2,
									const QString& label3,
									const QString& label4,
									const QString& label5) {
	// It is really that simple
	_labelI1.setText(label1);
	_labelI2.setText(label2);
	_labelI3.setText(label3);
	_labelI4.setText(label4);
	_labelI5.setText(label5);
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-20)
/**
 * @brief Clears a QLayout by removing all items from it. This function
 * is used to clear the topology layout before reinserting items into it
 * for it to be sorted.
 * 
 * If the item to be removed is a widget, it is detached from the layout,
 * but not deleted. This is to avoid deleting the widget while it is
 * still in use. Other items are deleted.
 * 
 * @param layout The layout to be cleared.
 */
static void clear_topo_layout(QLayout* layout) {
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (QWidget* topo_widget = item->widget()) {
            topo_widget->setParent(nullptr);  // Detach but don't delete
        }
        delete item; // Delete other things
    }
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-15)
/**
 * @brief Hides or shows the wrapper topology widget (specifically its owner,
 * the scroll area) and hide button.
 * 
 * @param hide Whether to hide the wrapper topology widget and hide button.
 */
void KsTraceGraph::numatvHideTopologyWidget(bool hide) {
	_hideTopoBtn.setHidden(hide);
	_topoScrollArea.setHidden(hide);
	updateGeom();
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-17)
/**
 * @brief Removes and deletes all topology widgets from the trace graph.
 */
void KsTraceGraph::numatvClearTopologyWidgets() {
	for (auto&& [stream_id, widget] : _topoWidgets) {
		if (widget) {
			delete widget;
			widget = nullptr;
		}
	}
	_topoWidgets.clear();
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-17)
/**
 * @brief Inserts a topology widget for a stream into the trace graph's
 * topology map. If a topology widget already exists for the stream, it is
 * removed first.
 * 
 * @param sd Identifier of the stream.
 * @param brief_topo Brief topology of the stream from which the stream's
 * topology widget is created.
 */
void KsTraceGraph::_numatvInsertTopologyWidget(int sd, const TopoNodeCorePU& brief_topo)
{
	bool existsFor_stream = _topoWidgets.count(sd);
	if (existsFor_stream) {
		// Remove existing topology widget
		_numatvRemoveTopologyWidget(sd);
	}

	auto new_widget = new KsStreamNUMATopology{sd, brief_topo, this, &_topoSpace};

	_topoWidgets[sd] = new_widget;
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-17)
/**
 * @brief Removes and deletes a topology widget of a stream.
 * 
 * @param stream_id Identifier of the stream.
 */
void KsTraceGraph::_numatvRemoveTopologyWidget(int stream_id) {
	KsStreamNUMATopology* topoWidget = _topoWidgets[stream_id];
	delete topoWidget;
	_topoWidgets.erase(stream_id);
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-18)
/**
 * @brief Hides or shows a topology widget of a stream.
 * 
 * @param stream_id Identifier of the stream.
 * @param hide Whether to hide or show the topology widget.
 */
void KsTraceGraph::_numatvHideStreamTopo(int stream_id, bool hide)
{ _topoWidgets[stream_id]->hideTopology(hide); }
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-18)
/**
 * @brief Action to be taken when a topology widget already exists.
 * If the view requested is a NUMA tree, topology tree is created from
 * the brief topology in the stream's configuration. On default view,
 * the topology widget is still created, but the tree is empty and the
 * topology tree's widget is hidden to be extra safe it won't show up.
 * 
 * @param stream_id Identifier of the stream.
 * @param cpusToDraw CPUs to be drawn in the topology widget.
 * The list may be rearranged to match the topology tree.
 */
void KsTraceGraph::_numatvExistingTopologyAction(int stream_id,
	QVector<int>& cpusToDraw)
{
	auto stream_cfg = _numaTvCtx.observeConfig(stream_id);
	TopoViewType stream_view = stream_cfg->getViewType();
	TopoNodeCorePU brief_topo = {};
	bool hide_topo = true;
	
	if (stream_view == TopoViewType::NUMATREE) {
		brief_topo = stream_cfg->getBriefTopology();
		brief_topo = numatv_filter_by_PUs(brief_topo, cpusToDraw);
		cpusToDraw = stream_cfg->rearrangeCPUsWithBriefTopo(cpusToDraw,
			brief_topo);
		hide_topo = cpusToDraw.isEmpty();
	}

	_numatvInsertTopologyWidget(stream_id, brief_topo);
	_numatvHideStreamTopo(stream_id, hide_topo);
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-18)
/**
 * @brief Redraws the topology widgets for the given stream ID.
 * During this, the layout housing all stream topology widgets is
 * refilled with items to keep them sorted.
 * 
 * @param stream_id Identifier of the stream.
 * @param cpusToDraw CPUs to be drawn in the stream's topology widgets.
 * The list may be rearranged to match the topology tree.
 */
void KsTraceGraph::_numatvRedrawTopoWidgets(int stream_id,
	QVector<int>& cpusToDraw)
{
	bool topology_exists = _numaTvCtx.existsFor(stream_id);

	if (topology_exists) {
		_numatvExistingTopologyAction(stream_id,
			cpusToDraw);
	} else {
		_numatvInsertTopologyWidget(stream_id, {});
		_numatvHideStreamTopo(stream_id, true);
	}

	clear_topo_layout(&_topoLayout);

	for (auto&& [sd, topo_widget] : _topoWidgets) {
		topo_widget->setParent(&_topoSpace);
		_topoLayout.addWidget(topo_widget);
	}
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-20)
/**
 * @brief Resizes stream's topology widget to fit the number of graphs
 * the stream has in the GL widget.
 * 
 * @param stream_id Identifier of the stream.
 */
void KsTraceGraph::_numatvAdjustTopoTaskPadding(int stream_id) {
	if (_topoWidgets.count(stream_id)) {
		KsStreamNUMATopology* topo_widget = _topoWidgets[stream_id];
		int v_spacing = _glWindow.vSpacing();
		int all_graphs = _glWindow.graphCount(stream_id);
		int all_graphs_height = KS_GRAPH_HEIGHT * all_graphs;
		int all_graphs_spacings = (all_graphs - 1) * v_spacing;
		int new_height = all_graphs_height + all_graphs_spacings;

		topo_widget->resizeTopologyWidget(new_height);
	}
}
// END of change

// KsStreamNUMATopology

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
  _machine(&_topo),
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

	_topoLayout.addWidget(&_machine);
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
	int v_spacing, const TopoPUIds& PUs, const KsGLWidget* gl_widget,
	QLabel* node_parent, unsigned int& node_reds, unsigned int& node_greens,
	unsigned int& node_blues)
{
	QLabel* core = new QLabel(node_parent);
	core->setText(QString("(NN %1) C %2").arg(node_lid).arg(core_lid));
	core->setToolTip(QString{"Core %1 in NUMA Node %2"}
		.arg(core_lid).arg(node_lid));
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
 * 
 * @param node_lid Logical index of the node in the topology.
 * @param v_spacing Spacing between the graphs in KernelShark's GL widget.
 * @param cores Collection of cores in the node.
 * @param gl_widget Pointer to the GL widget from which to get
 * CPU colors.
 * @return Height of the created Node node. 
 */
int KsStreamNUMATopology::_setupTopologyTreeNode(int node_lid, int v_spacing,
	const TopoCorePU& cores, const KsGLWidget* gl_widget)
{
	
	QLabel* node = new QLabel(&_nodes);
	node->setText(QString{"NN %1"}.arg(node_lid));
	node->setAlignment(Qt::AlignCenter);
	node->setToolTip(QString{"NUMA Node %1"}.arg(node_lid));
	_nodesLayout.addWidget(node);

	int node_height = 0;
	unsigned int core_reds = 0;
	unsigned int core_greens = 0;
	unsigned int core_blues = 0;

	for (const auto& [core_lid, PUs] : cores) {
		int core_height = _setupTopologyTreeCore(core_lid,
			node_lid, v_spacing, PUs, gl_widget, node, core_reds,
			core_greens, core_blues);
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
	
	return node_height;
}
// END of change

//NOTE: Changed here. (NUMA TV) (2025-04-19)
/**
 * @brief Sets up the topology tree - the machine node (tree root), its
 * color, its text, its tooltip and also the nodes under it. Also sets
 * a fixed height for the topology tree's owning widget, the KsStreamNUMATopology
 * object.
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
	QString machine_name = QString("M %1").arg(stream_id);
	_machine.setText(machine_name);
	_machine.setToolTip(QString{"Machine (stream) %1"}.arg(stream_id));
	_machine.setAlignment(Qt::AlignCenter);
	_machine.setWordWrap(true);
	
	const KsPlot::ColorTable& stream_cols = gl_widget->getStreamColors();
	const KsPlot::Color& stream_color = stream_cols.at(stream_id);
	QString machine_ssheet = make_topo_item_stylesheet(stream_color);
	_machine.setStyleSheet(machine_ssheet);
	
	int machine_height = 0;

	for (const auto& [node_lid, cores] : brief_topo) {
		int node_height = _setupTopologyTreeNode(node_lid, v_spacing,
			cores, gl_widget); 
		machine_height += node_height + v_spacing;
	}

	machine_height -= v_spacing; // One less node spacing
	_machine.setFixedHeight(std::max(machine_height, 0));

	int all_graphs = gl_widget->graphCount(stream_id);
	int all_graphs_height = KS_GRAPH_HEIGHT * all_graphs;
	int all_graphs_spacings = (all_graphs - 1) * v_spacing;
	int machine_height_or_gl_height = std::max(machine_height, 
		all_graphs_height + all_graphs_spacings);
	resizeTopologyWidget(machine_height_or_gl_height);
}
// END of change
