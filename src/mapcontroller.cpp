#include "mapcontroller.h"
#include "map.h"
#include "mapcommands.h"
#include "mapobject.h"


MapController::MapController(QObject *parent) : QObject(parent) {
	_map = new Map(this);
}


const Map *MapController::map() {
	return _map;
}


void MapController::clear() {
	_map->clear(); // TODO undo/redo
}


QString MapController::load(const QString &path) {
	QString result = _map->load(path);
	if (result.isNull()) {
		_undoStack.clear();
	}
	return result;
}


QString MapController::save(const QString &path) {
	return _map->save(path);
}


void MapController::deleteObject(int objectNo) {
	Q_ASSERT(OBJECT_MIN <= objectNo and objectNo <= OBJECT_MAX);
	_undoStack.push(new MapCommands::DeleteObject(*_map, objectNo));
}


void MapController::moveObject(int objectNo, const QPoint &position) {
	_undoStack.push(new MapCommands::MoveObject(*_map, objectNo, position));
}


void MapController::setObject(int objectNo, const MapObject &object) { // TODO rename to modifyObject? check callers
	_undoStack.push(new MapCommands::ModifyObject(*_map, objectNo, object));
}


void MapController::beginTileDrawing() {
	_undoStack.beginMacro("Set Tiles");
}


void MapController::endTileDrawing() {
	_undoStack.endMacro();
}


void MapController::floodFill(const QPoint &position, uint8_t tileNo) {
	_undoStack.push(new MapCommands::FloodFill(*_map, position, tileNo));
}


void MapController::setTile(const QPoint &position, uint8_t tileNo) {
	_undoStack.push(new MapCommands::SetTile(*_map, position, tileNo));
}
