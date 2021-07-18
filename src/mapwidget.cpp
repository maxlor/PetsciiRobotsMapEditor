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
	
	_tilesImage = new QImage(imageSize(), IMAGE_FORMAT);
}


MapWidget::~MapWidget() {
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
	_redrawTiles = true;
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
	if (_redrawTiles) { makeTilesImage(); }
	
	QPainter painter(this);
	painter.scale(scale(), scale());
	painter.drawImage(0, 0, *_tilesImage);
	
	if (_objectsVisible) {
		for (MapObject::id_t id = MapObject::IdMin; id <= MapObject::IdMax; ++id) {
			drawMapObject(painter, id);
		}
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
	_redrawTiles = true;
	makeObjectImages();
	update();
}


void MapWidget::onMapObjectsChanged() {
	if (_objectsVisible) {
		update();
	}
}


void MapWidget::onMapTilesChanged() {
	_redrawTiles = true;
	update();
}


QSize MapWidget::imageSize() const {
	return QSize(_map->width() * tileset()->tileSize().width(),
	             _map->height() * tileset()->tileSize().height());
}


void MapWidget::drawMapObject(QPainter &painter, MapObject::id_t objectId) {
	const MapObject &object = _map->object(objectId);
	if (object.unitType == MapObject::UnitType::None) { return; }
	
	if (object.unitType == MapObject::UnitType::WaterRaft) {
		const QRectF leftStop = tileRect(QPoint(object.b, object.y));
		const QRectF rightStop = tileRect(QPoint(object.c, object.y));
		painter.setPen(QPen(C::colorWaterRaft, 2));
		painter.drawLine(leftStop.center(), rightStop.center());
	}
	
	const QRect r = tileRect(object.pos());
	painter.drawImage(r, _objectImages.at(object.unitType));
	
	if (objectId == _selectedObject) {
		painter.setPen(Qt::NoPen);
		painter.setBrush(C::colorTileSelection);
		drawMargin(painter, tileRect(object.pos()), 2);
	}
}


void MapWidget::drawObject(QPainter &painter, const QRect &rect, MapObject::UnitType unitType) {
	static const std::unordered_map<MapObject::UnitType, std::pair<uint8_t, QColor>> objectTiles = {
	    { MapObject::UnitType::Player,         {  97, C::colorPlayer }},
	    { MapObject::UnitType::HoverbotLR,     {  98, C::colorRobot }},
	    { MapObject::UnitType::HoverbotUD,     {  98, C::colorRobot }},
	    { MapObject::UnitType::HoverbotAttack, {  99, C::colorRobot }},
	    { MapObject::UnitType::Evilbot,        { 100, C::colorRobot }},
	    { MapObject::UnitType::RollerbotUD,    { 164, C::colorRobot }},
	    { MapObject::UnitType::RollerbotLR,    { 165, C::colorRobot }},
	};
	
	std::pair<uint8_t, QColor> pair;
	try {
		pair = objectTiles.at(unitType);
		
		const uint8_t &tileNo = pair.first;
		const QColor &color = pair.second;
		
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter.drawImage(rect, tileset()->tile(tileNo).image());
		if (not tileset()->haveColor()) {
			painter.setCompositionMode(QPainter::CompositionMode_Darken);
			painter.setBrush(color);
			painter.drawRect(rect);
			painter.setPen(QPen(Qt::white, 2));
		} else {
			painter.setPen(QPen(QColor(0xFFBB66), 2));
		}
		
		painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
		const QSize ts = tileset()->tileSize();
		if (unitType == MapObject::UnitType::HoverbotLR or
		        unitType == MapObject::UnitType::RollerbotLR) {
			const int y = rect.top() + ts.height() / 2;
			const int third = ts.width() / 3 + 1;
			painter.drawLine(rect.left() + third, y, rect.right() - third, y);
		} else if (unitType == MapObject::UnitType::HoverbotUD or
		           unitType == MapObject::UnitType::RollerbotUD) {
			const int x = rect.left() + ts.width() / 2;
			const int third = ts.height() / 3 + 1;
			painter.drawLine(x, rect.top() + third, x, rect.bottom() - third);
		}
	}  catch (std::out_of_range) {
		try {
			drawSpecialObject(painter, rect, unitType);
		} catch (std::out_of_range) {
			// draw nothing
		}
	}
}


void MapWidget::drawSpecialObject(QPainter &painter, const QRect &rect, MapObject::UnitType unitType) {
	static const std::unordered_map<MapObject::UnitType, std::pair<QString, QColor>> textAndColor {
		{ MapObject::UnitType::TransporterPad, { "Pad", C::colorTransporterPad }},
		{ MapObject::UnitType::Door, { "Door", C::colorDoor }},
		{ MapObject::UnitType::TrashCompactor, { "TC", C::colorTC }},
		{ MapObject::UnitType::Elevator, { "Lift", C::colorElevator }},
		{ MapObject::UnitType::WaterRaft, { "Raft", C::colorWaterRaft }},
		{ MapObject::UnitType::Key, { "Key", C::colorKey }},
		{ MapObject::UnitType::TimeBomb, { "Bom", C::colorWeapon }},
		{ MapObject::UnitType::EMP, { "EMP", C::colorTool }},
		{ MapObject::UnitType::Pistol, { "Gun", C::colorWeapon }},
		{ MapObject::UnitType::PlasmaGun, { "Plas", C::colorWeapon }},
		{ MapObject::UnitType::Medkit, { "Med", C::colorMedkit }},
		{ MapObject::UnitType::Magnet, { "Mag", C::colorTool }},
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
	static constexpr int Bpp(4); // bytes per pixel
	if (_map == nullptr or tileset() == nullptr) { return; }
	QPainter painter(_tilesImage);
	painter.setPen(Qt::NoPen);
	
	uchar *dst = _tilesImage->bits();
	for (int y = 0; y < _map->height(); ++y) {
		for (int x = 0; x < _map->width(); ++x) {
			const QPoint position(x, y);
			const QImage &tileImage = tile(position).image();
			const QRect r = tileRect(position);
#if 1
			const uchar *src = tileImage.bits();
			for (int py = 0; py < r.height(); ++py) {
				memcpy(&dst[(r.top() + py) * _tilesImage->bytesPerLine() + r.left() * Bpp],
				       &src[py * tileImage.bytesPerLine()],
				       tileImage.bytesPerLine());
			}
#else
			painter.drawImage(r, tileImage);
#endif
		}
	}
	_redrawTiles = false;
}


void MapWidget::makeObjectImages() {
	for (MapObject::UnitType unitType : MapObject::unitTypes()) {
		auto emplaceResult = _objectImages.try_emplace(unitType, tileset()->tileSize() * 2, IMAGE_FORMAT);
		QImage &image = emplaceResult.first->second;
		image.fill(Qt::transparent);
		QPainter painter(&image);
		painter.scale(2, 2);
		
		drawObject(painter, tileRect({0, 0}), unitType);
	}
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
