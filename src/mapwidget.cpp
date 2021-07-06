#include "mapwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QSize>
#include <unordered_map>
#include "constants.h"
#include "map.h"
#include "tile.h"
#include "tileset.h"


MapWidget::MapWidget(QWidget *parent) : AbstractTileWidget(parent) {
	setMouseTracking(true);
	
	_image = new QImage(imageSize(), IMAGE_FORMAT);
	_tilesImage = new QImage(imageSize(), IMAGE_FORMAT);
}


MapWidget::~MapWidget() {
	delete _image;
	delete _tilesImage;
}


void MapWidget::setDragMode(MapWidget::DragMode dragMode) {
	_dragMode = dragMode;
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


QRect MapWidget::selectedArea() const {
	if (_dragAreaBegin.x() >= 0) {
		return QRect(QPoint(qMin(_dragAreaBegin.x(), _dragAreaEnd.x()),
		                    qMin(_dragAreaBegin.y(), _dragAreaEnd.y())),
		             QPoint(qMax(_dragAreaBegin.x(), _dragAreaEnd.x()),
		                    qMax(_dragAreaBegin.y(), _dragAreaEnd.y())));
	}
	return QRect();
}


void MapWidget::clearSelection() {
	_dragAreaBegin = _dragAreaEnd = QPoint(-1, -1);
	update();
}


void MapWidget::markObject(MapObject::id_t objectId) {
	auto setSelectedObject = [&](MapObject::id_t id) {
		if (_selectedObject != id) {
			_selectedObject = id;
			update();
		}
	};
	
	if (objectId < MapObject::IdMin or objectId > MapObject::IdMax) {
		setSelectedObject(MapObject::IdNone);
		return;
	}
	
	const MapObject &object = _map->object(objectId);
	if (object.unitType == MapObject::UnitType::None) {
		setSelectedObject(MapObject::IdNone);
	} else {
		setSelectedObject(objectId);
	}
}


void MapWidget::selectAll() {
	setDragMode(DragMode::Area);
	_dragAreaBegin = _map->rect().topLeft();
	_dragAreaEnd = _map->rect().bottomRight();
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
	for (int y = 0; y < _map->height(); ++y) {
		for (int x = 0; x < _map->width(); ++x) {
			const QPoint pos(x, y);
			emit tilePressed(pos);

			if (not _objectsVisible) { continue; }
			
			for (MapObject::id_t id = MapObject::IdMin; id <= MapObject::IdMax; ++id) {
				const MapObject &object = _map->object(id);
				if (object.unitType != MapObject::UnitType::None and object.pos() == pos) {
					emit objectClicked(id);
					break;
				}
			}
			
			emit objectClicked(MapObject::IdNone);
		}
	}
}


void MapWidget::paintEvent(QPaintEvent *event) {
	Q_UNUSED(event);
	
	if (_redrawImage) { makeImage(); }
	
	QPainter painter(this);
	painter.scale(scale(), scale());
	painter.drawImage(0, 0, *_image);
	
	if (_objectsVisible) {
		painter.save();
		for (MapObject::id_t id = MapObject::IdMin; id <= MapObject::IdMax; ++id) {
			drawObject(painter, id);
		}
		painter.restore();
	}
	
	if (highlightAttribute() != Tile::None) {
		for (int y = 0; y < _map->height(); ++y) {
			for (int x = 0; x < _map->width(); ++x) {
				const QPoint position(x, y);
				Tile t = tile(position);
				QRect r = tileRect(position);
				painter.setPen(Qt::NoPen);
				painter.setBrush(t.attributes().testFlag(highlightAttribute()) ?
				                     highlightColor() : noHighlightColor());
				painter.drawRect(r);
			}
		}
	}
	
	painter.resetTransform();
	
	const QSizeF tileSize = QSizeF(tileset()->tileSize()) * scale();
	
	if (_showGridLines) {
		painter.setPen(QPen(C::colorGrid, 1));
		for (int i = 1; i < _map->width(); ++i) {
			const int x = i * tileSize.width();
			painter.drawLine(x, 0, x, imageSize().height());
		}
		for (int i = 1; i < _map->height(); ++i) {
			const int y = i * tileSize.height();
			painter.drawLine(0, y, imageSize().width(), y);
		}
	}
	
	if (_dragMode == DragMode::Area and _dragAreaBegin.x() >= 0) {
		painter.setPen(QPen(C::colorAreaSelection, 2));
		QColor fillColor(C::colorAreaSelection);
		fillColor.setAlpha(50);
		painter.setBrush(fillColor);
		const double left = qMin(_dragAreaBegin.x(), _dragAreaEnd.x()) * tileSize.width();
		const double right = (qMax(_dragAreaBegin.x(), _dragAreaEnd.x()) + 1) * tileSize.width() - 1;
		const double top = qMin(_dragAreaBegin.y(), _dragAreaEnd.y()) * tileSize.height();
		const double bottom = (qMax(_dragAreaBegin.y(), _dragAreaEnd.y()) + 1) * tileSize.height() - 1;
		painter.drawRect(QRectF(QPointF(left, top), QPointF(right, bottom)));
	}
}


QSize MapWidget::sizeHint() const {
	return imageSize() * scale();
}


void MapWidget::highlightAttributeChanged() {
	_redrawImage = true;
	update();
}


void MapWidget::mouseMoveEvent(QMouseEvent *event) {
	const QPoint tilePos = pixelToTile(event->pos());
	emit mouseOverTile(tilePos);
	
	if (event->buttons() == Qt::LeftButton) {
		event->accept();
		
		switch (_dragMode) {
		case DragMode::Area:
			_dragAreaEnd = tilePos;
			update();
			break;
		case DragMode::Single:
			emit tileDragged(tilePos);
			if (_dragObject != MapObject::IdNone) {
				emit objectDragged(_dragObject, tilePos);
			}
			break;
		}
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
	const QPoint tilePos = pixelToTile(event->pos());
	switch (_dragMode) {
	case DragMode::Area:
		_dragAreaBegin = tilePos;
		_dragAreaEnd = tilePos;
		update();
		break;
	case DragMode::Single:
		if (_objectsVisible) {
			bool foundObject = false;
			for (MapObject::id_t id = MapObject::IdMin; id <= MapObject::IdMax; ++id) {
				const MapObject &object = _map->object(id);
				if (object.unitType != MapObject::UnitType::None and object.pos() == tilePos) {
					emit objectClicked(id);
					foundObject = true;
					_dragObject = id;
					break;
				}
			}
			
			if (not foundObject) {
				emit objectClicked(MapObject::IdNone);
			}
		}
		
		if (event->modifiers().testFlag(Qt::ControlModifier)) {
			emit tileCopied(_map->tileNo(tilePos));
		} else {
			emit tilePressed(tilePos);
		}
		emit mouseOverTile(tilePos);
		break;
	}
}


void MapWidget::mouseReleaseEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
		event->accept();
		_dragObject = MapObject::IdNone;
		emit released();
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
	return QSize(_map->width() * tileset()->tileSize().width(),
	             _map->height() * tileset()->tileSize().height());
}


void MapWidget::drawObject(QPainter &painter, MapObject::id_t objectId) {
	static const QColor playerColor(128, 255, 128);
	static const QColor robotColor(255, 128, 128);
	static const std::unordered_map<MapObject::UnitType, std::pair<uint8_t, QColor>> objectTiles = {
	    { MapObject::UnitType::Player,         { 97, playerColor }},
	    { MapObject::UnitType::HoverbotLR,     {  98, robotColor }},
	    { MapObject::UnitType::HoverbotUD,     {  98, robotColor }},
	    { MapObject::UnitType::HoverbotAttack, {  99, robotColor }},
	    { MapObject::UnitType::Evilbot,        { 100, robotColor }},
	    { MapObject::UnitType::RollerbotUD,    { 164, robotColor }},
	    { MapObject::UnitType::RollerbotLR,    { 165, robotColor }},
	};
	
	const MapObject &object = _map->object(objectId);
	if (object.unitType == MapObject::UnitType::None) { return; }
	const QRect r = tileRect(object.pos());
	std::pair<uint8_t, QColor> pair;
	try {
		pair = objectTiles.at(object.unitType);
		
		const uint8_t &tileNo = pair.first;
		const QColor &color = pair.second;
		
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.drawImage(r, tileset()->tile(tileNo).image());
		painter.setCompositionMode(QPainter::CompositionMode_Darken);
		painter.setBrush(color);
		painter.drawRect(r);
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.setPen(QPen(Qt::white, 2));
		
		const QSize ts = tileset()->tileSize();
		if (object.unitType == MapObject::UnitType::HoverbotLR or
		        object.unitType == MapObject::UnitType::RollerbotLR) {
			const int y = r.top() + ts.height() / 2;
			const int third = ts.width() / 3 + 1;
			painter.drawLine(r.left() + third, y, r.right() - third, y);
		} else if (object.unitType == MapObject::UnitType::HoverbotUD or
		           object.unitType == MapObject::UnitType::RollerbotUD) {
			const int x = r.left() + ts.width() / 2;
			const int third = ts.height() / 3 + 1;
			painter.drawLine(x, r.top() + third, x, r.bottom() - third);
		}
	}  catch (std::out_of_range) {
		try {
			drawSpecialObject(painter, r, object.unitType);
		} catch (std::out_of_range) {
			// draw nothing
		}
	}
	
	if (objectId == _selectedObject) {
		painter.setPen(Qt::NoPen);
		painter.setBrush(C::colorTileSelection);
		drawMargin(painter, tileRect(object.pos()), 2);
	}
}


void MapWidget::drawSpecialObject(QPainter &painter, const QRect &rect, MapObject::UnitType unitType) {
	static const QColor toolColor(255, 255, 100);
	static const QColor weaponColor(255, 150, 0);
	static const std::unordered_map<MapObject::UnitType, std::pair<QString, QColor>> textAndColor {
		{ MapObject::UnitType::TransporterPad, { "Pad", { 150, 255, 255 }}},
		{ MapObject::UnitType::Door, { "Door", { 140, 190, 255 }}},
		{ MapObject::UnitType::TrashCompactor, { "TC", { 255, 64, 0 }}},
		{ MapObject::UnitType::Elevator, { "Lift", { 150, 200, 255 }}},
		{ MapObject::UnitType::WaterRaft, { "Raft", { 150, 200, 255 }}},
		{ MapObject::UnitType::Key, { "Key", { 80, 130, 255 }}},
		{ MapObject::UnitType::TimeBomb, { "Bom", weaponColor }},
		{ MapObject::UnitType::EMP, { "EMP", toolColor }},
		{ MapObject::UnitType::Pistol, { "Gun", weaponColor }},
		{ MapObject::UnitType::PlasmaGun, { "Plas", weaponColor }},
		{ MapObject::UnitType::Medkit, { "Med", { 100, 255, 100 }}},
		{ MapObject::UnitType::Magnet, { "Mag", toolColor }},
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
	
	for (int y = 0; y < _map->height(); ++y) {
		for (int x = 0; x < _map->width(); ++x) {
			const QPoint position(x, y);
			const Tile t = tile(position);
			const QRect r = tileRect(position);
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


Tile MapWidget::tile(QPoint position) const {
	const uint8_t tileNo = _map->tileNo(position);
	return tileset()->tile(tileNo);
}


QRect MapWidget::tileRect(const QPoint &position) const {
	const QSize ts = tileset()->tileSize();
	return QRect(QPoint(position.x() * ts.width(), position.y() * ts.height()), ts);
}


QPoint MapWidget::pixelToTile(QPoint pos) {
	const QSizeF tileSize = QSizeF(tileset()->tileSize()) * scale();
	const int x = qBound(0, static_cast<int>(pos.x() / tileSize.width()), _map->width() - 1);
	const int y = qBound(0, static_cast<int>(pos.y() / tileSize.height()), _map->height() - 1);
	return QPoint(x, y);
}
