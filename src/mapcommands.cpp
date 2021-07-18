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
      _object(object) {
	if (object.unitType == MapObject::UnitType::WaterRaft) {
		applyWaterRaftConstraints();
	}
}


void MapCommands::ModifyObject::redo() {
	_previousObject = _map.object(_objectId);
	_map.setObject(_objectId, _object);
}


void MapCommands::ModifyObject::undo() {
	_map.setObject(_objectId, _previousObject);
}


void MapCommands::ModifyObject::applyWaterRaftConstraints() {
	Q_ASSERT(_object.unitType == MapObject::UnitType::WaterRaft);
	int offB = int(_object.b) - _object.x;
	int offC = int(_object.c) - _object.x;
	// If the new position is right at the map border, force the direction
	// towards the map.
	if (_object.x == 0) {
		_object.a = 1;
	} else if (_object.x == _map.width() - 1) {
		_object.a = 0;
	}
	if (_object.a == 0) {
		offB = qMin(-1, offB);
		offC = qMax(0, offC);
	} else {
		offB = qMin(0, offB);
		offC = qMax(1, offC);
	}
	_object.b = qMax(0, _object.x + offB);
	_object.c = qMin(_map.width() - 1, _object.x + offC);
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
	if (_map.object(_objectId).unitType == MapObject::UnitType::WaterRaft) {
		redoWaterRaft();
	} else {
		_previousPosition = _map.object(_objectId).pos();
		_map.moveObject(_objectId, _position);
	}
}


void MapCommands::MoveObject::undo() {
	if (_map.object(_objectId).unitType == MapObject::UnitType::WaterRaft) {
		undoWaterRaft();
	} else {
		_map.moveObject(_objectId, _previousPosition);
	}
}


void MapCommands::MoveObject::redoWaterRaft() {
	MapObject object = _map.object(_objectId);
	_previousPosition = object.pos();
	_previousA = object.a;
	_previousB = object.b;
	_previousC = object.c;
	int offB = _previousB - _previousPosition.x();
	int offC = _previousC - _previousPosition.x();
	object.x = _position.x();
	object.y = _position.y();
	// If the new position is right at the map border, force the direction
	// towards the map.
	if (_position.x() == 0) {
		object.a = 1;
	} else if (_position.x() == _map.width() - 1) {
		object.a = 0;
	}
	if (object.a == 0) {
		offB = qMin(-1, offB);
		offC = qMax(0, offC);
	} else {
		offB = qMin(0, offB);
		offC = qMax(1, offC);
	}
	object.b = qMax(0, _position.x() + offB);
	object.c = qMin(_map.width() - 1, _position.x() + offC);
	_map.setObject(_objectId, object);
}


void MapCommands::MoveObject::undoWaterRaft() {
	MapObject object = _map.object(_objectId);
	object.x = _previousPosition.x();
	object.y = _previousPosition.y();
	object.a = _previousA;
	object.b = _previousB;
	object.c = _previousC;
	_map.setObject(_objectId, object);
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
