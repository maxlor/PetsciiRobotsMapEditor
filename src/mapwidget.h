#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QPoint>
#include <QRect>
#include <QSize>
#include <QWidget>
#include "abstracttilewidget.h"
#include "mapobject.h"

class Map;
class Tile;
class Tileset;


class MapWidget : public AbstractTileWidget {
	Q_OBJECT
public:
	enum class DragMode { Single, Area };
	
	explicit MapWidget(QWidget *parent = nullptr);
	virtual ~MapWidget();
	
	void setDragMode(DragMode dragMode);
	void setMap(const Map *map);
	
	QRect selectedArea() const;
	
public slots:
	void clearSelection();	
	void markObject(MapObject::id_t objectId);
	void selectAll();
	void setShowGridLines(bool enable);
	void setObjectsVisible(bool visible);
	void clickEveryTile(); // TODO: remove
	
signals:
	void tileCopied(uint8_t tileNo);
	void tilePressed(const QPoint &tile);
	void tileDragged(const QPoint &tile);
	void released();
	void mouseOverTile(const QPoint &tile);
	void objectClicked(MapObject::id_t objectId);
	void objectDragged(MapObject::id_t objectId, const QPoint &tile);
	
protected:
	void paintEvent(QPaintEvent *event) override;
	QSize sizeHint() const override;
	
	void highlightAttributeChanged() override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void scaleChanged() override;
	void tilesetChanged() override;
	
private slots:
	void onMapObjectsChanged();
	void onMapTilesChanged();
	
private:
	void drawObject(QPainter &painter, MapObject::id_t objectId);
	void drawSpecialObject(QPainter &painter, const QRect &rect, MapObject::UnitType unitType);
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
	DragMode _dragMode = DragMode::Single;
	MapObject::id_t _dragObject = MapObject::IdNone;
	MapObject::id_t _selectedObject = MapObject::IdNone;
	QPoint _dragAreaBegin = QPoint(-1, -1);
	QPoint _dragAreaEnd;
};

#endif // MAPWIDGET_H
