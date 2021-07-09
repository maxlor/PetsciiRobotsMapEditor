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
static constexpr size_t TILESET_PET_SIZE = 2817;
static constexpr size_t TILESET_C64_SIZE = 5121;
static constexpr char TILESET_PET_MAGIC[2] = { 0x00, 0x49 };
static constexpr Tile::Attribute TILE_ATTRIBUTES[] = {
    Tile::Walkable, Tile::Hoverable, Tile::Movable, Tile::Destructible,
    Tile::ShootThrough, Tile::PushOnto, Tile::Searchable
};
static const QRgb COLORS_COCO[16] = { // Comunity Colors V1.2
    0xFF000000, 0xFFFFFFFF, 0xFFAF2A29, 0xFF6ED8CC, 0xFFB03FB6, 0xFF4AC64A, 0xFF3739C4, 0xFFE4ED4E,
    0xFFB6591C, 0xFF683808, 0xFFEA746C, 0xFF4D4D4D, 0xFF848484, 0xFFA6FA9E, 0xFF707CE6, 0xFFB6B6B5
};
static const QRgb COLORS_COLODORE[16] = { // Colodore
    0xFF000000, 0xFFFFFFFF, 0xFF813338, 0xFF75CEC8, 0xFF8E3C97, 0xFF56AC4D, 0xFF2E2C9B, 0xFFEDF171,
    0xFF8E5029, 0xFF553800, 0xFFC46C71, 0xFF4A4A4A, 0xFF7B7B7B, 0xFFA9FF9F, 0xFF706DEB, 0xFFB2B2B2
};
static const QRgb COLORS_RGB[16] = { // RGB
    0xFF000000, 0xFFFFFFFF, 0xFF880000, 0xFFAAFFEE, 0xFFCC44CC, 0xFF00CC55, 0xFF0000AA, 0xFFEEEE77,
    0xFFDD8855, 0xFF664400, 0xFFFF7777, 0xFF333333, 0xFF777777, 0xFFAAFF66, 0xFF0088FF, 0xFFBBBBBB
};


Tileset::Tileset(QObject *parent) : QObject(parent), _tiles(TILE_COUNT) {
	readCharacters();
	_tilesetSize = TILESET_PET_SIZE;
	_tileset = new uint8_t[_tilesetSize];
	memset(_tileset, '#', _tilesetSize);
	createTileImage(0);
}


Tileset::~Tileset() {
	for (size_t i = 0; i < _tiles.size(); ++i) {
		delete _tiles.at(i);
		_tiles.at(i) = nullptr;
	}
	delete[] _tileset;
}


QString Tileset::load(const QString &path) {
	size_t tilesetSize;
	
	QFile file(path);
	if (not file.exists()) {
		return QString("file \"%1\" does not exist").arg(path);
	}
	
	if (file.size() == TILESET_PET_SIZE) {
		tilesetSize = TILESET_PET_SIZE;
	} else if (file.size() == TILESET_C64_SIZE) {
		tilesetSize = TILESET_C64_SIZE;
	} else {
		return QString("file %1 has size %2 but was expected to be size %3 or %4")
		        .arg(path).arg(file.size()).arg(TILESET_PET_SIZE).arg(TILESET_C64_SIZE);
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
	
	const size_t restBufferSize = tilesetSize - sizeof(TILESET_PET_MAGIC);
	uint8_t restBuffer[restBufferSize];
	bytesRead = file.read(reinterpret_cast<char*>(restBuffer), restBufferSize);
	if (bytesRead != qint64(restBufferSize)) {
		return QString("can't read rest of file \"%1\": got only %2 bytes but had requested %3")
		        .arg(path).arg(bytesRead).arg(restBufferSize);
	}
	
	if (tilesetSize != _tilesetSize) {
		delete[] _tileset;
		_tileset = new uint8_t[tilesetSize];
		_tilesetSize = tilesetSize;
	}
	memcpy(_tileset, magicBuffer, sizeof(magicBuffer));
	memcpy(&_tileset[sizeof(magicBuffer)], restBuffer, restBufferSize);
	
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


bool Tileset::haveColor() const {
	return _tilesetSize == TILESET_C64_SIZE;
}


bool Tileset::isValid() const {
	return _tileset[0] == 0x00 and _tileset[1] == 0x49;
}


Tileset::Palette Tileset::palette() const {
	return _palette;
}


void Tileset::setPalette(Tileset::Palette palette) {
	if (palette != _palette) {
		_palette = palette;
		
		if (haveColor()) {
			for (size_t i = 0; i < TILE_COUNT; ++i) {
				createTileImage(i);
			}
			
			emit changed();
		}
	}
}


Tileset::Palette Tileset::paletteFromInt(int value) {
	switch (value) {
	case int(Palette::CoCo): return Palette::CoCo;
	case int(Palette::Colodore): return Palette::Colodore;
	case int(Palette::RGB): return Palette::RGB;
	default: return Palette::CoCo;
	}
}


std::forward_list<Tileset::Palette> Tileset::palettes() {
	return { Palette::CoCo, Palette::Colodore, Palette::RGB };
}


QString Tileset::toString(Tileset::Palette palette) {
	switch (palette) {
	case Palette::RGB: return "RGB";
	case Palette::CoCo: return "Community Colors";
	case Palette::Colodore: return "Colodore";
	}
}


const QRgb *Tileset::colors() const {
	switch (_palette) {
	case Palette::CoCo: return COLORS_COCO;
	case Palette::Colodore: return COLORS_COLODORE;
	case Palette::RGB: return COLORS_RGB;
	}
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
			const uint8_t c = index < _tilesetSize ? _tileset[index] : ' ';
			QImage image = characterImage(c);
			if (haveColor()) {
				constexpr size_t colorBaseOffset = TILESET_PET_SIZE;
				const size_t offset = colorBaseOffset + (3 * 256) * row + 256 * col + tileNo + 1;
				if (offset < _tilesetSize) {
					const uint8_t fgColorNum = _tileset[offset] & 0x0F;
					const QRgb fg = colors()[fgColorNum];
					const QRgb bg = 0xFF000000;
					for (int y = 0; y < image.width(); ++y) {
						for (int x = 0; x < image.height(); ++x) {
							image.setPixel(x, y, image.pixel(x, y) == 0xFFFFFFFF ? fg : bg);
						}
					}
				}
			}
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
