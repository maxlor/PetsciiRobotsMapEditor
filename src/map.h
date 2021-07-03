#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <QObject>
#include <QPoint>
#include <QString>
#include "mapobject.h"


class Map : public QObject {
	Q_OBJECT
	Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)
	Q_PROPERTY(QString path READ path NOTIFY pathChanged)
public:
	Map(QObject *parent = nullptr);
	virtual ~Map();
	
	int width() const;
	int height() const;
	QRect rect() const;
	
	void clear();
	QString load(const QString &path);
	QString save(const QString &path);
	
	int hiddenItemCount() const;
	int mapFeatureCount() const;
	int robotCount() const;
	
	MapObject::id_t nextAvailableObjectId(MapObject::Group group) const;
	
	const MapObject &object(MapObject::id_t no) const;
	void deleteObject(MapObject::id_t no);
	void moveObject(MapObject::id_t no, const QPoint &pos);
	void setObject(MapObject::id_t no, const MapObject &object);
	void setObjects(const MapObject objects[]);
	
	uint8_t tileNo(const QPoint &tile) const;
	uint8_t *tiles();
	void setTile(const QPoint &position, uint8_t tileNo);
	void setTiles(const QRect &rect, uint8_t *tiles);
	void floodFill(const QPoint &position, uint8_t tileNo);
	
	bool isModified() const;
	
	const QString &path() const;
	
	void compact();
	
	QByteArray data() const;
	
signals:
	void modifiedChanged();
	void objectsChanged();
	void pathChanged();
	void tilesChanged();
	
private slots:
	void setModifiedFlag();
	
private:
	MapObject &objectAt(MapObject::id_t no);
	int recursiveFloodFill(const QPoint &position, uint8_t oldTile, uint8_t newTile);
	void setModified(bool modified);
	void setPath(const QString &path);
	
	MapObject *_objects;
	uint8_t *_tiles;
	bool _modified = false;
	QString _path;
};

#endif // MAP_H
