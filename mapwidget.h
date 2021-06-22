#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QRect>
#include <QSize>
#include <QWidget>
#include "abstracttilewidget.h"

class Map;
class Tile;
class Tileset;


class MapWidget : public AbstractTileWidget {
	Q_OBJECT
public:
	explicit MapWidget(QWidget *parent = nullptr);
	virtual ~MapWidget();
	
	void setMap(Map *map);
	
public slots:
	void setObjectsVisible(bool visible);
	void clickEveryTile(); // TODO: remove
	
signals:
	void tileClicked(int x, int y);
	void objectClicked(int objectNo);
	void mouseOverTile(int x, int y);
	
protected:
	void paintEvent(QPaintEvent *event) override;
	QSize sizeHint() const override;
	
	void flagsChanged() override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void scaleChanged() override;
	void tilesetChanged() override;
	
private:
	void drawObject(QPainter &painter, int objectNo);
	QSize imageSize() const;
	void makeTilesImage();
	void makeObjectsImage();
	void makeImage();
	Tile tile(int x, int y);
	QRect tileRect(int x, int y);
	
	bool _objectsVisible = true;
	Map *_map = nullptr;
	QImage *_objectsImage = nullptr;
	QImage *_tilesImage = nullptr;
	QImage *_image = nullptr;
};

#endif // MAPWIDGET_H
