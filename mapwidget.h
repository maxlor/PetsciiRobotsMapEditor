#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QPoint>
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
	
	void setMap(const Map *map);
	
public slots:
	void clearSelection();
	void setShowGridLines(bool enable);
	void setObjectsVisible(bool visible);
	void clickEveryTile(); // TODO: remove
	
signals:
	void tileClicked(const QPoint &tile);
	void tileDragged(const QPoint &tile);
	void mouseOverTile(const QPoint &tile);
	void mouseReleased();
	void objectClicked(int objectNo);
	void objectDragged(int objectNo, const QPoint &tile);
	
protected:
	void paintEvent(QPaintEvent *event) override;
	QSize sizeHint() const override;
	
	void highlightFlagsChanged() override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void scaleChanged() override;
	void tilesetChanged() override;
	
private slots:
	void onMapObjectsChanged();
	void onMapTilesChanged();
	
private:
	void drawObject(QPainter &painter, int objectNo);
	void drawSpecialObject(QPainter &painter, const QRect &rect, int unitType);
	QSize imageSize() const;
	void makeTilesImage();
	void makeImage();
	Tile tile(QPoint position) const;
	QRect tileRect(const QPoint &position) const;
	QPoint pixelToTile(QPoint pos);
	
	bool _objectsVisible = true;
	const Map *_map = nullptr;
	QImage *_tilesImage = nullptr;
	QImage *_image = nullptr;
	bool _redrawTiles = false;
	bool _redrawImage = false;
	bool _showGridLines = false;
	int _dragObject = -1;
};

#endif // MAPWIDGET_H
