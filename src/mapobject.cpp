#include "mapobject.h"
#include "constants.h"


MapObject::MapObject() : MapObject(UNITTYPE_NONE) {}


MapObject::MapObject(uint8_t unitType)
    : unitType(unitType), x(0), y(0), a(0), b(0), c(0), d(0), health(0) {}


MapObject::Kind MapObject::kind() const {
	return kind(unitType);
}


QPoint MapObject::pos() const {
	return QPoint(x, y);
}


MapObject::Kind MapObject::kind(uint8_t unitType) {
	switch (unitType) {
	case 1: return Kind::Player;
	case 2:
	case 3:
	case 4:
	case 9:
	case 17:
	case 18: return Kind::Robot;
	case 7: return Kind::TransporterPad;
	case 10: return Kind::Door;
	case 16: return Kind::TrashCompator;
	case 19: return Kind::Elevator;
	case 22: return Kind::WaterRaft;
	case 128: return Kind::Key;
	case 129:
	case 130:
	case 131:
	case 132:
	case 133:
	case 134: return Kind::HiddenObject;
	default: return Kind::Invalid;
	}
}


const QString &MapObject::toString(MapObject::Kind kind) {
	static const QString player = "player";
	static const QString robots = "robot";
	static const QString transporterPad = "transporter pad";
	static const QString door = "door";
	static const QString trashCompactor = "trash compactor";
	static const QString elevator = "elevator";
	static const QString waterRaft = "water raft";
	static const QString key = "key";
	static const QString hiddenObject = "item";
	static const QString invalid = "invalid";
	switch (kind) {
	case Kind::Player: return player;
	case Kind::Robot: return robots;
	case Kind::TransporterPad: return transporterPad;
	case Kind::Door: return door;
	case Kind::TrashCompator: return trashCompactor;
	case Kind::Elevator: return elevator;
	case Kind::WaterRaft: return waterRaft;
	case Kind::Key: return key;
	case Kind::HiddenObject: return hiddenObject;
	case Kind::Invalid: return invalid;
	}
}


const QString &MapObject::category(MapObject::Kind kind) {
	static const QString player = "player";
	static const QString robots = "robots";
	static const QString mapObjects = "map features";
	static const QString hiddenItems = "hidden items";
	static const QString invalid = "invalid";
	switch (kind) {
	case Kind::Player: return player;
	case Kind::Robot: return robots;
	case Kind::TransporterPad: return mapObjects;
	case Kind::Door: return mapObjects;
	case Kind::TrashCompator: return mapObjects;
	case Kind::Elevator: return mapObjects;
	case Kind::WaterRaft: return mapObjects;
	case Kind::Key: return hiddenItems;
	case Kind::HiddenObject: return hiddenItems;
	case Kind::Invalid: Q_ASSERT(false); return invalid;
	}
}
