#include "mapwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QSize>
#include "constants.h"
#include "map.h"
#include "tile.h"
#include "tileset.h"
#include <QtDebug>


MapWidget::MapWidget(QWidget *parent) : AbstractTileWidget(parent) {
	setMouseTracking(true);
	
	_image = new QImage(imageSize(), IMAGE_FORMAT);
	_tilesImage = new QImage(imageSize(), IMAGE_FORMAT);
}


MapWidget::~MapWidget() {
	delete _image;
	delete _tilesImage;
}


void MapWidget::setMap(const Map *map) {
	disconnect(_map);
	_map = map;
	connect(_map, &Map::tilesChanged, this, &MapWidget::onMapTilesChanged);
	connect(_map, &Map::objectsChanged, this,
	        static_cast<void(MapWidget::*)()>(&MapWidget::update));
	_redrawImage = _redrawTiles = true;
	update();
}


void MapWidget::clearSelection() {
	// TODO
	update();
}


void MapWidget::setShowGridLines(bool enable) {
	if (_showGridLines != enable) {
		_showGridLines = enable;
		update();
	}
}


void MapWidget::setObjectsVisible(bool visible) {
	_objectsVisible = visible;
	update();
}


void MapWidget::clickEveryTile() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			emit tileClicked(x, y);
			if (_objectsVisible) {
				for (int i = 0; i <= OBJECT_MAX; ++i) {
					const Map::Object &object = _map->object(i);
					if (object.x == x and object.y == y) {
						emit objectClicked(i);
						break;
					}
				}
				
				emit objectClicked(-1);
			}
		}
	}
	qDebug() << Q_FUNC_INFO << "done";
}


void MapWidget::paintEvent(QPaintEvent *event) {
	Q_UNUSED(event);
	
	if (_redrawImage) { makeImage(); }
	
	QPainter painter(this);
	painter.scale(scale(), scale());
	painter.drawImage(0, 0, *_image);
	
	if (_objectsVisible) {
		painter.save();
		for (int i = 0; i <= OBJECT_MAX; ++i) {
			drawObject(painter, i);
		}
		painter.restore();
	}
	
	if (highlightFlags()) {
		for (int y = 0; y < MAP_HEIGHT; ++y) {
			for (int x = 0; x < MAP_WIDTH; ++x) {
				Tile t = tile(x, y);
				QRect r = tileRect(x, y);
				painter.setPen(Qt::NoPen);
				painter.setBrush(t.flags() & highlightFlags() ? highlightColor() : noHighlightColor());
				painter.drawRect(r);
			}
		}
	}
	
	painter.resetTransform();
	painter.setPen(QPen(C::colorGrid, 1));
	if (_showGridLines) {
		for (int i = 1; i < MAP_WIDTH; ++i) {
			const int x = i * TILE_WIDTH * GLYPH_WIDTH * scale();
			painter.drawLine(x, 0, x, imageSize().height());
		}
		for (int i = 1; i < MAP_HEIGHT; ++i) {
			const int y = i * TILE_HEIGHT * GLYPH_HEIGHT * scale();
			painter.drawLine(0, y, imageSize().width(), y);
		}
	}
}


QSize MapWidget::sizeHint() const {
	static const QSize baseSize(MAP_WIDTH * TILE_WIDTH * GLYPH_WIDTH,
	                            MAP_HEIGHT * TILE_HEIGHT * GLYPH_HEIGHT);
	return baseSize * scale();
}


void MapWidget::highlightFlagsChanged() {
	_redrawImage = true;
	update();
}


void MapWidget::mouseMoveEvent(QMouseEvent *event) {
	QPoint tilePos = pixelToTile(event->pos());
	emit mouseOverTile(tilePos.x(), tilePos.y());
	
	if (_drag) {
		event->accept();
		emit tileDragged(tilePos.x(), tilePos.y());
	} else {
		QWidget::mouseMoveEvent(event);
	}
}


void MapWidget::mousePressEvent(QMouseEvent *event) {
	if (event->button() != Qt::LeftButton) {
		QWidget::mousePressEvent(event);
		return;
	}
	
	event->accept();
	QPoint tilePos = pixelToTile(event->pos());
	_drag = true;
	if (_objectsVisible) {
		bool foundObject = false;
		for (int i = 0; i <= OBJECT_MAX; ++i) {
			const Map::Object &object = _map->object(i);
			if (object.x == tilePos.x() and object.y == tilePos.y()) {
				emit objectClicked(i);
				foundObject = true;
				break;
			}
		}
		
		if (not foundObject) {
			emit objectClicked(-1);
		}
	}
	
	emit tileClicked(tilePos.x(), tilePos.y());
	emit mouseOverTile(tilePos.x(), tilePos.y());
}


void MapWidget::mouseReleaseEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		event->accept();
		_drag = false;
		emit mouseReleased();
	} else {
		QWidget::mouseReleaseEvent(event);
	}
}


void MapWidget::scaleChanged() {
	update();
}


void MapWidget::tilesetChanged() {
	_redrawImage = _redrawTiles = true;
	update();
}


void MapWidget::onMapObjectsChanged() {
	if (_objectsVisible) {
		update();
	}
}


void MapWidget::onMapTilesChanged() {
	_redrawImage = _redrawTiles = true;
	update();
}


QSize MapWidget::imageSize() const {
	return QSize(MAP_WIDTH * TILE_WIDTH * GLYPH_WIDTH,
	             MAP_HEIGHT * TILE_HEIGHT * GLYPH_HEIGHT);
}


