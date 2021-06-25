#include "map.h"
#include <QFile>
#include <QLoggingCategory>
#include <cstring>
#include <forward_list>
#include <unordered_map>

Q_LOGGING_CATEGORY(lcMap, "map");

static constexpr char MAP_MAGIC[2] = { 0x00, 0x5d };


Map::Map(QObject *parent) : QObject(parent) {
	memset(_tiles, 0, sizeof(_tiles));
	memset(_objects, 0, sizeof(_objects));
	readMap(":/res/level-d"); // TODO make configurable
}


int Map::hiddenItemCount() const {
	int count = 0;
	for (int i = HIDDEN_OBJECT_MIN; i <= HIDDEN_OBJECT_MAX; ++i) {
		if (_objects[i].unitType != UNITTYPE_NONE) { ++count; }
	}
	return count;
}


int Map::mapFeatureCount() const {
	int count = 0;
	for (int i = MAP_FEATURE_MIN; i <= MAP_FEATURE_MAX; ++i) {
		if (_objects[i].unitType != UNITTYPE_NONE) { ++count; }
	}
	return count;
}


int Map::robotCount() const {
	int count = 0;
	for (int i = ROBOT_MIN; i <= ROBOT_MAX; ++i) {
		if (_objects[i].unitType != UNITTYPE_NONE) { ++count; }
	}
	return count;
}


int Map::nextAvailableObjectId(Object::Kind kind) const {
	switch (kind) {
	case Object::Kind::Player: return PLAYER_OBJ;
	case Object::Kind::Robot:
		for (int i = ROBOT_MIN; i <= ROBOT_MAX; ++i) {
			if (_objects[i].unitType == UNITTYPE_NONE) { return i; }
		}
		return -1;
	case Object::Kind::TransporterPad:
	case Object::Kind::Door:
	case Object::Kind::TrashCompator:
	case Object::Kind::Elevator:
	case Object::Kind::WaterRaft:
		for (int i = MAP_FEATURE_MIN; i <= MAP_FEATURE_MAX; ++i) {
			if (_objects[i].unitType == UNITTYPE_NONE) { return i; }
		}
		return -1;
	case Object::Kind::Key:
	case Object::Kind::HiddenObject:
		for (int i = HIDDEN_OBJECT_MIN; i <= HIDDEN_OBJECT_MAX; ++i) {
			if (_objects[i].unitType == UNITTYPE_NONE) { return i; }
		}
		return -1;
	case Object::Kind::Invalid: return -1;
	}
}


int Map::tileNo(int x, int y) const {
	Q_ASSERT(0 <= x and x < MAP_WIDTH);
	Q_ASSERT(0 <= y and y < MAP_HEIGHT);
	return _tiles[x + MAP_WIDTH * y];
}


const Map::Object &Map::object(int no) const {
	Q_ASSERT_X(0 <= no and no <= 63, Q_FUNC_INFO,
	           QString("Can't get object no %1").arg(no).toUtf8().constData());
	return _objects[no];
}


void Map::setTile(int x, int y, int tileNo) {
	int oldTileNo = this->tileNo(x, y);
	if (oldTileNo != tileNo) {
		_tiles[x + MAP_WIDTH * y] = tileNo;
		emit tilesChanged();
	}
}


void Map::setObject(int objectNo, const Map::Object &object) {
	Q_ASSERT_X(OBJECT_MIN <= objectNo and objectNo <= OBJECT_MAX, Q_FUNC_INFO,
	           QString("can't load objectNo %1").arg(objectNo).toUtf8().constData());
	_objects[objectNo] = object;
	emit objectsChanged();
}


const std::list<std::pair<uint8_t, QString> > &Map::unitTypes() {
	static std::list<std::pair<uint8_t, QString>> unitTypes = {
	    { 2, "Hoverbot horizontal patrol" },
	    { 3, "Hoverbot vertical patrol" },
	    { 4, "Hoverbot attack mode" },
	    { 9, "Evilbot" },
	    { 18, "Rollerbot horizontal patrol" },
	    { 17, "Rollerbot vertical patrol" },
	    { 10, "Moving Door" },
	    { 128, "Hidden Key" },
	    { 129, "Time Bomb"  },
	    { 130, "EMP" },
	    { 131, "Pistol" },
	    { 132, "Plasma Gun" },
	    { 133, "Medkit" },
	    { 134, "Magnet" },
	    { 7, "Transporter Pad" },
	    { 16, "Trash Compactor" },
	    { 19, "Elevator" },
	    { 22, "Water Raft" },
	};
	return unitTypes;
}


void Map::compact() {
	static const std::forward_list<std::pair<int, int>> ranges = {
	    { ROBOT_MIN, ROBOT_MAX }, { MAP_FEATURE_MIN, MAP_FEATURE_MAX },
	    { HIDDEN_OBJECT_MIN, HIDDEN_OBJECT_MAX }};
	
	bool changed = false;
	
	for (const std::pair<int, int> &range : ranges) {
		for (int i = range.first; i <= range.second - 1; ++i) {
			if (_objects[i].unitType != UNITTYPE_NONE) { continue; }
			bool moved = false;
			for (int j = i + 1; j < range.second; ++j) {
				if (_objects[j].unitType != UNITTYPE_NONE) {
					_objects[i] = _objects[j];
					_objects[j] = Object();
					++i;
					moved = true;
					changed = true;
				}
			}
			if (not moved) { break; }
		}
	}
	
	if (changed) {
		emit objectsChanged();
	}
}


