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
}


Tile Tileset::tile(int tileNo) const {
	if (0 <= tileNo) {
		Q_ASSERT(tileNo <= 255);
		const uint8_t flags = _tileset[0x102 + tileNo];
		return Tile(flags, const_cast<Tileset*>(this)->standardTile(tileNo));
	} else {
		return Tile(0, const_cast<Tileset*>(this)->specialTile(tileNo));
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
	static const QString filename = ":/res/characters.png";
	
	QFile file(filename);
	if (not file.exists()) {
		qCWarning(lcTileset) << "file" << filename << "is missing";
		return;
	}

	if (_characters.load(&file, nullptr)) {
		_characters = _characters.convertToFormat(QImage::Format_ARGB32_Premultiplied);
	} else {
		qCWarning(lcTileset) << "cannot load file" << filename;
	}
}


void Tileset::readTileset() {
	static const QString filename = ":/res/tileset.pet";
	memset(_tileset, 0, sizeof(_tileset));
	
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
		
	file.read(reinterpret_cast<char*>(_tileset), sizeof(_tileset));
}


const QImage &Tileset::standardTile(int tileNo) {
	auto pair = _tiles.try_emplace(tileNo, QImage(TILE_WIDTH * GLYPH_WIDTH,
	                                              TILE_HEIGHT * GLYPH_HEIGHT,
	                                              _characters.format()));
	const auto it = pair.first;
	bool isNew = pair.second;
	if (isNew) {
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
	return it->second;
}


const QImage &Tileset::specialTile(int tileNo) {
	static const QColor weaponColor(255, 255, 100);
	static const QColor toolColor(255, 128, 0);
	static const std::unordered_map<int, std::pair<QString, QColor>> textAndColor {
		{ OBJECT_TRANSPORTER, { "Trns", { 150, 200, 255 }}},
		{ OBJECT_DOOR, { "Door", { 150, 200, 255 }}},
		{ OBJECT_TRASH_COMPACTOR, { "TC", { 255, 64, 0 }}},
		{ OBJECT_ELEVATOR, { "Lift", { 150, 200, 255 }}},
		{ OBJECT_WATER_RAFT, { "Raft", { 150, 200, 255 }}},
		{ OBJECT_KEY, { "Key", { 80, 130, 255 }}},
		{ OBJECT_TIME_BOMB, { "Bmb", weaponColor }},
		{ OBJECT_EMP, { "EMP", toolColor }},
		{ OBJECT_PISTOL, { "Gun", weaponColor }},
		{ OBJECT_PLASMA_GUN, { "Plas", weaponColor }},
		{ OBJECT_MEDKIT, { "Med", { 100, 255, 100 }}},
		{ OBJECT_MAGNET, { "Mag", toolColor }},
	};
	static const QColor objectBgColor(0, 0, 0, 180);
	auto pair = _tiles.try_emplace(tileNo, QImage(TILE_WIDTH * GLYPH_WIDTH,
	                                              TILE_HEIGHT * GLYPH_HEIGHT,
	                                              _characters.format()));
	const auto it = pair.first;
	bool isNew = pair.second;
	if (isNew) {
		QImage &image = it->second;
		std::pair<QString, QColor> textAndColorPair = textAndColor.at(-tileNo);
		QPainter painter(&image);
		image.fill(Qt::transparent);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.setPen(QPen(textAndColorPair.second, 2, Qt::DotLine));
		painter.setBrush(objectBgColor);
		QFont font("Sans-Serif");
		font.setPixelSize(8);
		painter.setFont(font);
		painter.drawEllipse(QRect(1, 1, image.width() - 2, image.height() - 2));
		QString text = textAndColorPair.first;
		painter.drawText(image.rect(), Qt::AlignCenter, text);
	}
	return it->second;
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
