#include "tileset.h"

#include <QByteArray>
#include <QFile>
#include <QPainter>
#include <cstring>
#include "constants.h"
#include "tile.h"


static constexpr size_t TILE_COUNT(256);
static constexpr size_t TILE_WIDTH(3);
static constexpr size_t TILE_HEIGHT(3);
static constexpr size_t GLYPH_WIDTH(8);
static constexpr size_t GLYPH_HEIGHT(8);
static constexpr char TILESET_PET_MAGIC[2] = { 0x00, 0x49 };
static constexpr Tile::Attribute TILE_ATTRIBUTES[] = {
    Tile::Walkable, Tile::Hoverable, Tile::Movable, Tile::Destructible,
    Tile::ShootThrough, Tile::PushOnto, Tile::Searchable
};


Tileset::Tileset(QObject *parent) : QObject(parent), _tiles(TILE_COUNT) {
	readCharacters();
	memset(_tileset, '#', sizeof(_tileset));
	createTileImage(0);
}


Tileset::~Tileset() {
	for (size_t i = 0; i < _tiles.size(); ++i) {
		delete _tiles.at(i);
		_tiles.at(i) = nullptr;
	}
}


QString Tileset::load(const QString &path) {
	QFile file(path);
	if (not file.exists()) {
		return QString("file \"%1\" does not exist").arg(path);
	}
	if (file.size() != sizeof(_tileset)) {
		return QString("file %1 has size %2 but was expected to be size %3")
		        .arg(path).arg(file.size()).arg(sizeof(_tileset));
	}
	
	if (not file.open(QFile::ReadOnly)) {
		return QString("can't read file \"%1\": %2").arg(path, file.errorString());
	}
	
	char magicBuffer[sizeof(TILESET_PET_MAGIC)];
	qint64 bytesRead = file.read(magicBuffer, sizeof(magicBuffer));
	if (bytesRead != sizeof(magicBuffer)) {
		return QString("can't read magic of file \"%1\": got only %2 bytes but had requested %3")
		        .arg(path).arg(bytesRead).arg(sizeof(magicBuffer));
	}
	
	for (size_t i = 0; i < sizeof(TILESET_PET_MAGIC); ++i) {
		if (magicBuffer[i] != TILESET_PET_MAGIC[i]) {
			return QString("file \"%1\" is invalid because its magic (0x%2%3) is wrong "
                           "(it should be 0x%4%5)")
			        .arg(path)
			        .arg(static_cast<unsigned int>(magicBuffer[0]) & 0xFF, 2, 16, QChar('0'))
			        .arg(static_cast<unsigned int>(magicBuffer[1]) & 0xFF, 2, 16, QChar('0'))
			        .arg(static_cast<unsigned int>(TILESET_PET_MAGIC[0]) & 0xFF, 2, 16, QChar('0'))
			        .arg(static_cast<unsigned int>(TILESET_PET_MAGIC[1]) & 0xFF, 2, 16, QChar('0'));
		}
	}
	
	char restBuffer[sizeof(_tileset) - sizeof(TILESET_PET_MAGIC)];
	bytesRead = file.read(restBuffer, sizeof(restBuffer));
	if (bytesRead != sizeof(restBuffer)) {
		return QString("can't read rest of file \"%1\": got only %2 bytes but had requested %3")
		        .arg(path).arg(bytesRead).arg(sizeof(restBuffer));
	}
	
	memcpy(_tileset, magicBuffer, sizeof(magicBuffer));
	memcpy(&_tileset[sizeof(magicBuffer)], restBuffer, sizeof(restBuffer));
	
	for (size_t i = 0; i < TILE_COUNT; ++i) {
		createTileImage(i);
	}
	
	emit changed();
	return QString();
}


Tile Tileset::tile(uint8_t tileNo) const {
	QFlags<Tile::Attribute> flags;
	uint8_t f = _tileset[0x102 + tileNo];
	for (Tile::Attribute attribute : TILE_ATTRIBUTES) {
		flags.setFlag(attribute, f & attribute);
	}
	return Tile(flags, tileImage(tileNo));
}


size_t Tileset::tileCount() const {
	return TILE_COUNT;
}


/** Returns the pixel size of a single tile. */
QSize Tileset::tileSize() const {
	return { GLYPH_WIDTH * TILE_WIDTH, GLYPH_HEIGHT * TILE_HEIGHT };
}


bool Tileset::isValid() const {
	return _tileset[0] == 0x00 and _tileset[1] == 0x49;
}


QImage Tileset::characterImage(uint8_t c) const {
	bool invert = c & 0x80;
	bool shift8 = (c & 0xC0) == 0x40;
	bool shift4 = (c & 0xE0) == 0x00;
	bool sub4 = (c & 0xC0) == 0x80;
	if (c == '\0') { c = '@'; }
	else if (shift8) { c ^= 0x80; }
	else if (shift4) { c ^= 0x40; }
	else if (sub4) { c -= 0x40; }

	int row = (0xFF & c) / 16;
	int col = (0xFF & c) % 16;
	QImage image = _characters.copy(col * GLYPH_WIDTH, row * GLYPH_HEIGHT,
	                                GLYPH_WIDTH, GLYPH_HEIGHT);
	if (invert) {  image.invertPixels(); }
	return image;
}


void Tileset::readCharacters() {
	static const QString filename = ":/characters.png";
	
	QFile file(filename);
	if (not file.exists()) {
		qWarning("file \"%s\" does not exist", filename.toUtf8().constData());
		return;
	}

	if (_characters.load(&file, nullptr)) {
		_characters = _characters.convertToFormat(IMAGE_FORMAT);
	} else {
		qWarning("cannot load file \"%s\"", filename.toUtf8().constData());
	}
}


void Tileset::createTileImage(uint8_t tileNo) {
	QImage *image = new QImage(tileSize(), IMAGE_FORMAT);
	_tiles.at(tileNo) = image;
	
	QPainter painter(image);
	for (size_t row = 0; row < TILE_HEIGHT; ++row) {
		for (size_t col = 0; col < TILE_WIDTH; ++col) {
			Q_ASSERT(TILE_WIDTH == 3 and TILE_HEIGHT == 3);
			const size_t index = 0x202 + col * 0x100 + row * 0x300 + tileNo;
			const uint8_t c = index < sizeof(_tileset) ? _tileset[index] : ' ';
			const QImage image = characterImage(c);
			painter.drawImage(col * GLYPH_WIDTH, row * GLYPH_HEIGHT, image);
		}
	}
}


const QImage &Tileset::tileImage(uint8_t tileNo) const {
	const QImage *pImage = _tiles.at(tileNo);
	if (pImage) {
		return *pImage;
	}
	return *_tiles.at(0); // first image is created in the constructor and always exists
}
