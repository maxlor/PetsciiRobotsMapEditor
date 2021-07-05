#include "mapcheck.h"
#include <QRect>
#include <unordered_set>
#include "map.h"
#include "mapcontroller.h"


MapCheck::MapCheck(MapController &mapController) : _mapController(mapController) {
	check();
}


void MapCheck::check() {
	_problems.clear();
	checkPlayerExists();
	checkPlayerInBounds();
	checkUnitTypes();
	checkHealth();
	checkDoors();
}


void MapCheck::fixAll() {
	bool fixed;
	do {
		fixed = false;
		
		for (const Problem &problem : _problems) {
			if (problem.fix) {
				problem.fix();
				check();
				fixed = true;
				break;
			}
		}
		
	} while (fixed);
}


void MapCheck::fixSilent() {
	for (const Problem &problem : _silentProblems) {
		problem.fix();
	}
	_silentProblems.clear();
}


const std::vector<MapCheck::Problem> &MapCheck::problems() const {
	return _problems;
}


const std::vector<MapCheck::Problem> &MapCheck::silentProblems() const {
	return _silentProblems;
}


void MapCheck::silent(MapObject::id_t id, const QString &text, std::function<void ()> fix) {
	_silentProblems.push_back(Problem{Severity::Silent, id, text, fix});
}


void MapCheck::info(MapObject::id_t id, const QString &text, std::function<void ()> fix) {
	addProblem(Severity::Info, id, text, fix);
}


void MapCheck::warn(MapObject::id_t id, const QString &text, std::function<void ()> fix) {
	addProblem(Severity::Warning, id, text, fix);
}


void MapCheck::error(MapObject::id_t id, const QString &text, std::function<void ()> fix) {
	addProblem(Severity::Error, id, text, fix);
}


void MapCheck::addProblem(MapCheck::Severity severity, MapObject::id_t id, const QString &text, std::function<void ()> fix) {
	_problems.push_back(Problem{severity, id, text, fix});
}


void MapCheck::checkPlayerExists() {
	const MapObject::ObjectId id = MapObject::IdPlayer;
	const MapObject &player = _mapController.map()->object(id);
	
	if (player.unitType == MapObject::UnitType::Player) {
		return;
	} else if (player.unitType == MapObject::UnitType::None) {
		error(id, "player spawn point not set");
	} else {
		error(id, "player object has wrong unit type", [=]() {
			MapObject object = _mapController.map()->object(MapObject::IdPlayer);
			object.unitType = MapObject::UnitType::Player;
			_mapController.setObject(MapObject::IdPlayer, object);
		});
	}
}


void MapCheck::checkPlayerInBounds() {
	const MapObject::ObjectId id = MapObject::IdPlayer;
	const MapObject &player = _mapController.map()->object(id);
	if (player.unitType == MapObject::UnitType::None) { return; }
	
	const QRect validPositions = walkableRect();
	if (not validPositions.contains(player.pos())) {
		error(id, "player spawn point is too close to the map border");
	}
}


void MapCheck::checkUnitTypes() {
	for (MapObject::id_t id = MapObject::IdMin; id <= MapObject::IdMax; ++id) {
		const MapObject &object = _mapController.map()->object(id);
		if (object.unitType == MapObject::UnitType::None) { continue; }
		const MapObject::Group expected = MapObject::group(id);
		const MapObject::Group actual = MapObject::group(object.unitType);
		if (expected != actual) {
			auto fix = [=]() {
				_mapController.deleteObject(id);
			};
			error(id, QString("object %1 should be of group %2 but is of group %3").arg(id)
			      .arg(MapObject::toString(expected), MapObject::toString(actual)), fix);
		}
	}
}