QByteArray Map::data() {
	QByteArray ba(MAP_BYTES, 0);
	
	memcpy(ba.data(), MAP_MAGIC, sizeof(MAP_MAGIC));
	for (int i = OBJECT_MIN; i <= OBJECT_MAX; ++i) {
		ba[0x002 + i] = _objects[i].unitType;
		ba[0x042 + i] = _objects[i].x;
		ba[0x082 + i] = _objects[i].y;
		ba[0x0c2 + i] = _objects[i].a;
		ba[0x102 + i] = _objects[i].b;
		ba[0x142 + i] = _objects[i].c;
		ba[0x182 + i] = _objects[i].d;
		ba[0x1c2 + i] = _objects[i].health;
	}
	
	// I don't know what the range 0x202-0x301 is for, in the original maps
	// those bytes are all set to either 0x00 or 0xAA.
	
	Q_ASSERT(0x302 + sizeof(_tiles) == MAP_BYTES);
	memcpy(ba.data() + 0x302, _tiles, sizeof(_tiles));
	
	return ba;
}


QString Map::readMap(const QString &path) {
	memset(_tiles, 0, sizeof(_tiles));
	
	QFile file(path);
	if (not file.exists()) {
		return QString("file \"%1\" does not exist").arg(path);
	}
	if (file.size() != MAP_BYTES) {
		return QString("file %1 has size %2 but was expected to be size %3")
		        .arg(path).arg(file.size()).arg(MAP_BYTES);
	}
	
	if (not file.open(QFile::ReadOnly)) {
		return QString("can't read file \"%1\": %2").arg(path, file.errorString());
	}
	
	char magicBuffer[sizeof(MAP_MAGIC)];
	qint64 bytesRead = file.read(magicBuffer, sizeof(magicBuffer));
	if (bytesRead != sizeof(magicBuffer)) {
		return QString("can't read magic of file \"%1\": got only %2 bytes but had requested %3")
		        .arg(path).arg(bytesRead).arg(sizeof(magicBuffer));
	}
	
	for (size_t i = 0; i < sizeof(MAP_MAGIC); ++i) {
		if (magicBuffer[i] != MAP_MAGIC[i]) {
			return QString("file \"%1\" is invalid because its magic (0x%2%3) is wrong "
                           "(it should be 0x%4%5)")
			        .arg(path)
			        .arg(static_cast<unsigned int>(magicBuffer[0]) & 0xFF, 2, 16, QChar('0'))
			        .arg(static_cast<unsigned int>(magicBuffer[1]) & 0xFF, 2, 16, QChar('0'))
			        .arg(static_cast<unsigned int>(MAP_MAGIC[0]) & 0xFF, 2, 16, QChar('0'))
			        .arg(static_cast<unsigned int>(MAP_MAGIC[1]) & 0xFF, 2, 16, QChar('0'));
		}
	}
	
	for (int i = 0; i < 64; ++i) { file.getChar(reinterpret_cast<char*>(&_objects[i].unitType)); }
	for (int i = 0; i < 64; ++i) { file.getChar(reinterpret_cast<char*>(&_objects[i].x)); }
	for (int i = 0; i < 64; ++i) { file.getChar(reinterpret_cast<char*>(&_objects[i].y)); }
	for (int i = 0; i < 64; ++i) { file.getChar(reinterpret_cast<char*>(&_objects[i].a)); }
	for (int i = 0; i < 64; ++i) { file.getChar(reinterpret_cast<char*>(&_objects[i].b)); }
	for (int i = 0; i < 64; ++i) { file.getChar(reinterpret_cast<char*>(&_objects[i].c)); }
	for (int i = 0; i < 64; ++i) { file.getChar(reinterpret_cast<char*>(&_objects[i].d)); }
	for (int i = 0; i < 64; ++i) { file.getChar(reinterpret_cast<char*>(&_objects[i].health)); }
	file.seek(0x302);
	file.read(reinterpret_cast<char*>(_tiles), 0x2000);
	return QString();
}


Map::Object::Object() : Map::Object(UNITTYPE_NONE) {}


Map::Object::Object(uint8_t unitType)
    : unitType(unitType), x(0), y(0), a(0), b(0), c(0), d(0), health(0) {}


Map::Object::Kind Map::Object::kind() const {
	return kind(unitType);
}


Map::Object::Kind Map::Object::kind(uint8_t unitType) {
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


const QString &Map::Object::toString(Map::Object::Kind kind) {
	static const std::unordered_map<Object::Kind, QString> names = {
	    { Object::Kind::Player, "player" }, { Object::Kind::Robot, "robot" },
	    { Object::Kind::TransporterPad, "transporter pad" }, { Object::Kind::Elevator, "elevator" },
	    { Object::Kind::WaterRaft, "water raft" }, { Object::Kind::Key, "key" },
	    { Object::Kind::HiddenObject, "hidden object" }, { Object::Kind::Invalid, "invalid" }
	};
	
	auto it = names.find(kind);
	Q_ASSERT(it != names.end());
	return it->second;
}


const QString &Map::Object::category(Map::Object::Kind kind) {
	static const QString mapObjects = "map features";
	static const QString hiddenItems = "hidden items";
	static const std::unordered_map<Object::Kind, QString> names = {
	    { Object::Kind::Player, "player" }, { Object::Kind::Robot, "robots" },
	    { Object::Kind::TransporterPad, mapObjects }, { Object::Kind::Elevator, mapObjects },
	    { Object::Kind::WaterRaft, mapObjects }, { Object::Kind::Key, hiddenItems },
	    { Object::Kind::HiddenObject, hiddenItems }, { Object::Kind::Invalid, "invalid" }
	};
	
	auto it = names.find(kind);
	Q_ASSERT(it != names.end());
	return it->second;
}
