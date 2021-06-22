#include "tilewidget.h"
#include <QImage>
#include <QPainter>
#include <QRect>
#include "constants.h"
#include "tile.h"
#include "tileset.h"
#include <QtDebug>


static const int TILES_PER_ROW = 4;
static const int MARGIN = 2;


TileWidget::TileWidget(QWidget *parent) : AbstractTileWidget(parent) {
	
}


TileWidget::~TileWidget() {
	delete _image;
}


void TileWidget::paintEvent(QPaintEvent *event) {
	Q_UNUSED(event);
	if (tileset() == nullptr) { return; }
	QPainter painter(this);
	painter.scale(scale(), scale());
	if (scale() == 2) {
		painter.translate(-MARGIN * 0.25, 0);
	}
	painter.drawImage(0, 0, *_image);
	painter.setPen(Qt::NoPen);
	
	
	if (highlightFlags()) {
		for (int tileNo = 0; tileNo < 256; ++tileNo) {
			const Tile tile = tileset()->tile(tileNo);
			painter.setBrush(tile.flags() & highlightFlags() ? highlightColor() : noHighlightColor());
			painter.drawRect(tileRect(tileNo));
		}
	}
}


QSize TileWidget::sizeHint() const {
	QSize result = imageSize() * scale() ;
	if (scale() == 2) {
		result -= QSize(MARGIN, 0);
	}
	return result;
}


void TileWidget::flagsChanged() {
	makeImage();
	update();
}


void TileWidget::scaleChanged() {
	makeImage();
	update();
}


void TileWidget::tilesetChanged() {
	makeImage();
	update();
}


QSize TileWidget::imageSize() const {
	const int horiTiles = tileColumns() * TILES_PER_ROW;
	const int vertTiles = 256 / horiTiles;
	return QSize(horiTiles * (TILE_WIDTH * GLYPH_WIDTH + MARGIN) + MARGIN,
	             vertTiles * (TILE_HEIGHT * GLYPH_HEIGHT + MARGIN) + MARGIN);
}


void TileWidget::makeImage() {
	delete _image;
	_image = new QImage(imageSize(), QImage::Format_ARGB32_Premultiplied);
	QPainter painter(_image);
	painter.fillRect(_image->rect(), Qt::darkGray);
	
	for (int tileNo = 0; tileNo < 256; ++tileNo) {
		const Tile tile = tileset()->tile(tileNo);
		painter.drawImage(tileRect(tileNo), tile.image());
	}
}


int TileWidget::tileColumns() const {
	return 2 / scale();
}


QRect TileWidget::tileRect(int tileNo) const {
	int tileCol = tileNo / (256 / tileColumns());
	tileNo -= tileCol * (256 / tileColumns());
	int row = tileNo / TILES_PER_ROW;
	tileNo -= row * TILES_PER_ROW;
	int col = tileNo;
	return QRect((tileCol * TILES_PER_ROW + col) * (TILE_WIDTH * GLYPH_WIDTH + MARGIN) + MARGIN,
	             row * TILE_HEIGHT * GLYPH_HEIGHT + (row + 1) * MARGIN,
	             TILE_WIDTH * GLYPH_WIDTH, TILE_HEIGHT * GLYPH_HEIGHT);
}
