#include "tile.h"

Tile::Tile(QFlags<Attribute> flags, QImage image) : _attributes(flags), _image(image) {}


Tile::Tile(Tile &&other) {
	_attributes = other._attributes;
	_image.swap(other._image);
}


Tile &Tile::operator =(const Tile &other) {
	_attributes = other._attributes;
	_image = other._image;
	return *this;
}
