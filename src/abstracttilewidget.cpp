#include "abstracttilewidget.h"
#include <QPainter>
#include <QRect>
#include <cmath>
#include <unordered_map>
#include "constants.h"
#include "tileset.h"


/** @class AbstractTileWidget
 * AbstractTileWidget displays PETSCII Robots tiles in a 2-dimensional grid.
 * It can also show colored overlays to indicate a tile's attributes.
 */


AbstractTileWidget::AbstractTileWidget(QWidget *parent) : QWidget(parent) {}


AbstractTileWidget::~AbstractTileWidget() {}


/// @{
/** Set the tileset that's used for drawing. */
void AbstractTileWidget::setTileset(const Tileset *tileset) {
	if (_tileset) { disconnect(_tileset); }
	_tileset = tileset;
	connect(_tileset, &Tileset::changed, [&]() { tilesetChanged(); });
	tilesetChanged();
}


/** Get the tileset that's used for drawing. */
const Tileset *AbstractTileWidget::tileset() const {
	return _tileset;
}
/// @}


/// @{
/** Zoom in, drawing the tiles larger.
 * Three zoom levels are supported, 0.5x, 1x and 2x.  
 */
void AbstractTileWidget::zoomIn() {
	_scale = fmin(2.0, _scale * 2.0);
	updateGeometry();
	scaleChanged();
}


/** Zoom out, drawing the tiles smaller.
 * Three zoom levels are supported, 0.5x, 1x and 2x.  
 */
void AbstractTileWidget::zoomOut() {
	_scale = fmax(0.5, _scale * 0.5);
	updateGeometry();
	scaleChanged();
}
/// @}


/** Set which attribute should be highlighted. Set it to #Tile::None to
 *  disable highlighting.
 */
void AbstractTileWidget::setHighlightAttribute(Tile::Attribute attribute) {
	_highlightAttribute = attribute;
	highlightAttributeChanged();
}


/** Whether to enable showing the currently selected tile by drawing a colored
 * frame around it.
 */
void AbstractTileWidget::setShowSelected(bool show) {
	_showSelected = show;
	update();
}


/** Whether the currently selected tile is visibly marked. */
bool AbstractTileWidget::showSelected() const {
	return _showSelected;
}


/// @{
/** @fn AbstractTileWidget::highlightAttributeChanged
 *  
 * @brief This method is called when the tile attribute to be highlighted has
 * changed, i.e. #setHighlightAttribute() has been called.
 */

/** @fn AbstractTileWidget::scaleChanged
 *  
 *  @brief This method is called when the scale has been changed, i.e.
 *  #zoomIn() or #zoomOut() has been called.
 */

/** @fn AbstractTileWidget::tilesetChanged
 * 
 * @brief This method is called when the tileset has changed. This means that
 * either #setTileset() has been called, or the tileset itself has changed
 * because #Tileset::load() has been called.
 */
/// @}


void AbstractTileWidget::drawMargin(QPainter &painter, const QRect &rect, int margin) {
	Q_ASSERT(painter.pen().style() == Qt::NoPen);
	QRect oRect = rect.adjusted(-margin, -margin, margin, margin);
	QRect iRect = rect.adjusted(-1, -1, 1, 1);	
	painter.drawRect(QRect(oRect.topLeft(), QPoint(oRect.right(), iRect.top()))); // top
	painter.drawRect(QRect(QPoint(oRect.left(), iRect.bottom()), oRect.bottomRight())); // bottom
	painter.drawRect(QRect(QPoint(oRect.left(), rect.top()), QPoint(iRect.left(), rect.bottom()))); // left
	painter.drawRect(QRect(QPoint(iRect.right(), rect.top()), QPoint(oRect.right(), rect.bottom()))); // right
}


/// @{
/** Return the color for the currently highlighted attribute. */
QColor AbstractTileWidget::highlightColor() const {
	static const std::unordered_map<Tile::Attribute, QColor> Colors = {
	    { Tile::None, Qt::transparent }, { Tile::Walkable, C::colorWalkable },
	    { Tile::Hoverable, C::colorHoverable }, { Tile::Movable, C::colorMovable },
	    { Tile::Destructible, C::colorDestructible }, { Tile::ShootThrough, C::colorShootThrough },
	    { Tile::PushOnto, C::colorPushOnto }, { Tile::Searchable, C::colorSearchable }
	};
	return Colors.at(_highlightAttribute);
}


/** The color for tiles that don't have the currently highlighted attribute. */
QColor AbstractTileWidget::noHighlightColor() const {
	return C::colorDarken;
}
/// @}


/** The currently highlighted tile attribute.
 *  
 * Will return #Tile::None if no attribute is currently highlighted.
 */
Tile::Attribute AbstractTileWidget::highlightAttribute() const {
	return _highlightAttribute;
}


/** The current scale. The scale is changed through #zoomIn() and #zoomOut(). */
double AbstractTileWidget::scale() const {
	return _scale;
}
