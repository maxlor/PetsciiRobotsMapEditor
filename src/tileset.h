#ifndef TILESET_H
#define TILESET_H

#include <QImage>
#include <QObject>
#include <QSize>
#include <QString>
#include <forward_list>
#include <vector>

class Tile;


/**
 * A tile set of 256 3x3 character tiles.
 * 
 * Should be loaded from `tileset.pet`.
 */
class Tileset : public QObject {
	Q_OBJECT
public:
	enum class Palette { CoCo, Colodore, RGB };
	Tileset(QObject *parent = nullptr);
	virtual ~Tileset();
	
	/** Load the tileset from a file.
	 *  
	 *  @param path the path
	 *  @return a null string on success, an error message otherwise.
	 */
	QString load(const QString &path);
	Tile tile(uint8_t tileNo) const;
	
	size_t tileCount() const;
	QSize tileSize() const;
	
	bool haveColor() const;
	bool isValid() const;
	
	Palette palette() const;
	void setPalette(Palette palette);
	
	static Palette paletteFromInt(int value);
	static std::forward_list<Palette> palettes();
	static QString toString(Palette palette);
	
signals:
	void changed();
	
private:
	const QRgb *colors() const;
	
	QImage characterImage(uint8_t c) const;
	void readCharacters();
	void createTileImage(uint8_t tileNo);
	const QImage &tileImage(uint8_t tileNo) const;
	
	QImage _characters;
	uint8_t *_tileset = nullptr;
	size_t _tilesetSize;
	std::vector<const QImage*> _tiles;
	Palette _palette = Palette::CoCo;
};

#endif // TILESET_H
