#include "map.h"
#include <QFile>
#include <QLoggingCategory>
#include <cstring>

Q_LOGGING_CATEGORY(lcMap, "map");


Map::Map(QObject *parent) : QObject(parent) {
	memset(_tiles, 0, sizeof(_tiles));
	memset(_objects, 0, sizeof(_objects));
	readMap(":/res/level-d"); // TODO make configurable
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


void Map::readMap(const QString &filename) {
	qCInfo(lcMap) << "opening" << filename;
	
	memset(_tiles, 0, sizeof(_tiles));
	
	QFile file(filename);
	if (not file.exists()) {
		qCWarning(lcMap) << "file" << filename << "is missing";
		return;
	}
	if (file.size() != 8962) {
		qCWarning(lcMap) << "file" << filename << "has size" << file.size() << "but it was"
		                     << "expected to be size 8962";
		return;
	}
	
	if (not file.open(QFile::ReadOnly)) {
		qCWarning(lcMap()).nospace() << "Can't read file " << filename << ": "
		                                 << file.errorString();
		return;
	}
	
	file.seek(2);
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
}


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
