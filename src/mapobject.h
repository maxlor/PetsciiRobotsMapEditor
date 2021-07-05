#ifndef MAPOBJECT_H
#define MAPOBJECT_H

#include <QPoint>
#include <QString>
#include <cstdint>
#include <forward_list>


class MapObject {
public:
	typedef uint8_t unitType_t;
	typedef int id_t;
	
	enum class Group {
		Invalid, Player, Robots, MapFeatures, HiddenObjects
	};
	
	enum class UnitType : unitType_t {
		None = 0, Player = 1, HoverbotLR = 2, HoverbotUD = 3, HoverbotAttack = 4,
		TransporterPad = 7, Evilbot = 9, Door = 10, TrashCompactor = 16, RollerbotUD = 17,
		RollerbotLR = 18, Elevator = 19, WaterRaft = 22, Key = 128, TimeBomb = 129,
		EMP = 130, Pistol = 131, PlasmaGun = 132, Medkit = 133, Magnet = 134
	};
	
	enum ObjectId : id_t {
		IdNone = -1, IdPlayer = 0, IdMin = 0, IdRobotMin = 1, IdRobotMax = 28, IdMapFeatureMin = 32,
		IdMapFeatureMax = 47, IdHiddenMin = 48, IdHiddenMax = 63, IdMax = 63
	};
	
	MapObject();
	MapObject(UnitType unitType);
	
	UnitType unitType;
	uint8_t x;
	uint8_t y;
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t health;
	
	Group group() const;
	QPoint pos() const;
	static Group group(UnitType unitType);
	static Group group(id_t objectId);
	static const QString &toString(UnitType unitType);
	static const QString &toString(Group group);
	static const std::forward_list<UnitType> &unitTypes();
};

#endif // MAPOBJECT_H
