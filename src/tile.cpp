#include "tile.h"

Tile::Tile(uint8_t flags, QImage image) : _flags(flags), _image(image) {}


Tile::Tile(Tile &&other) {
	_flags = other._flags;
	_image.swap(other._image);
}


Tile &Tile::operator =(const Tile &other) {
	_flags = other._flags;
	_image = other._image;
	return *this;
}
