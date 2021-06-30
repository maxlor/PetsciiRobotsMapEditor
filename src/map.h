#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <QObject>
#include <QPoint>
#include <QString>
#include <list>
#include <utility>
#include "constants.h"
#include "mapobject.h"


class Map : public QObject {
	Q_OBJECT
public:
	Map(QObject *parent = nullptr);
	
	static constexpr int width() { return MAP_WIDTH; };
	static constexpr int height() { return MAP_HEIGHT; }
	
	void clear();
	QString load(const QString &path);
	
	int hiddenItemCount() const;
	int mapFeatureCount() const;
	int robotCount() const;
	
	int nextAvailableObjectId(MapObject::Kind kind) const;
	
	uint8_t tileNo(const QPoint &tile) const;
	const MapObject &object(int no) const;
	
	void setTile(const QPoint &position, int tileNo);
	void floodFill(const QPoint &position, uint8_t tileNo);
	void setObject(int objectNo, const MapObject &object);
	
	static const std::list<std::pair<uint8_t, QString> > &unitTypes();
	
	void compact();
	
	QByteArray data() const;
	
signals:
	void tilesChanged();
	void objectsChanged();
	
private:
	int recursiveFloodFill(const QPoint &position, uint8_t oldTile, uint8_t newTile);
	
	uint8_t _tiles[MAP_WIDTH * MAP_HEIGHT];
	MapObject _objects[64];
};

#endif // MAP_H
