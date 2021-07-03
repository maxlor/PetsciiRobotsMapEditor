#include "mapcommands.h"
#include <QRect>
#include <cstring>
#include "util.h"


enum CommandIds { MoveObjectId = 1 };


static QString unitTypeS(const MapObject &object) {
	return capitalize(MapObject::toString(object.unitType));
}


MapCommands::DeleteObject::DeleteObject(Map &map, MapObject::id_t objectId, QUndoCommand *parent)
    : QUndoCommand("Delete " + unitTypeS(map.object(objectId)), parent), _map(map), _objectId(objectId) {}


void MapCommands::DeleteObject::redo() {
	for (MapObject::id_t id = MapObject::IdMin; id <= MapObject::IdMax; ++id) {
		_previousState[id] = _map.object(id);
	}
	_map.deleteObject(_objectId);
}


void MapCommands::DeleteObject::undo() {
	_map.setObjects(_previousState);
}


MapCommands::ModifyObject::ModifyObject(Map &map, MapObject::id_t objectId, const MapObject &object,
                                        QUndoCommand *parent)
    : QUndoCommand("Modify " + unitTypeS(object), parent), _map(map), _objectId(objectId),
      _object(object) {}


void MapCommands::ModifyObject::redo() {
	_previousObject = _map.object(_objectId);
	_map.setObject(_objectId, _object);
}


void MapCommands::ModifyObject::undo() {
	_map.setObject(_objectId, _previousObject);
}


MapCommands::MoveObject::MoveObject(Map &map, MapObject::id_t objectId, const QPoint &pos, int mergeCounter,
                                    QUndoCommand *parent)
    : QUndoCommand("Move " + unitTypeS(map.object(objectId)), parent), _map(map), _objectId(objectId),
      _position(pos), _mergeCounter(mergeCounter) {}


int MapCommands::MoveObject::id() const {
	return MoveObjectId;
}


bool MapCommands::MoveObject::mergeWith(const QUndoCommand *command) {
	const MoveObject *other = dynamic_cast<const MoveObject*>(command);
	if (other->_objectId == _objectId and other->_mergeCounter == _mergeCounter) {
		_position = other->_position;
		return true;
	}
	return false;
}


void MapCommands::MoveObject::redo() {
	_previousPosition = _map.object(_objectId).pos();
	_map.moveObject(_objectId, _position);
}


void MapCommands::MoveObject::undo() {
	_map.moveObject(_objectId, _previousPosition);
}


MapCommands::NewObject::NewObject(Map &map, MapObject::id_t objectId, const MapObject &object,
                                  QUndoCommand *parent)
    : QUndoCommand("Place " + unitTypeS(object), parent) {
	new ModifyObject(map, objectId, object, this);
}


MapCommands::SetTile::SetTile(Map &map, const QPoint &pos, uint8_t tileNo, QUndoCommand *parent)
    : QUndoCommand("Set Tile", parent), _map(map), _pos(pos), _tileNo(tileNo) {}


void MapCommands::SetTile::redo() {
	_previousTileNo = _map.tileNo(_pos);
	_map.setTile(_pos, _tileNo);
}


void MapCommands::SetTile::undo() {
	_map.setTile(_pos, _previousTileNo);
}


MapCommands::FloodFill::FloodFill(Map &map, const QPoint &pos, uint8_t tileNo, QUndoCommand *parent)
    : QUndoCommand("Flood fill", parent), _map(map), _pos(pos), _tileNo(tileNo) {
	_previousTiles = new uint8_t[_map.width() * _map.height()];
}


MapCommands::FloodFill::~FloodFill() {
	delete[] _previousTiles;
}


void MapCommands::FloodFill::redo() {
	memcpy(_previousTiles, _map.tiles(), sizeof(_previousTiles[0]) * _map.width() * _map.height());
	_map.floodFill(_pos, _tileNo);
}


void MapCommands::FloodFill::undo() {
	_map.setTiles(_map.rect(), _previousTiles);
}
