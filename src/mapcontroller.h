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
	
	void deleteObject(int objectNo);
	void moveObject(int objectNo, const QPoint &position);
	int newObject(const MapObject &object, QString *error);
	void setObject(int objectNo, const MapObject &object, bool isNew = false);
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
};

#endif // MAPCONTROLLER_H
