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
}


MapWidget::~MapWidget() {
	delete _tilesImage;
	delete _image;
}


void MapWidget::setMap(Map *map) {
	_map = map;
	makeTilesImage();
	makeObjectsImage();
	makeImage();
	update();
}


void MapWidget::setObjectsVisible(bool visible) {
	_objectsVisible = visible;
	makeImage();
	update();
}


void MapWidget::clickEveryTile() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			emit tileClicked(x, y);
			if (_objectsVisible) {
				for (int i = 0; i <= OBJECTS_MAX; ++i) {
					Map::Object &object = _map->object(i);
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
	if (_image == nullptr) { return; }
	QPainter painter(this);
	painter.scale(scale(), scale());
	painter.drawImage(0, 0, *_image);
}


QSize MapWidget::sizeHint() const {
	static const QSize baseSize(MAP_WIDTH * TILE_WIDTH * GLYPH_WIDTH,
	                            MAP_HEIGHT * TILE_HEIGHT * GLYPH_HEIGHT);
	return baseSize * scale();
}


void MapWidget::flagsChanged() {
	makeImage();
	update();
}


void MapWidget::mouseMoveEvent(QMouseEvent *event) {
	const QPoint p = event->pos();
	const int x = p.x() / scale() / GLYPH_WIDTH / TILE_WIDTH;
	const int y = p.y() / scale() / GLYPH_HEIGHT / TILE_HEIGHT;
	emit mouseOverTile(x, y);
	QWidget::mouseMoveEvent(event);
}


void MapWidget::mousePressEvent(QMouseEvent *event) {
	if (event->button() != Qt::LeftButton) {
		QWidget::mousePressEvent(event);
		return;
	}
	
	const QPoint p = event->pos();
	const int x = p.x() / scale() / GLYPH_WIDTH / TILE_WIDTH;
	const int y = p.y() / scale() / GLYPH_HEIGHT / TILE_HEIGHT;
	emit tileClicked(x, y);
	if (_objectsVisible) {
		for (int i = 0; i <= OBJECTS_MAX; ++i) {
			Map::Object &object = _map->object(i);
			if (object.x == x and object.y == y) {
				emit objectClicked(i);
				return;
			}
		}
		
		emit objectClicked(-1);
	}
}


void MapWidget::scaleChanged() {
	makeImage();
	update();
}


void MapWidget::tilesetChanged() {
	makeImage();
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
	    { PLAYER_OBJ,             {  97, playerColor }},
	    { ROBOT_HOVERBOT_LR,      {  98, robotColor }},
	    { ROBOT_HOVERBOT_UD,      {  98, robotColor }},
	    { ROBOT_HOVERBOT_ATTACK,  {  99, robotColor }},
	    { ROBOT_EVILBOT,          { 100, robotColor }},
	    { ROBOT_ROLLERBOT_LR,     { 164, robotColor }},
	    { ROBOT_ROLLERBOT_UD,     { 164, robotColor }},
	};
	
	const Map::Object &object = _map->object(objectNo);
	std::pair<int, QColor> pair;
	try {
		pair = objectTiles.at(object.unitType);
	}  catch (std::out_of_range) {
		pair = { -object.unitType, QColor() };
	}
	const int &tileNo = pair.first;
	const QColor &color = pair.second;
	
	const QRect r = tileRect(object.x, object.y);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter.drawImage(r, tileset()->tile(tileNo).image());
	if (tileNo >= 0) { // special tiles are already colorized because this doesn't work with transparency
		painter.setCompositionMode(QPainter::CompositionMode_Darken);
		painter.setBrush(color);
		painter.drawRect(r);
	}
}


void MapWidget::makeTilesImage() {
	delete _tilesImage;
	_tilesImage = nullptr;
	if (_map == nullptr or tileset() == nullptr) { return; }
	_tilesImage = new QImage(imageSize(), QImage::Format_ARGB32_Premultiplied);
	QPainter painter(_tilesImage);
	painter.setPen(Qt::NoPen);
	
	for (int y = 0; y < 64; ++y) {
		for (int x = 0; x < 128; ++x) {
			Tile t = tile(x, y);
			QRect r = tileRect(x, y);
			painter.drawImage(r, t.image());
		}
	}
}


void MapWidget::makeObjectsImage() {
	
	
	if (_map == nullptr or tileset() == nullptr) { return; }
	if (not _objectsImage) {
		_objectsImage = new QImage(imageSize(), QImage::Format_ARGB32_Premultiplied);
	}
	_objectsImage->fill(Qt::transparent);
	
	QPainter painter(_objectsImage);
	painter.setPen(Qt::NoPen);
	
	drawObject(painter, PLAYER_OBJ);
	
	for (int i = OBJECT_MIN; i <= OBJECT_MAX; ++i) {
		Map::Object &object = _map->object(i);
		if (object.kind() != Map::Object::Kind::Invalid) {
			drawObject(painter, i);
		}
	}
}


void MapWidget::makeImage() {
	delete _image;
	if (_map == nullptr or tileset() == nullptr) { return; }
	_image = new QImage(imageSize(), QImage::Format_ARGB32_Premultiplied);
	QPainter painter(_image);
	painter.setPen(Qt::NoPen);
	painter.drawImage(0, 0, *_tilesImage);
	
	if (_objectsImage and _objectsVisible) {
		painter.drawImage(QPoint(0, 0), *_objectsImage, _objectsImage->rect(), Qt::NoOpaqueDetection);
	}
	
	if (highlightFlags()) {
		for (int y = 0; y < 64; ++y) {
			for (int x = 0; x < 128; ++x) {
				Tile t = tile(x, y);
				QRect r = tileRect(x, y);
				painter.setBrush(t.flags() & highlightFlags() ? highlightColor() : noHighlightColor());
				painter.drawRect(r);
			}
		}
	}
}


Tile MapWidget::tile(int x, int y) {
	int tileNo = _map->tileNo(x, y);
	return tileset()->tile(tileNo);
}


QRect MapWidget::tileRect(int x, int y) {
	return QRect(x * TILE_WIDTH * GLYPH_WIDTH, y * TILE_HEIGHT * GLYPH_HEIGHT,
	             TILE_WIDTH * GLYPH_WIDTH, TILE_HEIGHT * GLYPH_HEIGHT);
}
