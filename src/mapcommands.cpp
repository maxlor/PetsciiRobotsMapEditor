#include "mapcommands.h"
#include <cstring>


static QString kindS(const Map &map, int objectNo) {
	return MapObject::toString(map.object(objectNo).kind());
}


MapCommands::DeleteObject::DeleteObject(Map &map, int objectNo, QUndoCommand *parent)
    : QUndoCommand("Delete " + kindS(map, objectNo), parent), _map(map), _objectNo(objectNo) {}


void MapCommands::DeleteObject::redo() {
	for (int i = OBJECT_MIN; i <= OBJECT_MAX; ++i) {
		_previousState[i] = _map.object(i);
	}
	_map.deleteObject(_objectNo);
}


void MapCommands::DeleteObject::undo() {
	_map.setObjects(_previousState);
}


MapCommands::ModifyObject::ModifyObject(Map &map, int objectNo, const MapObject &object,
                                        QUndoCommand *parent)
    : QUndoCommand("Modify " + kindS(map, objectNo), parent), _map(map), _objectNo(objectNo),
      _object(object) {}


void MapCommands::ModifyObject::redo() {
	_previousObject = _map.object(_objectNo);
	_map.setObject(_objectNo, _object);
}


void MapCommands::ModifyObject::undo() {
	_map.setObject(_objectNo, _previousObject);
}


MapCommands::MoveObject::MoveObject(Map &map, int objectNo, const QPoint &pos, QUndoCommand *parent)
    : QUndoCommand("Move " + kindS(map, objectNo), parent), _map(map), _objectNo(objectNo),
      _position(pos) {}


void MapCommands::MoveObject::redo() {
	_map.moveObject(_objectNo, _position);
}


void MapCommands::MoveObject::undo() {
	_map.moveObject(_objectNo, _previousPosition);
}


MapCommands::NewObject::NewObject(Map &map, int objectNo, const MapObject &object,
                                  QUndoCommand *parent)
    : QUndoCommand("New " + kindS(map, objectNo), parent) {
	new ModifyObject(map, objectNo, object, this);
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
    : QUndoCommand("Flood fill", parent), _map(map), _pos(pos), _tileNo(tileNo) {}


void MapCommands::FloodFill::redo() {
	memcpy(_previousTiles, _map.tiles(), sizeof(_previousTiles));
	_map.floodFill(_pos, _tileNo);
}


void MapCommands::FloodFill::undo() {
	_map.setTiles(QRect(0, 0, MAP_WIDTH, MAP_HEIGHT), _previousTiles);
}
