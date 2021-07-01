#include "mapcontroller.h"
#include <QAction>
#include <QKeySequence>
#include "map.h"
#include "mapcommands.h"
#include "mapobject.h"


static MapObject defaultObject(MapObject::Kind kind, const QPoint &pos);



MapController::MapController(QObject *parent) : QObject(parent) {
	_map = new Map(this);
	_undoStack.setUndoLimit(64);
}


QAction *MapController::redoAction() {
	if (_redoAction == nullptr) {
		_redoAction = _undoStack.createRedoAction(this);
		_redoAction->setShortcut(QKeySequence::Redo);
		_redoAction->setIcon(QIcon(":/redo.svg"));
	}
	return _redoAction;
}


QAction *MapController::undoAction() {
	if (_undoAction == nullptr) {
		_undoAction = _undoStack.createUndoAction(this);
		_undoAction->setShortcut(QKeySequence::Undo);
		_undoAction->setIcon(QIcon(":/undo.svg"));
	}
	return _undoAction;
}


const Map *MapController::map() {
	return _map;
}


void MapController::clear() {
	_map->clear();
	_undoStack.clear();
}


QString MapController::load(const QString &path) {
	QString result = _map->load(path);
	if (result.isNull()) {
		_undoStack.clear();
	}
	return result;
}


QString MapController::save(const QString &path) {
	QString result = _map->save(path);
	if (result.isNull()) {
		_undoStack.setClean();
	}
	return result;
}


void MapController::deleteObject(int objectNo) {
	Q_ASSERT(OBJECT_MIN <= objectNo and objectNo <= OBJECT_MAX);
	_undoStack.push(new MapCommands::DeleteObject(*_map, objectNo));
}


void MapController::moveObject(int objectNo, const QPoint &position) {
	_undoStack.push(new MapCommands::MoveObject(*_map, objectNo, position, _mergeCounter));
}


int MapController::newObject(MapObject::Kind kind, const QPoint &position, QString *error) {
	int no = _map->nextAvailableObjectId(kind);
	if (no == OBJECT_NONE) {
		if (error) {
			const QString &category = MapObject::category(kind);
			*error = QString("the map already contais the maximum number of " + category);
			return OBJECT_NONE;
		}
	}
	
	const MapObject object = defaultObject(kind, position);
	_undoStack.push(new MapCommands::NewObject(*_map, no, object));
	
	return no;
}


void MapController::setObject(int objectNo, const MapObject &object, bool isNew) {
	if (isNew) {
		_undoStack.push(new MapCommands::NewObject(*_map, objectNo, object));
	} else {
		_undoStack.push(new MapCommands::ModifyObject(*_map, objectNo, object));
	}
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


void MapController::incrementMergeCounter() {
	++_mergeCounter;
}


MapObject defaultObject(MapObject::Kind kind, const QPoint &pos) {
	MapObject result;
	switch (kind) {
	case MapObject::Kind::Player:
		result.unitType = UNITTYPE_PLAYER;
		result.health = 12;
		break;
	case MapObject::Kind:: Robot:
		result.unitType = ROBOT_HOVERBOT_LR;
		result.health = 10;
		break;
	case MapObject::Kind::TransporterPad:
		result.unitType = OBJECT_TRANSPORTER;
		result.a = 1;
		break;
	case MapObject::Kind::Door:
		result.unitType = OBJECT_DOOR;
		result.b = 5;
		break;
	case MapObject::Kind::TrashCompactor:
		result.unitType = OBJECT_TRASH_COMPACTOR;
		break;
	case MapObject::Kind::Elevator:
		result.unitType = OBJECT_ELEVATOR;
		result.b = 2;
		result.c = 1;
		result.d = 1;
		break;
	case MapObject::Kind::WaterRaft:
		result.unitType = OBJECT_WATER_RAFT;
		result.c = pos.x() - 1;
		result.d = pos.x() + 1;
		break;
	case MapObject::Kind::Key:
		result.unitType = OBJECT_KEY;
		result.c = 1;
		result.d = 1;
		break;
	case MapObject::Kind::HiddenObject:
		result.unitType = OBJECT_TIME_BOMB;
		result.a = 10;
		break;
	case MapObject::Kind::Invalid:
		Q_ASSERT(false);
		break;
	}
	result.x = pos.x();
	result.y = pos.y();
	
	return result;
}
