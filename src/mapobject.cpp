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
	static const std::unordered_map<Kind, QString> names = {
	    { Kind::Player, "player" }, { Kind::Robot, "robot" },
	    { Kind::TransporterPad, "transporter pad" }, { Kind::Elevator, "elevator" },
	    { Kind::WaterRaft, "water raft" }, { Kind::Key, "key" },
	    { Kind::HiddenObject, "hidden object" }, { Kind::Invalid, "invalid" }
	};
	
	auto it = names.find(kind);
	Q_ASSERT(it != names.end());
	return it->second;
}


const QString &MapObject::category(MapObject::Kind kind) {
	static const QString mapObjects = "map features";
	static const QString hiddenItems = "hidden items";
	static const std::unordered_map<Kind, QString> names = {
	    { Kind::Player, "player" }, { Kind::Robot, "robots" },
	    { Kind::TransporterPad, mapObjects }, { Kind::Elevator, mapObjects },
	    { Kind::WaterRaft, mapObjects }, { Kind::Key, hiddenItems },
	    { Kind::HiddenObject, hiddenItems }, { Kind::Invalid, "invalid" }
	};
	
	auto it = names.find(kind);
	Q_ASSERT(it != names.end());
	return it->second;
}