void MapCheck::checkHealth() {
	const MapObject::ObjectId playerId = MapObject::IdPlayer;
	const MapObject &player = _mapController.map()->object(playerId);
	
	auto fixPlayer = [=]() {
		MapObject object = _mapController.map()->object(MapObject::IdPlayer);
		object.health = 12;
		_mapController.setObject(MapObject::IdPlayer, object);
	};
	
	if (player.unitType == MapObject::UnitType::Player) {
		if (player.health == 0) {
			error(playerId, "player's health set to 0", fixPlayer);
		} else if (player.health > 12) {
			warn(playerId, "player's health is set to more than 12", fixPlayer);
		}
	}
	
	for (MapObject::id_t id = MapObject::IdRobotMin; id <= MapObject::IdRobotMax; ++id) {
		const MapObject &robot = _mapController.map()->object(id);
		
		if (robot.unitType != MapObject::UnitType::None and robot.health == 0) {
			auto fixRobot = [=]() {
				MapObject object = _mapController.map()->object(MapObject::IdPlayer);
				switch (object.unitType) {
				case MapObject::UnitType::HoverbotLR:
				case MapObject::UnitType::HoverbotUD:
				case MapObject::UnitType::HoverbotAttack: object.health = 10; break;
				case MapObject::UnitType::Evilbot: object.health = 75; break;
				case MapObject::UnitType::RollerbotUD:
				case MapObject::UnitType::RollerbotLR: object.health = 20; break;
				default: Q_ASSERT(false);
				}
			};
			
			warn(id, "robot's health is set to 0", fixRobot);
		}
	}
}


void MapCheck::checkDoors() {
	static const std::unordered_set<uint8_t> VERTICAL_DOOR_TILES = { 0x48, 0x49, 0x50 };
	static const std::unordered_set<uint8_t> HORIZONTAL_DOOR_TILES = { 0x51, 0x55, 0x59 };
	for (MapObject::id_t id = MapObject::IdMapFeatureMin; id <= MapObject::IdMapFeatureMax; ++id) {
		const MapObject &object = _mapController.map()->object(id);
		if (object.unitType != MapObject::UnitType::Door) { continue; }
		
		const uint8_t tileNo = _mapController.map()->tileNo(object.pos());
		if (HORIZONTAL_DOOR_TILES.find(tileNo) != HORIZONTAL_DOOR_TILES.end()) {
			if (object.a != 0) {
			}
		} else if (VERTICAL_DOOR_TILES.find(tileNo) != VERTICAL_DOOR_TILES.end()) {
			if (object.a != 1) {
				error(id, "door should be set to vertical, but isn't", setAttribute(id, A, 1));
			}
		} else {
			error(id, "door object is not placed on a door tile");
		}
		
		if (object.b > 5) {
			silent(id, "door state is invalid", setAttribute(id, B, 5));
		}
		
		if (object.c > 3) {
			silent(id, "door lock is invalid", setAttribute(id, C, 0));
		}
		
		if (object.d != 0) {
			silent(id, "door's unused attribute d should be 0", setAttribute(id, D, 0));
		}
		
		if (object.health != 0) {
			silent(id, "door's unused attribute health should be 0", setAttribute(id, HEALTH, 0));
		}
	}
}


QRect MapCheck::walkableRect() const {
	static const int HORIZONTAL_MARGIN = 5;
	static const int VERTICAL_MARGIN = 3;
	
	QRect result = _mapController.map()->rect();
	result.setLeft(result.left() + HORIZONTAL_MARGIN);
	result.setRight(result.right() - HORIZONTAL_MARGIN);
	result.setTop(result.top() + VERTICAL_MARGIN);
	result.setBottom(result.bottom() - VERTICAL_MARGIN);
	
	return result;
}


std::function<void ()> MapCheck::setAttribute(MapObject::id_t id, MapCheck::Attribute attribute, uint8_t value) {
	return [=]() {
		MapObject object = _mapController.map()->object(id);
		switch (attribute) {
		case A: object.a = value; break;
		case B: object.b = value; break;
		case C: object.c = value; break;
		case D: object.d = value; break;
		case HEALTH: object.health = value; break;
		default:;	
		}
		_mapController.setObject(id, object);
	};
}
