#ifndef ABSTRACTTILEWIDGET_H
#define ABSTRACTTILEWIDGET_H

#include <QColor>
#include <QWidget>

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
	void setEnlarge(bool enlarge);
	void setShowWalkable(bool show);
	void setShowHoverable(bool show);
	void setShowMovable(bool show);
	void setShowDestructible(bool show);
	void setShowShootThrough(bool show);
	void setShowPushOnto(bool show);
	void setShowSearchable(bool show);
	
protected:
	virtual void flagsChanged() = 0;
	virtual void scaleChanged() = 0;
	virtual void tilesetChanged() = 0;
	
	QColor highlightColor() const;
	QColor noHighlightColor() const;
	uint8_t highlightFlags() const;
	double scale() const;
	
private:
	void setHighlightFlag(int bit, bool set);
	
	const Tileset *_tileset = nullptr;
	uint8_t _highlightFlags = 0;
	QColor _highlightColor;
	double _scale = 1.0;
};

#endif // ABSTRACTTILEWIDGET_H
