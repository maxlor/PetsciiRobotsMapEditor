#ifndef TILESET_H
#define TILESET_H

#include <QImage>
#include <QObject>
#include <QString>
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
	Tileset(QObject *parent = nullptr);
	virtual ~Tileset();
	
	/** Load the tileset from a file.
	 *  
	 *  @param path the path
	 *  @return a null string on success, an error message otherwise.
	 */
	QString load(const QString &path);
	Tile tile(int tileNo) const;
	
	bool isValid() const;
	
signals:
	void changed();
	
private:
	QImage characterImage(uint8_t c) const;
	void readCharacters();
	void createTileImage(int tileNo);
	const QImage &tileImage(int tileNo) const;
	
	QImage _characters;
	uint8_t _tileset[0xB01];
	std::vector<const QImage*> _tiles;
};

#endif // TILESET_H
