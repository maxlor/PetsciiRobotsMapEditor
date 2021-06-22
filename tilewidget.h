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
	
protected:
	void paintEvent(QPaintEvent *event) override;
	QSize sizeHint() const override;
	
	void flagsChanged() override;
	void scaleChanged() override;
	void tilesetChanged() override;
	
private:
	QSize imageSize() const;
	void makeImage();
	int tileColumns() const;
	QRect tileRect(int tileNo) const;
	
	QImage *_image = nullptr;
};

#endif // TILEWIDGET_H
