#include "tilewidget.h"
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include "constants.h"
#include "tile.h"
#include "tileset.h"


static const int TILES_PER_ROW = 4;
static const int OUTER_MARGIN = 1;
static const int TILE_MARGIN = 1;


TileWidget::TileWidget(QWidget *parent) : AbstractTileWidget(parent) {
	setShowSelected(true); // TODO remove
}


TileWidget::~TileWidget() {
	delete _image;
}


uint8_t TileWidget::selectedTile() const {
	return _selectedTile;
}


void TileWidget::mousePressEvent(QMouseEvent *event) {
	const QPoint p = (event->pos() - QPoint(OUTER_MARGIN, OUTER_MARGIN));
	if (p.x() < 0 or p.y() < 0) { return; }
	int col = p.x() / (tileSize().width() * scale());
	int row = p.y() / (tileSize().height() * scale());
	const int outerCol = col / TILES_PER_ROW;
	col -= outerCol * TILES_PER_ROW;
	row += outerCol * rowsPerColumn();
	_selectedTile = row * TILES_PER_ROW + col;
	update();
	emit tileSelected(_selectedTile);
}


void TileWidget::paintEvent(QPaintEvent *event) {
	Q_UNUSED(event);
	if (tileset() == nullptr) { return; }
	QPainter painter(this);
	
	// draw tiles
	painter.translate(OUTER_MARGIN, OUTER_MARGIN);
	painter.scale(scale(), scale());
	painter.drawImage(0, 0, *_image);
	
	// draw highlight overlay
	if (highlightAttribute() != Tile::None) {
		for (int tileNo = 0; tileNo < 256; ++tileNo) {
			const Tile tile = tileset()->tile(tileNo);
			painter.setBrush(tile.attributes().testFlag(highlightAttribute()) ?
			                     highlightColor() : noHighlightColor());
			painter.drawRect(tileRect(tileNo));
		}
	}
	
	// draw active border
	if (showSelected()) {
		painter.setPen(Qt::NoPen);
		painter.setBrush(C::colorTileSelection);
		drawMargin(painter, tileRect(_selectedTile, false), TILE_MARGIN * 2);
	}
}


QSize TileWidget::sizeHint() const {
	return imageSize() * scale() + QSize(OUTER_MARGIN * 2, OUTER_MARGIN * 2);
}


void TileWidget::highlightAttributeChanged() {
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


QSize TileWidget::tileSize() const {
	return tileset()->tileSize() + QSize(2 * TILE_MARGIN, 2 * TILE_MARGIN);
}


QSize TileWidget::imageSize() const {
	const int horiTiles = tileColumns() * TILES_PER_ROW;
	const int vertTiles = (TILE_COUNT + (TILES_PER_ROW - 1)) / horiTiles;
	return QSize(horiTiles * tileSize().width(), vertTiles * tileSize().height());
}


void TileWidget::makeImage() {
	delete _image;
	_image = new QImage(imageSize(), IMAGE_FORMAT);
	QPainter painter(_image);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::darkGray);
	
	for (int tileNo = 0; tileNo < 256; ++tileNo) {
		const Tile tile = tileset()->tile(tileNo);
		const QRect r = tileRect(tileNo, false);
		drawMargin(painter, r, TILE_MARGIN);
		painter.drawImage(r.topLeft(), tile.image());
	}
}


int TileWidget::tileColumns() const {
	return 2 / scale();
}


int TileWidget::rowsPerColumn() const {
	static constexpr int rows = (TILE_COUNT + TILES_PER_ROW - 1) / TILES_PER_ROW;
	return (rows + tileColumns() - 1) / tileColumns();
}


QRect TileWidget::tileRect(uint8_t tileNo, bool withMargin) const {
	const int tilesPerColumn = rowsPerColumn() * TILES_PER_ROW;
	const int outerCol = tileNo / tilesPerColumn;
	tileNo -= outerCol * tilesPerColumn;
	const int row = tileNo / TILES_PER_ROW;
	tileNo -= row * TILES_PER_ROW;
	const int col = tileNo;
	QRect result((outerCol * TILES_PER_ROW + col) * tileSize().width(), row * tileSize().height(),
	             tileSize().width(), tileSize().height());
	if (not withMargin) {
		result.adjust(TILE_MARGIN, TILE_MARGIN, -TILE_MARGIN, -TILE_MARGIN);
	}
	return result;
}
