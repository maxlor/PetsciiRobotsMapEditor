#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H

#include <QObject>
#include <QPoint>
#include <QUndoStack>
#include "mapobject.h"

class Map;

class MapController : public QObject {
	Q_OBJECT
public:
	MapController(QObject *parent = nullptr);
	
	const Map *map();
	
	QAction *redoAction();
	QAction *undoAction();
	
	void clear();
	
	QString load(const QString &path);
	QString save(const QString &path);
	
	void deleteObject(MapObject::id_t objectId);
	void moveObject(MapObject::id_t objectId, const QPoint &position);
	MapObject::id_t newObject(const MapObject &object, QString *error);
	void setObject(MapObject::id_t objectId, const MapObject &object, bool isNew = false);
	void incrementMergeCounter();
	
	void beginUndoGroup();
	void endUndoGroup();
	void floodFill(const QPoint &position, uint8_t tileNo);
	void setTile(const QPoint &position, uint8_t tileNo);
	
private:
	Map *_map;
	QAction *_redoAction = nullptr;
	QAction *_undoAction = nullptr;
	QUndoStack _undoStack;
	int _mergeCounter = 0;
	bool _inMacro = false;
};

#endif // MAPCONTROLLER_H
