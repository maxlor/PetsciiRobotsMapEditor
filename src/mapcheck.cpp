#include "mapcheck.h"
#include <QLoggingCategory>
#include <QRect>
#include <unordered_set>
#include "map.h"
#include "mapcontroller.h"
#include "tileset.h"
#include "tile.h"

static Q_LOGGING_CATEGORY(lc, "mapcheck");


MapCheck::MapCheck(MapController &mapController, const Tileset &tileset)
    : _mapController(mapController), _tileset(tileset) {
	check();
}


void MapCheck::check() {
	_problems.clear();
	checkPlayerExists();
	checkPlayerInBounds();
	checkUnitTypes();
	checkPlayerAndRobotsHealthABCD();
	checkPlayerAndRobotsOnWalkable();
	checkPositionBounds();
	checkTransporterPads();
	checkDoors();
	checkTrashCompactors();
	checkWaterRafts();
	checkKeys();
	checkWeapons();
	checkSearchAreas();
}


void MapCheck::fixAll() {
	bool fixed;
	do {
		fixed = false;
		
		for (const Problem &problem : _problems) {
			if (problem.fix) {
				qCInfo(lc).noquote() << "fixing:" << problem.text;
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
		qCInfo(lc).noquote() << QString("fixing object %1 silently: %2").arg(problem.objectId)
		                        .arg(problem.text);
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
	Q_ASSERT(fix);
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


void MapCheck::checkPlayerAndRobotsHealthABCD() {
	// check health
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
	
	checkUnused(MapObject::IdPlayer, true, true, true, true, false);
	for (MapObject::id_t id = MapObject::IdRobotMin; id <= MapObject::IdRobotMax; ++id) {
		checkUnused(id, true, true, true, true, false);
	}
}


void MapCheck::checkPlayerAndRobotsOnWalkable() {
	Q_ASSERT(MapObject::IdPlayer + 1 == MapObject::IdRobotMin);
	const Map &map = *_mapController.map();
	for (MapObject::id_t id = MapObject::IdPlayer; id <= MapObject::IdRobotMax; ++id) {
		const MapObject &object = map.object(id);
		if (id != MapObject::IdPlayer and object.group() != MapObject::Group::Robots) { continue; }
		
		bool hoverbot = false;
		switch (object.unitType) {
		case MapObject::UnitType::HoverbotLR:
		case MapObject::UnitType::HoverbotUD:
		case MapObject::UnitType::HoverbotAttack: hoverbot = true; break;
		default:;
		}
		
		const Tile tile = _tileset.tile(map.tileNo(object.pos()));
		const QString unitType = MapObject::toString(object.unitType).split(' ').at(0);
		if (hoverbot) {
			if (not tile.attributes().testFlag(Tile::Hoverable)) {
				warn(id, unitType + " is spawning on a tile that isn't hoverable");
			}
		} else if (not tile.attributes().testFlag(Tile::Walkable)) {
			warn(id, unitType + " is spawning on a tile that isn't walkable");
		}
	}
}


void MapCheck::checkPositionBounds() {
	const uint8_t maxX = _mapController.map()->width() - 1;
	const uint8_t maxY = _mapController.map()->height() - 1;
	
	for (MapObject::id_t id = MapObject::IdMin; id <= MapObject::IdMax; ++id) {
		if (id == MapObject::IdPlayer) { continue; }
		const MapObject &object = _mapController.map()->object(id);
		if (object.unitType == MapObject::UnitType::None) {
			if (object.x != 0) {
				silent(id, QString("null objects should have x position 0, not %1").arg(object.x),
				       setAttribute(id, X, 0));
			}
			if (object.y != 0) {
				silent(id, QString("null objects should have y position 0, not %1").arg(object.y),
				       setAttribute(id, Y, 0));
			}
			continue;
		}
		
		if (object.x > maxX) {
			silent(id, QString("object %1 (%2) has an x-position that is out of bounds")
			       .arg(id).arg(MapObject::toString(object.unitType)), setAttribute(id, X, maxX));
		}
		if (object.y > maxY) {
			silent(id, QString("object %1 (%2) has an x-position that is out of bounds")
			       .arg(id).arg(MapObject::toString(object.unitType)), setAttribute(id, Y, maxY));
		}
	}
}


void MapCheck::checkTransporterPads() {
	bool foundLevelExit = false;
	const Map &map = *_mapController.map();
	for (MapObject::id_t id = MapObject::IdMapFeatureMin; id <= MapObject::IdMapFeatureMax; ++id) {
		const MapObject &object = map.object(id);
		if (object.unitType != MapObject::UnitType::TransporterPad) { continue; }
		
		if (object.a > 1) {
			silent(id, "transported pad's activation invalid", setAttribute(id, A, 1));
		}
		int b = object.b;
		if (object.b > 1) {
			silent(id, "transporter pad's destination invalid", setAttribute(id, B, 0));
			b = 0;
		}
		if (b == 0) {
			foundLevelExit = true;
		} else if (b == 1) {
			if (object.c >= map.width() or object.d >= map.height()) {
				error(id, "transporter pad's destination coordinates are invalid");
			}
			
			const Tile tile = _tileset.tile(map.tileNo(QPoint(object.c, object.d)));
			if (not tile.attributes().testFlag(Tile::Walkable)) {
				error(id, "transporter pad's destination tile is not walkable");
			}
		}
	}
	
	if (not foundLevelExit) {
		error(MapObject::IdNone, "there is no transporter pad serving as a level exit");
	}
}


void MapCheck::checkDoors() {
	static const uint8_t EITHER_DOOR_TILE = 0x09;
	static const std::unordered_set<uint8_t> VERTICAL_DOOR_TILES = { 0x09, 0x48, 0x49, 0x50 };
	static const std::unordered_set<uint8_t> HORIZONTAL_DOOR_TILES = { 0x09, 0x51, 0x55, 0x59 };
	
	for (MapObject::id_t id = MapObject::IdMapFeatureMin; id <= MapObject::IdMapFeatureMax; ++id) {
		const MapObject &object = _mapController.map()->object(id);
		const bool isDoor = object.unitType == MapObject::UnitType::Door;
		const bool isElevator = object.unitType == MapObject::UnitType::Elevator;
		if (not (isDoor or isElevator)) { continue; }
		
		const uint8_t tileNo = _mapController.map()->tileNo(object.pos());
		if (isDoor) {
			if (HORIZONTAL_DOOR_TILES.find(tileNo) != HORIZONTAL_DOOR_TILES.end()) {
				if (object.a != 0 and tileNo != EITHER_DOOR_TILE) {
					error(id, "door should be set to horizontal, but isn't", setAttribute(id, A, 0));
				}
			} else if (VERTICAL_DOOR_TILES.find(tileNo) != VERTICAL_DOOR_TILES.end()) {
				if (object.a != 1 and tileNo != EITHER_DOOR_TILE) {
					error(id, "door should be set to vertical, but isn't", setAttribute(id, A, 1));
				}
			} else {
				error(id, "door object is not placed on a door tile");
			}
		} else {
			Q_ASSERT(isElevator);
			if (HORIZONTAL_DOOR_TILES.find(tileNo) == HORIZONTAL_DOOR_TILES.end()) {
				error(id, "elevator object is not placed on a horizontal door tile");
			}
		}
		
		if (object.b > 5) {
			silent(id, "door/elevator state is invalid", setAttribute(id, B, 5));
		}
		
		if (isDoor) {
			if (object.c > 3) {
				silent(id, "door lock is invalid", setAttribute(id, C, 0));
			}
			checkUnused(id, false, false, false, true, true);
		} else {
			Q_ASSERT(isElevator);
			uint8_t c = object.c;
			if (c < 1) {
				error(id, "elevator's current floor is smaller than 1", setAttribute(id, C, 1));
				c = 1;
			}
			if (object.d < c) {
				error(id, "elevator's top floor is smaller than the current floor",
				       setAttribute(id, D, c));
			}
			checkUnused(id, false, false, false, false, true);
		}
	}
}


void MapCheck::checkTrashCompactors() {
	const uint8_t maxX = _mapController.map()->width() - 2;
	const uint8_t minY = 1;
	
	static const uint8_t tcTiles[] = { 0x90, 0x91, 0x94, 0x94, };
	static const uint8_t tcWidth = 2;
	
	for (MapObject::id_t id = MapObject::IdMapFeatureMin; id <= MapObject::IdMapFeatureMax; ++id) {
		const MapObject &object = _mapController.map()->object(id);
		if (object.unitType != MapObject::UnitType::TrashCompactor) { continue; }
	
		if (object.x > maxX or object.y < minY) {
			error(id, "trash compactor is too close to the map border");
			continue;
		}
		
		for (size_t i = 0; i < sizeof(tcTiles) / sizeof(tcTiles[0]); ++i) {
			uint8_t my = i / tcWidth;
			uint8_t mx = i - my * tcWidth;
			uint8_t expectedTileNo = tcTiles[i];
			uint8_t tileNo = _mapController.map()->tileNo({object.x + mx, object.y + my - 1});
			if (tileNo != expectedTileNo) {
				error(id, "trash compactor tiles are not set up correctly", [=]() {
					for (size_t i = 0; i < sizeof(tcTiles) / sizeof(tcTiles[0]); ++i) {
						uint8_t my = i / tcWidth;
						uint8_t mx = i - my * tcWidth;
						_mapController.setTile({object.x + mx, object.y + my - 1}, tcTiles[i]);
					}
				});
				break;
			}
		}
		
		checkUnused(id, true, true, true, true, true);
	}
}


void MapCheck::checkWaterRafts() {
	for (MapObject::id_t id = MapObject::IdMapFeatureMin; id <= MapObject::IdMapFeatureMax; ++id) {
		const MapObject &object = _mapController.map()->object(id);
		if (object.unitType != MapObject::UnitType::WaterRaft) { continue; }
		
		uint8_t a = object.a;
		if (a > 1) {
			silent(id, "water raft's direction must be 0 or 1", setAttribute(id, A, 0));
			a = 0;
		}
		
		if (a == 0 and object.b >= object.x) {
			error(id, "water raft's left stop must be left of its current position",
			      setAttribute(id, B, object.x - 1));
		} else if (a == 1 and object.c <= object.x) {
			error(id, "water raft's right stop must be right of its current position",
			      setAttribute(id, C, object.x + 1));
		}
		uint8_t c = object.c;
		if (object.b == _mapController.map()->width() - 1) {
			error(id, "water raft's right stop cannot be at the right map border",
			      setAttribute(id, B, _mapController.map()->width() - 2));
		}
		if (c == 0) {
			error(id, "water raft's right stop cannot be at the left map border",
			      setAttribute(id, C, 1));
			c = 1;
		}
		if (object.b >= object.c) {
			error(id, "water raft's left stop must be left of its right stop",
			      setAttribute(id, B, c - 1));
		}
		
		checkUnused(id, false, false, false, true, true);
	}
}


void MapCheck::checkKeys() {
	bool haveSpadeDoor = false;
	bool haveHeartDoor = false;
	bool haveStarDoor = false;
	
	for (MapObject::id_t id = MapObject::IdMapFeatureMin; id <= MapObject::IdMapFeatureMax; ++id) {
		const MapObject &door = _mapController.map()->object(id);
		if (door.unitType != MapObject::UnitType::Door) { continue; }
		switch (door.c) {
		case 1: haveSpadeDoor = true; break;
		case 2: haveHeartDoor = true; break;
		case 3: haveStarDoor = true; break;
		default:;
		}
	}
	
	for (MapObject::id_t id = MapObject::IdMapFeatureMin; id <= MapObject::IdMapFeatureMax; ++id) {
		const MapObject &object = _mapController.map()->object(id);
		if (object.unitType != MapObject::UnitType::Key) { continue; }

		
		switch (object.a) {
		case 0:
			if (not haveSpadeDoor) {
				warn(id, "key unlocks spade doors, but there is no such door");
			} break;
		case 1:
			if (not haveHeartDoor) {
				warn(id, "key unlocks heart doors, but there is no such door");
			} break;
		case 2:
			if (not haveStarDoor) {
				warn(id, "key unlocks star doors, but there is no such door");
			} break;
		default:
			silent(id, "key's attribute A is invalid", setAttribute(id, A, 0));
		}
		
		checkUnused(id, false, true, false, false, true);
	}
}


void MapCheck::checkWeapons() {
	for (MapObject::id_t id = MapObject::IdMapFeatureMin; id <= MapObject::IdMapFeatureMax; ++id) {
		const MapObject &object = _mapController.map()->object(id);
		if (object.group() != MapObject::Group::HiddenObjects or
		        object.unitType == MapObject::UnitType::Key) {
			continue;
		}
		
		if (object.a == 0) {
			error(id, "item quantity is set to 0", setAttribute(id, A, 10));
		}
		
		checkUnused(id, false, true, false, false, true);
	}
}


void MapCheck::checkSearchAreas() {
	const Map &map = *_mapController.map();
	static const std::unordered_set<uint8_t> neverExtendTiles = {
	    0x29, 0x2d, 0xc7, 0xca, 0xcb
	};
	for (MapObject::id_t id = MapObject::IdHiddenMin; id <= MapObject::IdHiddenMax; ++id) {
		const MapObject &object = map.object(id);
		if (object.group() != MapObject::Group::HiddenObjects) { continue; }
		
		QString unitType = MapObject::toString(object.unitType);
		const uint8_t tileNo = map.tileNo(object.pos());
		const Tile tile = _tileset.tile(tileNo);
		if (not tile.attributes().testFlag(Tile::Searchable)) {
			warn(id, "item is not on a searchable tile");
			continue;
		}
		
		if (neverExtendTiles.find(tileNo) != neverExtendTiles.end()) {
			if (object.c != 0) {
				warn(id, QString("%1's container width should be set to 1").arg(unitType),
				     setAttribute(id, C, 0));
			}
			if (object.d != 0) {
				warn(id, QString("%1's container height should be set to 1").arg(unitType),
				     setAttribute(id, D, 0));
			}
			continue;
		}
		
		uint8_t extendH = 0;
		uint8_t extendV = 0;
		for (; object.x + extendH < map.width() - 1; ++extendH) {
			const Tile nextTile = _tileset.tile(map.tileNo(object.pos() + QPoint(extendH + 1, 0)));
			if (not nextTile.attributes().testFlag(Tile::Searchable)) {
				break;
			}
		}
		for (; object.y + extendV < map.height() - 1; ++extendV) {
			const Tile nextTile = _tileset.tile(map.tileNo(object.pos() + QPoint(0, extendV + 1)));
			if (not nextTile.attributes().testFlag(Tile::Searchable)) {
				break;
			}
		}
		if (extendH > object.c) {
			info(id, QString("maybe set %1's container width to %2?").arg(unitType)
			     .arg(extendH + 1));
		}
		if (extendV > object.d) {
			info(id, QString("maybe set %1's vertical search area to %2?").arg(unitType)
			     .arg(extendV + 1));
		}
	}
}


void MapCheck::checkUnused(MapObject::id_t id, bool a, bool b, bool c, bool d, bool health) {
	const MapObject &object = _mapController.map()->object(id);
	const QString unitType = MapObject::toString(object.unitType);
	
	auto checkZero = [&](bool check, uint8_t objAttrValue, const QString &attrName, Attribute attr) {
		if (check and objAttrValue != 0) {
			silent(id, QString("%1's unused attribute %2 should be 0, not %3")
			       .arg(unitType, attrName).arg(objAttrValue), setAttribute(id, attr, 0));
		}
	};
	
	checkZero(a, object.a, "A", A);
	checkZero(b, object.b, "B", B);
	checkZero(c, object.c, "C", C);
	checkZero(d, object.d, "D", D);
	checkZero(health, object.health, "health", HEALTH);
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
		case X: object.x = value; break;
		case Y: object.y = value; break;
		case HEALTH: object.health = value; break;
		default:;	
		}
		_mapController.setObject(id, object);
	};
}
