#include "tileset.h"

#include <QByteArray>
#include <QFile>
#include <QLoggingCategory>
#include <QPainter>
#include <cstring>
#include "constants.h"
#include "tile.h"

Q_LOGGING_CATEGORY(lcTileset, "tileset");


Tileset::Tileset() {
	readCharacters();
	readTileset();
	
	for (int i = 0; i < 256; ++i) {
		createTileImage(i);
	}
}


Tile Tileset::tile(int tileNo) const {
	Q_ASSERT(0 <= tileNo and tileNo <= 255);
	const uint8_t flags = _tileset[0x102 + tileNo];
	return Tile(flags, tileImage(tileNo));
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
	static const QString filename = ":/res/characters.png";
	
	QFile file(filename);
	if (not file.exists()) {
		qCWarning(lcTileset) << "file" << filename << "is missing";
		return;
	}

	if (_characters.load(&file, nullptr)) {
		_characters = _characters.convertToFormat(IMAGE_FORMAT);
	} else {
		qCWarning(lcTileset) << "cannot load file" << filename;
	}
}


void Tileset::readTileset() {
	static const QString filename = ":/res/tileset.pet";
	
	QFile file(filename);
	if (not file.exists()) {
		qCWarning(lcTileset) << "file" << filename << "is missing";
		return;
	}
	if (file.size() != sizeof(_tileset)) {
		qCWarning(lcTileset) << "file" << filename << "has size" << file.size() << "but it was"
		                     << "expected to be size" << sizeof(_tileset);
		return;
	}
	
	if (not file.open(QFile::ReadOnly)) {
		qCWarning(lcTileset()).nospace() << "Can't read file " << filename << ": "
		                                 << file.errorString();
		return;
	}
		
	qint64 bytesRead = file.read(reinterpret_cast<char*>(_tileset), sizeof(_tileset));
	Q_ASSERT(bytesRead = sizeof(_tileset));
}


void Tileset::createTileImage(int tileNo) {
	static constexpr int width = TILE_WIDTH * GLYPH_WIDTH;
	static constexpr int height = TILE_HEIGHT * GLYPH_HEIGHT;
	
	auto pair = _tiles.try_emplace(tileNo, width, height, IMAGE_FORMAT);
	Q_ASSERT(pair.second); // image did not exist before emplace() call
	
	const auto it = pair.first;
	QImage &image = it->second;
	QPainter painter(&image);
	for (int row = 0; row < TILE_HEIGHT; ++row) {
		for (int col = 0; col < TILE_WIDTH; ++col) {
			Q_ASSERT(TILE_WIDTH == 3 and TILE_HEIGHT == 3);
			const size_t index = 0x202 + col * 0x100 + row * 0x300 + tileNo;
			const uint8_t c = index < sizeof(_tileset) ? _tileset[index] : ' ';
			const QImage image = characterImage(c);
			painter.drawImage(col * GLYPH_WIDTH, row * GLYPH_HEIGHT, image);
		}
	}
}


const QImage &Tileset::tileImage(int tileNo) const {
	return _tiles.at(tileNo);
}


void Tileset::debugImage(const QImage &image) {
	for (int row = 0; row < image.height(); ++row) {
		QString t;
		for (int col = 0; col < image.width(); ++col) {
			t += image.pixel(col, row) == 0xFFFFFFFF ? "@" : " ";
		}
		qDebug() << Q_FUNC_INFO << t;
	}
}
