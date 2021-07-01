#ifndef TILEWIDGET_H
#define TILEWIDGET_H

#include <QColor>
#include <QRect>
#include <QSize>
#include <QWidget>
#include "abstracttilewidget.h"

class QImage;
class Tileset;


class TileWidget : public AbstractTileWidget {
	Q_OBJECT
public:
	explicit TileWidget(QWidget *parent = nullptr);
	virtual ~TileWidget();
	
	uint8_t selectedTile() const;
	
signals:
	void tileSelected(uint8_t tileNo);
	
protected:
	void mousePressEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
	QSize sizeHint() const override;
	
	void highlightAttributeChanged() override;
	void scaleChanged() override;
	void tilesetChanged() override;
	
private:
	QSize tileSize() const;
	QSize imageSize() const;
	void makeImage();
	int tileColumns() const;
	int rowsPerColumn() const;
	QRect tileRect(uint8_t tileNo, bool withMargin = true) const;
	
	QImage *_image = nullptr;
	uint8_t _selectedTile = 0;
};

#endif // TILEWIDGET_H
