#ifndef MAPOBJECT_H
#define MAPOBJECT_H

#include <cstdint>
#include <QPoint>
#include <QString>


class MapObject {
public:
	enum class Kind {
		Player, Robot, TransporterPad, Door, TrashCompator, Elevator, WaterRaft, Key,
		HiddenObject, Invalid
	};
	
	MapObject();
	MapObject(uint8_t unitType);
	
	uint8_t unitType;
	uint8_t x;
	uint8_t y;
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t health;
	
	Kind kind() const;
	QPoint pos() const;
	static Kind kind(uint8_t unitType);
	static const QString &toString(Kind kind);
	static const QString &category(Kind kind);
};

#endif // MAPOBJECT_H
