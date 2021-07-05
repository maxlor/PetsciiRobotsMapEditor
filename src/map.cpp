#include "map.h"
#include <QFile>
#include <QLoggingCategory>
#include <QRect>
#include <cstring>
#include <forward_list>
#include <utility>


Q_LOGGING_CATEGORY(lcMap, "map");

static constexpr char MAP_MAGIC[2] = { 0x00, 0x5d };
static constexpr size_t MAP_BYTES(8962);
static constexpr int MAP_WIDTH(128);
static constexpr int MAP_HEIGHT(64);
static constexpr size_t OBJECT_COUNT(64);
static constexpr size_t TILE_COUNT(MAP_WIDTH * MAP_HEIGHT);


Map::Map(QObject *parent) : QObject(parent) {
	_objects = new MapObject[OBJECT_COUNT];
	_tiles = new uint8_t[TILE_COUNT];
	memset(_objects, 0, sizeof(_objects[0]) * OBJECT_COUNT);
	memset(_tiles, 0, sizeof(_tiles[0]) * TILE_COUNT);
	
	connect(this, &Map::objectsChanged, &Map::setModifiedFlag);
	connect(this, &Map::tilesChanged, &Map::setModifiedFlag);
}


Map::~Map() {
	delete[] _tiles;
	delete[] _objects;
}


int Map::width() const {
	return MAP_WIDTH;
}


int Map::height() const {
	return MAP_HEIGHT;
}


QRect Map::rect() const {
	return QRect(0, 0, width(), height()); 
}


void Map::clear() {
	memset(_objects, 0, sizeof(_objects[0]) * OBJECT_COUNT);
	memset(_tiles, 0, sizeof(_tiles[0]) * TILE_COUNT);
	
	_modified = true; // make sure only one modifiedChanged signal is emitted	
	setPath(QString());
	emit objectsChanged();
	emit tilesChanged();
	setModified(false);
}


QString Map::load(const QString &path) {
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
	
	_modified = true; // make sure only one modifiedChanged signal is emitted
	setPath(path);
	emit tilesChanged();
	emit objectsChanged();
	setModified(false);
	
	return QString();
}


QString Map::save(const QString &path) {
	QFile file(path);
	if (not file.open(QFile::WriteOnly)) {
		return QString("cannot open \"%1\" for writing: %2") .arg(path, file.errorString());
	}
	
	qint64 bytesWritten = file.write(data());
	if (bytesWritten == -1) {
		return QString("cannot write to \"%1\": %2") .arg(path, file.errorString());
	} else if (bytesWritten != MAP_BYTES) {
		return QString("cannot write to \"%1\": short " "write, only %2 out of %3 bytes were "
		               "written").arg(path).arg(bytesWritten).arg(MAP_BYTES);
	}
	
	setPath(path);
	setModified(false);
	return QString();
}


int Map::hiddenItemCount() const {
	int count = 0;
	for (MapObject::id_t i = MapObject::IdHiddenMin; i <= MapObject::IdHiddenMax; ++i) {
		if (_objects[i].unitType != MapObject::UnitType::None) { ++count; }
	}
	return count;
}


int Map::mapFeatureCount() const {
	int count = 0;
	for (MapObject::id_t i = MapObject::IdMapFeatureMin; i <= MapObject::IdMapFeatureMax; ++i) {
		if (_objects[i].unitType != MapObject::UnitType::None) { ++count; }
	}
	return count;
}


int Map::robotCount() const {
	int count = 0;
	for (MapObject::id_t i = MapObject::IdRobotMin; i <= MapObject::IdRobotMax; ++i) {
		if (_objects[i].unitType != MapObject::UnitType::None) { ++count; }
	}
	return count;
}


MapObject::id_t Map::nextAvailableObjectId(MapObject::Group group) const {
	switch (group) {
	case MapObject::Group::Invalid: return MapObject::IdNone;
	case MapObject::Group::Player: return MapObject::IdPlayer;
	case MapObject::Group::Robots:
		for (MapObject::id_t i = MapObject::IdRobotMin; i <= MapObject::IdRobotMax; ++i) {
			if (_objects[i].unitType == MapObject::UnitType::None) { return i; }
		}
		return MapObject::IdNone;
	case MapObject::Group::MapFeatures:
		for (int i = MapObject::IdMapFeatureMin; i <= MapObject::IdMapFeatureMax; ++i) {
			if (_objects[i].unitType == MapObject::UnitType::None) { return i; }
		}
		return MapObject::IdNone;
	case MapObject::Group::HiddenObjects:
		for (int i = MapObject::IdHiddenMin; i <= MapObject::IdHiddenMax; ++i) {
			if (_objects[i].unitType == MapObject::UnitType::None) { return i; }
		}
		return MapObject::IdNone;
	}
}


const MapObject &Map::object(int no) const {
	return const_cast<Map*>(this)->objectAt(no);
}


void Map::deleteObject(MapObject::id_t no) {
	objectAt(no) = MapObject();
	compact();
	emit objectsChanged();
}


void Map::moveObject(MapObject::id_t no, const QPoint &pos) {
	MapObject &object = objectAt(no);
	object.x = pos.x();
	object.y = pos.y();
	emit objectsChanged();
}


