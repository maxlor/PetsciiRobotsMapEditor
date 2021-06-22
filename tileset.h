#ifndef TILESET_H
#define TILESET_H

#include <QString>
#include <QImage>
#include <unordered_map>

class Tile;


class Tileset {
public:
	Tileset();
	
	Tile tile(int tileNo) const;	
	
	static void debugImage(const QImage &image);
private:
	QImage characterImage(uint8_t c) const;
	void readCharacters();
	void readTileset();
	const QImage &standardTile(int tileNo);
	const QImage &specialTile(int tileNo);
	
	QImage _characters;
	uint8_t _tileset[0xB01];
	std::unordered_map<int, QImage> _tiles;
};

#endif // TILESET_H
