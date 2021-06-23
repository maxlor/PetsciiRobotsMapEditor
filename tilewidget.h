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
	
	int selectedTile() const;
	
signals:
	void tileSelected(int tileNo);
	
protected:
	void mousePressEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
	QSize sizeHint() const override;
	
	void highlightFlagsChanged() override;
	void scaleChanged() override;
	void tilesetChanged() override;
	
private:
	QSize tileSize() const;
	QSize imageSize() const;
	void makeImage();
	int tileColumns() const;
	int rowsPerColumn() const;
	QRect tileRect(int tileNo, bool withMargin = true) const;
	
	QImage *_image = nullptr;
	int _selectedTile = -1;
};

#endif // TILEWIDGET_H
