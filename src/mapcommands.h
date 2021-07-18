#ifndef MAPCOMMANDS_H
#define MAPCOMMANDS_H


#include <QPoint>
#include <QUndoCommand>
#include "map.h"
#include "mapobject.h"


namespace MapCommands {

class DeleteObject : public QUndoCommand {
public:
	DeleteObject(Map &map, MapObject::id_t objectId, QUndoCommand *parent = nullptr);
	
	void redo() override;
	void undo() override;
private:
	Map &_map;
	const MapObject::id_t _objectId;
	MapObject _previousState[MapObject::IdMax + 1];
};


class ModifyObject : public QUndoCommand {
public:
	ModifyObject(Map &map, MapObject::id_t objectId, const MapObject &object, QUndoCommand *parent = nullptr);
	
	void redo() override;
	void undo() override;
	
private:
	void applyWaterRaftConstraints();
	
	Map &_map;
	const MapObject::id_t _objectId;
	MapObject _object;
	MapObject _previousObject;
};


class MoveObject : public QUndoCommand {
public:
	MoveObject(Map &map, MapObject::id_t objectId, const QPoint &pos, int mergeCounter,
	           QUndoCommand *parent = nullptr);
	
	int id() const override;
	bool mergeWith(const QUndoCommand *command) override;
	void redo() override;
	void undo() override;
	
private:
	void redoWaterRaft();
	void undoWaterRaft();
	
	Map &_map;
	const MapObject::id_t _objectId;
	QPoint _position;
	QPoint _previousPosition;
	uint8_t _previousA;
	uint8_t _previousB;
	uint8_t _previousC;
	int _mergeCounter;
};


class NewObject : public QUndoCommand {
public:
	NewObject(Map &map, MapObject::id_t objectId, const MapObject &object, QUndoCommand *parent = nullptr);
};


class FloodFill : public QUndoCommand {
public:
	FloodFill(Map &map, const QPoint &pos, uint8_t tileNo, QUndoCommand *parent = nullptr);
	virtual ~FloodFill();
	
	void redo() override;
	void undo() override;
	
private:
	Map &_map;
	const QPoint _pos;
	const uint8_t _tileNo;
	uint8_t *_previousTiles;
};


class SetTile : public QUndoCommand {
public:
	SetTile(Map &map, const QPoint &pos, uint8_t tileNo, QUndoCommand *parent = nullptr);
	
	void redo() override;
	void undo() override;
	
private:
	Map &_map;
	const QPoint _pos;
	const uint8_t _tileNo;
	uint8_t _previousTileNo;
};
} // namespace MapCommands

#endif // MAPCOMMANDS_H
