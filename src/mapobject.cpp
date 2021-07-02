#include "mapobject.h"


MapObject::MapObject() : MapObject(UnitType::None) {}


MapObject::MapObject(UnitType unitType)
    : unitType(unitType), x(0), y(0), a(0), b(0), c(0), d(0), health(0) {
	// set default attributes
	switch (unitType) {
	case UnitType::None: break;
	case UnitType::Player: health = 12; break;
	case UnitType::HoverbotLR: health = 10; break;
	case UnitType::HoverbotUD: health = 10; break;
	case UnitType::HoverbotAttack: health = 10; break;
	case UnitType::Evilbot: health = 75; break;
	case UnitType::RollerbotUD: health = 20; break;
	case UnitType::RollerbotLR: health = 20; break;
	case UnitType::TransporterPad: a = 1; break;
	case UnitType::Door: b = 5; break;
	case UnitType::TrashCompactor: break;
	case UnitType::Elevator: b = 2; c = 1; d = 1; break;
	case UnitType::WaterRaft: break;
	case UnitType::Key: c = 1; d = 1; break;
	case UnitType::TimeBomb: a = 10; c = 1; d = 1; break;
	case UnitType::EMP: a = 10; c = 1; d = 1; break;
	case UnitType::Pistol: a = 10; c = 1; d = 1; break;
	case UnitType::PlasmaGun: a = 10; c = 1; d = 1; break;
	case UnitType::Medkit: a = 10; c = 1; d = 1; break;
	case UnitType::Magnet: a = 10; c = 1; d = 1; break;
	}
}


MapObject::Group MapObject::group() const {
	return group(unitType);
}


QPoint MapObject::pos() const {
	return QPoint(x, y);
}


MapObject::Group MapObject::group(MapObject::UnitType unitType) {
	switch (unitType) {
	case UnitType::None: return Group::Invalid;
	case UnitType::Player: return Group::Player;
	case UnitType::HoverbotLR:
	case UnitType::HoverbotUD:
	case UnitType::HoverbotAttack:
	case UnitType::Evilbot:
	case UnitType::RollerbotUD:
	case UnitType::RollerbotLR: return Group::Robots;
	case UnitType::TransporterPad:
	case UnitType::Door:
	case UnitType::TrashCompactor:
	case UnitType::Elevator:
	case UnitType::WaterRaft: return Group::MapFeatures;
	case UnitType::Key:
	case UnitType::TimeBomb:
	case UnitType::EMP:
	case UnitType::Pistol:
	case UnitType::PlasmaGun:
	case UnitType::Medkit:
	case UnitType::Magnet: return Group::HiddenObjects;
	}
}


const QString &MapObject::toString(MapObject::UnitType unitType) {
	static const QString none = QStringLiteral("none");
	static const QString player = QStringLiteral("player");
	static const QString hoverbotLR = QStringLiteral("hoverbot horizontal patrol");
    static const QString hoverbotUD = QStringLiteral("hoverbot vertical patrol");
    static const QString hoverbotAttack = QStringLiteral("hoverbot attack mode");
    static const QString evilbot = QStringLiteral("evilbot");
    static const QString rollerbotUD = QStringLiteral("rollerbot vertical patrol");
    static const QString rollerbotLR = QStringLiteral("rollerbot horizontal patrol");
    static const QString door = QStringLiteral("moving door");
    static const QString key = QStringLiteral("hidden key");
    static const QString timeBomb = QStringLiteral("time bomb" );
    static const QString emp = QStringLiteral("EMP");
    static const QString pistol = QStringLiteral("pistol");
    static const QString plasmaGun = QStringLiteral("plasma gun");
    static const QString medkit = QStringLiteral("medkit");
    static const QString magnet = QStringLiteral("magnet");
    static const QString transporterPad = QStringLiteral("transporter pad");
    static const QString trashCompactor = QStringLiteral("trash compactor");
    static const QString elevator = QStringLiteral("elevator");
    static const QString waterRaft = QStringLiteral("water raft");
	
	switch (unitType) {
	case UnitType::None: return none;
	case UnitType::Player: return player;
	case UnitType::HoverbotLR: return hoverbotLR;
	case UnitType::HoverbotUD: return hoverbotUD;
	case UnitType::HoverbotAttack: return hoverbotAttack;
	case UnitType::Evilbot: return evilbot;
	case UnitType::RollerbotUD: return rollerbotUD;
	case UnitType::RollerbotLR: return rollerbotLR;
	case UnitType::TransporterPad: return transporterPad;
	case UnitType::Door: return door;
	case UnitType::TrashCompactor: return trashCompactor;
	case UnitType::Elevator: return elevator;
	case UnitType::WaterRaft: return waterRaft;
	case UnitType::Key: return key;
	case UnitType::TimeBomb: return timeBomb;
	case UnitType::EMP: return emp;
	case UnitType::Pistol: return pistol;
	case UnitType::PlasmaGun: return plasmaGun;
	case UnitType::Medkit: return medkit;
	case UnitType::Magnet: return magnet;
	}
}


const QString &MapObject::toString(MapObject::Group group) {
	static const QString invalid = QStringLiteral("invalid");
	static const QString player = QStringLiteral("player");
	static const QString robots = QStringLiteral("robots");
	static const QString mapFeatures = QStringLiteral("map features");
	static const QString hiddenObjects = QStringLiteral("hidden objects");
	
	switch (group) {
	case Group::Invalid: return invalid;
	case Group::Player: return player;
	case Group::Robots: return robots;
	case Group::MapFeatures: return mapFeatures;
	case Group::HiddenObjects: return hiddenObjects;
	}
}


const std::forward_list<MapObject::UnitType> &MapObject::unitTypes() {
	static const std::forward_list<UnitType> result = {
		UnitType::None, UnitType::Player, UnitType::HoverbotLR, UnitType::HoverbotUD,
		UnitType::HoverbotAttack, UnitType::Evilbot, UnitType::RollerbotUD, UnitType::RollerbotLR,
		UnitType::TransporterPad, UnitType::Door, UnitType::TrashCompactor, UnitType::Elevator,
		UnitType::WaterRaft, UnitType::Key, UnitType::TimeBomb, UnitType::EMP, UnitType::Pistol,
		UnitType::PlasmaGun, UnitType::Medkit, UnitType::Magnet,
	};
	return result;
}
