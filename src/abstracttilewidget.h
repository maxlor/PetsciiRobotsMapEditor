#ifndef ABSTRACTTILEWIDGET_H
#define ABSTRACTTILEWIDGET_H

#include <QColor>
#include <QFlags>
#include <QWidget>
#include "tile.h"

class Tileset;


class AbstractTileWidget : public QWidget{
	Q_OBJECT
public:
	AbstractTileWidget(QWidget *parent = nullptr);
	virtual ~AbstractTileWidget();

	void setTileset(const Tileset *tileset);
	const Tileset *tileset() const;
	
public slots:
	void zoomIn();
	void zoomOut();
	
	void setHighlightAttribute(Tile::Attribute attribute);
	
	void setShowSelected(bool show);
	
protected:
	virtual void highlightAttributeChanged() = 0;
	virtual void scaleChanged() = 0;
	virtual void tilesetChanged() = 0;
	bool showSelected() const;
	
	QColor highlightColor() const;
	QColor noHighlightColor() const;
	Tile::Attribute highlightAttribute() const;
	double scale() const;
	
	static void drawMargin(QPainter &painter, const QRect &rect, int margin);
	
private:
	const Tileset *_tileset = nullptr;
	Tile::Attribute _highlightAttribute = Tile::None;
	double _scale = 1.0;
	bool _showSelected = false;
};

#endif // ABSTRACTTILEWIDGET_H
