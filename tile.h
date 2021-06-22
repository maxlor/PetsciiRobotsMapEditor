#ifndef _TILE_H
#define _TILE_H

#include <QImage>

class Tileset;

class Tile {
	friend class Tileset;
public:
	Tile(const Tile &other) = default;
	Tile(Tile &&other);
	
	Tile &operator=(const Tile &other);
	
	bool canWalk() const { return _flags & 0x01; }
	bool canHover() const { return _flags & 0x02; }
	bool canBeMoved() const { return _flags & 0x04; }
	bool canBeDestroyed() const { return _flags & 0x08; }
	bool canShootThrough() const { return _flags & 0x10; }
	bool canPushOnto() const { return _flags & 0x20; }
	bool canSearch() const { return _flags & 0x40; }
	
	uint8_t flags() const { return _flags; }
	const QImage &image() const { return _image; }
	
protected:
	Tile(uint8_t flags, QImage image);
	
private:
	uint8_t _flags;
	QImage _image;
};

#endif // _TILE_H
