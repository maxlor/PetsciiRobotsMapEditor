#ifndef MAPCOMMANDS_H
#define MAPCOMMANDS_H


#include <QPoint>
#include <QUndoCommand>
#include "map.h"
#include "mapobject.h"


namespace MapCommands {

class DeleteObject : public QUndoCommand {
public:
	DeleteObject(Map &map, int objectNo, QUndoCommand *parent = nullptr);
	
	void redo() override;
	void undo() override;
private:
	Map &_map;
	const int _objectNo;
	MapObject _previousState[MapObject::IdMax + 1];
};


class ModifyObject : public QUndoCommand {
public:
	ModifyObject(Map &map, int objectNo, const MapObject &object, QUndoCommand *parent = nullptr);
	
	void redo() override;
	void undo() override;
	
private:
	Map &_map;
	const int _objectNo;
	const MapObject _object;
	MapObject _previousObject;
};


class MoveObject : public QUndoCommand {
public:
	MoveObject(Map &map, int objectNo, const QPoint &pos, int mergeCounter,
	           QUndoCommand *parent = nullptr);
	
	int id() const override;
	bool mergeWith(const QUndoCommand *command) override;
	void redo() override;
	void undo() override;
	
private:
	Map &_map;
	const int _objectNo;
	QPoint _position;
	QPoint _previousPosition;
	int _mergeCounter;
};


class NewObject : public QUndoCommand {
public:
	NewObject(Map &map, int objectNo, const MapObject &object, QUndoCommand *parent = nullptr);
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
