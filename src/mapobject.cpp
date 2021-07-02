#include "mapobject.h"


MapObject::MapObject() : MapObject(UnitType::None) {}


MapObject::MapObject(UnitType unitType)
    : unitType(unitType), x(0), y(0), a(0), b(0), c(0), d(0), health(0) {}


MapObject::Kind MapObject::kind() const {
	return kind(unitType);
}


QPoint MapObject::pos() const {
	return QPoint(x, y);
}


MapObject::Kind MapObject::kind(MapObject::UnitType unitType) {
	switch (unitType) {
	case UnitType::None: return Kind::Invalid;
	case UnitType::Player: return Kind::Player;
	case UnitType::HoverbotLR:
	case UnitType::HoverbotUD:
	case UnitType::HoverbotAttack:
	case UnitType::Evilbot:
	case UnitType::RollerbotUD:
	case UnitType::RollerbotLR: return Kind::Robot;
	case UnitType::Transporter: return Kind::TransporterPad;
	case UnitType::Door: return Kind::Door;
	case UnitType::TrashCompactor: return Kind::TrashCompactor;
	case UnitType::Elevator: return Kind::Elevator;
	case UnitType::WaterRaft: return Kind::WaterRaft;
	case UnitType::Key: return Kind::Key;
	case UnitType::TimeBomb:
	case UnitType::EMP:
	case UnitType::Pistol:
	case UnitType::PlasmaGun:
	case UnitType::Medkit:
	case UnitType::Magnet: return Kind::HiddenObject;
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
	case Kind::TrashCompactor: return trashCompactor;
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
	case Kind::TrashCompactor: return mapObjects;
	case Kind::Elevator: return mapObjects;
	case Kind::WaterRaft: return mapObjects;
	case Kind::Key: return hiddenItems;
	case Kind::HiddenObject: return hiddenItems;
	case Kind::Invalid: Q_ASSERT(false); return invalid;
	}
}
