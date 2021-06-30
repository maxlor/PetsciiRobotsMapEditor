#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H

#include <QObject>
#include <QPoint>

class Map;
class MapObject;


class MapController : public QObject {
	Q_OBJECT
public:
	MapController(QObject *parent = nullptr);
	
	const Map *map();
	
	void clear();
	void deleteObject(int objectNo);
	QString load(const QString &path);
	void moveObject(int objectNo, const QPoint &position);
	void setObject(int objectNo, const MapObject &object);
	
	void beginTileDrawing();
	void endTileDrawing();
	void floodFill(const QPoint &position, uint8_t tileNo);
	void setTile(const QPoint &position, uint8_t tileNo);
	
private:
	Map *_map;
};

#endif // MAPCONTROLLER_H
