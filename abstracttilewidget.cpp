#include "abstracttilewidget.h"
#include <QPainter>
#include <QRect>
#include <cmath>
#include "constants.h"
#include "tileset.h"


AbstractTileWidget::AbstractTileWidget(QWidget *parent) : QWidget(parent) {}


AbstractTileWidget::~AbstractTileWidget() {}


void AbstractTileWidget::setTileset(const Tileset *tileset) {
	if (_tileset) { disconnect(_tileset); }
	_tileset = tileset;
	connect(_tileset, &Tileset::changed, [&]() { tilesetChanged(); });
	tilesetChanged();
}


const Tileset *AbstractTileWidget::tileset() const {
	return _tileset;
}


void AbstractTileWidget::zoomIn() {
	_scale = fmin(2.0, _scale * 2.0);
	updateGeometry();
	scaleChanged();
}


void AbstractTileWidget::zoomOut() {
	_scale = fmax(0.5, _scale * 0.5);
	updateGeometry();
	scaleChanged();
}


void AbstractTileWidget::setEnlarge(bool enlarge) {
	_scale = enlarge ? 2 : 1;
	updateGeometry();
	scaleChanged();
}


void AbstractTileWidget::setShowWalkable(bool show) {
	if (show) { _highlightColor = C::colorWalkable; }
	setHighlightFlag(0, show);
}


void AbstractTileWidget::setShowHoverable(bool show) {
	if (show) { _highlightColor = C::colorHoverable; }
	setHighlightFlag(1, show);
}


void AbstractTileWidget::setShowMovable(bool show) {
	if (show) { _highlightColor = C::colorMovable; }
	setHighlightFlag(2, show);
}


void AbstractTileWidget::setShowDestructible(bool show) {
	if (show) { _highlightColor = C::colorDestructible; }
	setHighlightFlag(3, show);
}


void AbstractTileWidget::setShowShootThrough(bool show) {
	if (show) { _highlightColor = C::colorShootThrough; }
	setHighlightFlag(4, show);
}


void AbstractTileWidget::setShowPushOnto(bool show) {
	if (show) { _highlightColor = C::colorPushOnto; }
	setHighlightFlag(5, show);
}


void AbstractTileWidget::setShowSearchable(bool show) {
	if (show) { _highlightColor = C::colorSearchable; }
	setHighlightFlag(6, show);
}


void AbstractTileWidget::setShowSelected(bool show) {
	_showSelected = show;
	update();
}


bool AbstractTileWidget::showSelected() const {
	return _showSelected;
}


void AbstractTileWidget::drawMargin(QPainter &painter, const QRect &rect, int margin) {
	Q_ASSERT(painter.pen().style() == Qt::NoPen);
	QRect oRect = rect.adjusted(-margin, -margin, margin, margin);
	QRect iRect = rect.adjusted(-1, -1, 1, 1);	
	painter.drawRect(QRect(oRect.topLeft(), QPoint(oRect.right(), iRect.top()))); // top
	painter.drawRect(QRect(QPoint(oRect.left(), iRect.bottom()), oRect.bottomRight())); // bottom
	painter.drawRect(QRect(QPoint(oRect.left(), rect.top()), QPoint(iRect.left(), rect.bottom()))); // left
	painter.drawRect(QRect(QPoint(iRect.right(), rect.top()), QPoint(oRect.right(), rect.bottom()))); // right
}


QColor AbstractTileWidget::highlightColor() const {
	return _highlightColor;
}


QColor AbstractTileWidget::noHighlightColor() const {
	return C::colorDarken;
}


uint8_t AbstractTileWidget::highlightFlags() const {
	return _highlightFlags;
}


double AbstractTileWidget::scale() const {
	return _scale;
}


void AbstractTileWidget::setHighlightFlag(int bit, bool set) {
	if (set) {
		_highlightFlags |= (1 << bit);
	} else {
		_highlightFlags &= ~(1 << bit);
	}
	highlightFlagsChanged();
}
