#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <QObject>
#include <QString>
#include <list>
#include <utility>
#include "constants.h"


class Map : public QObject {
	Q_OBJECT
public:
	struct Object {
		enum class Kind {
			Player, Robot, TransporterPad, Door, TrashCompator, Elevator, WaterRaft, Key,
			HiddenObject, Invalid
		};
		
		Object();
		Object(uint8_t unitType);
		
		uint8_t unitType;
		uint8_t x;
		uint8_t y;
		uint8_t a;
		uint8_t b;
		uint8_t c;
		uint8_t d;
		uint8_t health;
		
		Kind kind() const;
		static Kind kind(uint8_t unitType);
		static const QString &toString(Kind kind);
		static const QString &category(Kind kind);
	};
	
	Map(QObject *parent = nullptr);
	
	static constexpr int width() { return MAP_WIDTH; };
	static constexpr int height() { return MAP_HEIGHT; }
	
	int hiddenItemCount() const;
	int mapFeatureCount() const;
	int robotCount() const;
	
	int nextAvailableObjectId(Object::Kind kind) const;
	
	int tileNo(int x, int y) const;
	const Object &object(int no) const;
	
	void setTile(int x, int y, int tileNo);
	void setObject(int objectNo, const Object &object);
	
	static const std::list<std::pair<uint8_t, QString> > &unitTypes();
	
	void compact();
	
	QByteArray data();
	
signals:
	void tilesChanged();
	void objectsChanged();
	
private:
	QString readMap(const QString &path);
	
	uint8_t _tiles[MAP_WIDTH * MAP_HEIGHT];
	Object _objects[64];
};

#endif // MAP_H
