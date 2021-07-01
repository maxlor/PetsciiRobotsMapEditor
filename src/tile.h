#ifndef _TILE_H
#define _TILE_H

#include <QFlags>
#include <QImage>

class Tileset;

class Tile {
	friend class Tileset;
public:
	enum Attribute : uint8_t {
		None = 0x00, Walkable = 0x01, Hoverable = 0x02, Movable = 0x04, Destructible = 0x08,
		ShootThrough = 0x10, PushOnto = 0x20, Searchable = 0x40
	};
	
	Tile(const Tile &other) = default;
	Tile(Tile &&other);
	
	Tile &operator=(const Tile &other);
	
	QFlags<Attribute> attributes() const { return _attributes; }
	const QImage &image() const { return _image; }
	
protected:
	Tile(QFlags<Attribute> flags, QImage image);
	
private:
	QFlags<Attribute> _attributes;
	QImage _image;
};

#endif // _TILE_H
