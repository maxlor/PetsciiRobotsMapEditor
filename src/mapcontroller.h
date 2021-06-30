#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H

#include <QObject>
#include <QPoint>
#include <QUndoStack>

class Map;
class MapObject;


class MapController : public QObject {
	Q_OBJECT
public:
	MapController(QObject *parent = nullptr);
	
	const Map *map();
	
	void clear();
	QString load(const QString &path);
	QString save(const QString &path);
	
	void deleteObject(int objectNo);
	void moveObject(int objectNo, const QPoint &position);
	void setObject(int objectNo, const MapObject &object);
	
	void beginTileDrawing();
	void endTileDrawing();
	void floodFill(const QPoint &position, uint8_t tileNo);
	void setTile(const QPoint &position, uint8_t tileNo);
	
private:
	Map *_map;
	QUndoStack _undoStack;
};

#endif // MAPCONTROLLER_H
