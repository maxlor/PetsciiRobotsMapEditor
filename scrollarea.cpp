#include "scrollarea.h"
#include <QMouseEvent>
#include <QScrollBar>
#include <QtDebug>


static const qreal minMoveAmount(10);


ScrollArea::ScrollArea(QWidget *parent) : QScrollArea(parent) {}


Qt::MouseButton ScrollArea::panButton() const {
	return _panButton;
}


void ScrollArea::setPanButton(Qt::MouseButton panButton) {
	_panButton = panButton;
}


void ScrollArea::mouseMoveEvent(QMouseEvent *event) {
	if (_panningButton != Qt::NoButton) {
		event->accept();
		const QPointF distance = event->windowPos() - _pressPos;
		if (not _panning and distance.manhattanLength() >= minMoveAmount) {
			_panning = true;
		}
		if (_panning) {
			horizontalScrollBar()->setValue(_pressScrollbarValues.x() - distance.x());
			verticalScrollBar()->setValue(_pressScrollbarValues.y() - distance.y());
		}
	} else {
		QScrollArea::mouseMoveEvent(event);
	}
}


void ScrollArea::mousePressEvent(QMouseEvent *event) {
	// We may be already panning if the pan button was changed while panning.
	// In that case, finish the existing pan first.
	if (event->button() == _panButton and not _panning) {
		event->accept();
		_panningButton = _panButton;
		_pressPos = event->windowPos();
		_pressScrollbarValues.setX(horizontalScrollBar()->value());
		_pressScrollbarValues.setY(verticalScrollBar()->value());
	} else {
		QScrollArea::mousePressEvent(event);
	}
}


void ScrollArea::mouseReleaseEvent(QMouseEvent *event) {
	bool wasPanning = _panning;
	_panning = false;
	_panningButton = Qt::NoButton;
	
	if (wasPanning) {
		event->accept();	
	} else {
		QScrollArea::mouseReleaseEvent(event);
	}
}
