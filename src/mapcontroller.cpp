#include "mapcontroller.h"
#include "map.h"
#include "mapobject.h"


MapController::MapController(QObject *parent) : QObject(parent) {
	_map = new Map(this);
}


const Map *MapController::map() {
	return _map;
}


void MapController::clear() {
	_map->clear();
}


void MapController::deleteObject(int objectNo) {
	Q_ASSERT(OBJECT_MIN <= objectNo and objectNo <= OBJECT_MAX);
	_map->setObject(objectNo, MapObject());
	_map->compact();
}


QString MapController::load(const QString &path) {
	return _map->load(path);
}


void MapController::moveObject(int objectNo, const QPoint &position) {
	MapObject object = _map->object(objectNo);
	if (object.pos() == position) { return; }
	object.x = position.x();
	object.y = position.y();
	_map->setObject(objectNo, object);
}


void MapController::setObject(int objectNo, const MapObject &object) {
	_map->setObject(objectNo, object);
}


void MapController::beginTileDrawing() {
	
}


void MapController::endTileDrawing() {
	
}


void MapController::floodFill(const QPoint &position, uint8_t tileNo) {
	_map->floodFill(position, tileNo);
}


void MapController::setTile(const QPoint &position, uint8_t tileNo) {
	_map->setTile(position, tileNo);
}