void Map::setObject(MapObject::id_t no, const MapObject &object) {
	objectAt(no) = object;
	emit objectsChanged();
}


void Map::setObjects(const MapObject objects[]) {
	memcpy(_objects, objects, sizeof(_objects[0]) * OBJECT_COUNT);
	emit objectsChanged();
}


uint8_t Map::tileNo(const QPoint &tile) const {
	Q_ASSERT(0 <= tile.x() and tile.x() < width());
	Q_ASSERT(0 <= tile.y() and tile.y() < height());
	return _tiles[tile.x() + width() * tile.y()];
}


uint8_t *Map::tiles() {
	return _tiles;
}


void Map::setTile(const QPoint &position, uint8_t tileNo) {
	Q_ASSERT(0 <= position.x() and position.x() < width());
	Q_ASSERT(0 <= position.y() and position.y() < height());
	int oldTileNo = _tiles[position.x() + width() * position.y()];
	if (oldTileNo == tileNo) { return; }
	_tiles[position.x() + width() * position.y()] = tileNo;
	emit tilesChanged();
}


void Map::setTiles(const QRect &rect, uint8_t *tiles) {
	Q_ASSERT(0 <= rect.left() and rect.right() < width());
	Q_ASSERT(0 <= rect.top() and rect.bottom() < height());
	for (int y = rect.top(); y <= rect.bottom(); ++y) {
		memcpy(&_tiles[rect.left() + width() * y], &tiles[rect.width() * (y - rect.top())], rect.width());
	}
	emit tilesChanged();
}


void Map::floodFill(const QPoint &position, uint8_t tileNo) {
	Q_ASSERT(0 <= position.x() and position.x() < width());
	Q_ASSERT(0 <= position.y() and position.y() < height());
	uint8_t oldTileNo = _tiles[position.x() + width() * position.y()];
	if (oldTileNo == tileNo) { return; }
	recursiveFloodFill(position, oldTileNo, tileNo);
	emit tilesChanged();
}


bool Map::isModified() const {
	return _modified;
}


const QString &Map::path() const {
	return _path;
}


void Map::compact() {
	static const std::forward_list<std::pair<MapObject::id_t, MapObject::id_t>> ranges = {
	    { MapObject::IdRobotMin, MapObject::IdRobotMax },
	    { MapObject::IdMapFeatureMin, MapObject::IdMapFeatureMax },
	    { MapObject::IdHiddenMin, MapObject::IdHiddenMax }};
	
	bool changed = false;
	
	for (const std::pair<MapObject::id_t, MapObject::id_t> &range : ranges) {
		for (MapObject::id_t i = range.first; i <= range.second - 1; ++i) {
			if (_objects[i].unitType != MapObject::UnitType::None) { continue; }
			bool moved = false;
			for (MapObject::id_t j = i + 1; j < range.second; ++j) {
				if (_objects[j].unitType != MapObject::UnitType::None) {
					_objects[i] = _objects[j];
					_objects[j] = MapObject();
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


QByteArray Map::data() const {
	QByteArray ba(MAP_BYTES, 0);
	
	memcpy(ba.data(), MAP_MAGIC, sizeof(MAP_MAGIC));
	for (MapObject::id_t i = MapObject::IdMin; i <= MapObject::IdMax; ++i) {
		ba[0x002 + i] = MapObject::unitType_t(_objects[i].unitType);
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
	
	Q_ASSERT(0x302 + sizeof(_tiles[0]) * TILE_COUNT == MAP_BYTES);
	memcpy(ba.data() + 0x302, _tiles, sizeof(_tiles[0]) * TILE_COUNT);
	
	return ba;
}


void Map::setModifiedFlag() {
	setModified(true);
}


MapObject &Map::objectAt(MapObject::id_t no) {
	Q_ASSERT_X(0 <= no and no <= 63, Q_FUNC_INFO,
	           QString("Can't get object no %1").arg(no).toUtf8().constData());
	return _objects[no];
}


int Map::recursiveFloodFill(const QPoint &pos, uint8_t oldTile, uint8_t newTile) {
	Q_ASSERT(_tiles[pos.x() + width() * pos.y()] == oldTile);
	int left = pos.x();
	int right = pos.x();
	while (left > 0 and _tiles[left - 1 + width() * pos.y()] == oldTile) { --left; }
	while (right < width() - 1 and _tiles[right + 1 + width() * pos.y()] == oldTile) { ++right; }
	memset(&_tiles[left + width() * pos.y()], newTile, right - left + 1);
	for (int y2 : { pos.y() - 1, pos.y() + 1}) {
		if (not (0 <= y2 and y2 < height())) { continue; }
		for (int x2 = left; x2 <= right; ++x2) {
			if (_tiles[x2 + width() * y2] == oldTile) {
				int right2 = recursiveFloodFill({x2, y2}, oldTile, newTile);
				x2 = right2;
			}
		}
	}
	return right;
}


void Map::setModified(bool modified) {
	if (_modified != modified) {
		_modified = modified;
		emit modifiedChanged();
	}
}


void Map::setPath(const QString &path) {
	if (_path != path) {
		_path = path;
		emit pathChanged();
	}
}
