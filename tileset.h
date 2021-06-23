#ifndef TILESET_H
#define TILESET_H

#include <QImage>
#include <QString>
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
	void createTileImage(int tileNo);
	const QImage &tileImage(int tileNo) const;
	
	QImage _characters;
	uint8_t _tileset[0xB01];
	std::unordered_map<int, QImage> _tiles;
};

#endif // TILESET_H