void MapWidget::drawObject(QPainter &painter, int objectNo) {
	static const QColor playerColor(128, 255, 128);
	static const QColor robotColor(255, 128, 128);
	static const std::unordered_map<uint8_t, std::pair<int, QColor>> objectTiles = {
	    { UNITTYPE_PLAYER,        {  97, playerColor }},
	    { ROBOT_HOVERBOT_LR,      {  98, robotColor }},
	    { ROBOT_HOVERBOT_UD,      {  98, robotColor }},
	    { ROBOT_HOVERBOT_ATTACK,  {  99, robotColor }},
	    { ROBOT_EVILBOT,          { 100, robotColor }},
	    { ROBOT_ROLLERBOT_LR,     { 165, robotColor }},
	    { ROBOT_ROLLERBOT_UD,     { 164, robotColor }},
	};
	
	const Map::Object &object = _map->object(objectNo);
	if (object.unitType == UNITTYPE_NONE) { return; }
	const QRect r = tileRect(object.x, object.y);
	std::pair<int, QColor> pair;
	try {
		pair = objectTiles.at(object.unitType);
		
		const int &tileNo = pair.first;
		const QColor &color = pair.second;
		
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.drawImage(r, tileset()->tile(tileNo).image());
		painter.setCompositionMode(QPainter::CompositionMode_Darken);
		painter.setBrush(color);
		painter.drawRect(r);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.setPen(QPen(Qt::white, 2));
		if (object.unitType == ROBOT_HOVERBOT_LR or object.unitType == ROBOT_ROLLERBOT_LR) {
			int y = r.top() + 3 * GLYPH_WIDTH / 2;
			painter.drawLine(r.left() + GLYPH_WIDTH + 1, y, r.left() + 2 * GLYPH_WIDTH - 1, y);
		} else if (object.unitType == ROBOT_HOVERBOT_UD or object.unitType == ROBOT_ROLLERBOT_UD) {
			int x = r.left() + 3 * GLYPH_WIDTH / 2;
			painter.drawLine(x, r.top() + GLYPH_WIDTH + 1, x, r.top() + 2 * GLYPH_HEIGHT - 1);
		}
	}  catch (std::out_of_range) {
		try {
			drawSpecialObject(painter, r, object.unitType);
		} catch (std::out_of_range) {
			// draw nothing
		}
	}	
}


void MapWidget::drawSpecialObject(QPainter &painter, const QRect &rect, int unitType) {
	static const QColor toolColor(255, 255, 100);
	static const QColor weaponColor(255, 150, 0);
	static const std::unordered_map<int, std::pair<QString, QColor>> textAndColor {
		{ OBJECT_TRANSPORTER, { "Pad", { 150, 255, 255 }}},
		{ OBJECT_DOOR, { "Door", { 140, 190, 255 }}},
		{ OBJECT_TRASH_COMPACTOR, { "TC", { 255, 64, 0 }}},
		{ OBJECT_ELEVATOR, { "Lift", { 150, 200, 255 }}},
		{ OBJECT_WATER_RAFT, { "Raft", { 150, 200, 255 }}},
		{ OBJECT_KEY, { "Key", { 80, 130, 255 }}},
		{ OBJECT_TIME_BOMB, { "Bom", weaponColor }},
		{ OBJECT_EMP, { "EMP", toolColor }},
		{ OBJECT_PISTOL, { "Gun", weaponColor }},
		{ OBJECT_PLASMA_GUN, { "Plas", weaponColor }},
		{ OBJECT_MEDKIT, { "Med", { 100, 255, 100 }}},
		{ OBJECT_MAGNET, { "Mag", toolColor }},
	};
	static const QColor objectBgColor(0, 0, 0, 180);
	const std::pair<QString, QColor> textAndColorPair = textAndColor.at(unitType);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.setPen(QPen(textAndColorPair.second, 2));
	painter.setBrush(objectBgColor);
	QFont font("Nimbus Sans Narrow", -1, QFont::Bold);
	font.setPixelSize(10);
	painter.setFont(font);
	painter.drawEllipse(rect.adjusted(1, 1, -1, -1));
	painter.setPen(textAndColorPair.second);
	painter.drawText(rect, Qt::AlignCenter, textAndColorPair.first);
}


void MapWidget::makeTilesImage() {
	if (_map == nullptr or tileset() == nullptr) { return; }
	QPainter painter(_tilesImage);
	painter.setPen(Qt::NoPen);
	
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			const Tile t = tile(x, y);
			const QRect r = tileRect(x, y);
			painter.drawImage(r, t.image());
		}
	}
	_redrawTiles = false;
	_redrawImage = true;
}


void MapWidget::makeImage() {
	if (_map == nullptr or tileset() == nullptr) {
		_image->fill(Qt::black);
		return;
	}
	
	if (_redrawTiles) { makeTilesImage(); }
//	if (_redrawObjects and _objectsVisible) { makeObjectsImage(); }
	
	QPainter painter(_image);
	painter.setPen(Qt::NoPen);
	painter.drawImage(0, 0, *_tilesImage);
}


Tile MapWidget::tile(int x, int y) const {
	int tileNo = _map->tileNo(x, y);
	return tileset()->tile(tileNo);
}


QRect MapWidget::tileRect(int x, int y) const {
	return QRect(x * TILE_WIDTH * GLYPH_WIDTH, y * TILE_HEIGHT * GLYPH_HEIGHT,
	             TILE_WIDTH * GLYPH_WIDTH, TILE_HEIGHT * GLYPH_HEIGHT);
}


QPoint MapWidget::pixelToTile(QPoint pos) {
	const int x = pos.x() / scale() / GLYPH_WIDTH / TILE_WIDTH;
	const int y = pos.y() / scale() / GLYPH_HEIGHT / TILE_HEIGHT;
	return QPoint(x, y);
}
