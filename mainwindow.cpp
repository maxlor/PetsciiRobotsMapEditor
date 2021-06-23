#include "mainwindow.h"
#include <QDir>
#include <QFileDialog>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
#include "map.h"
#include "multisignalblocker.h"
#include "tileset.h"
#include <QtDebug>


static constexpr char SETTINGS_TILESET_DIRECTORY[] = "General/TilesetDirectory";
static constexpr char SETTINGS_TILESET_PATH[] = "General/TilesetPath";


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	_ui.setupUi(this);
	_ui.actionSelect->setChecked(true);
	_ui.toolBar->toggleViewAction()->setEnabled(false);
	_ui.actionShowObjects->setChecked(true);
	_ui.scrollAreaMap->setPanButton(Qt::RightButton);
	_ui.objectEditor->setVisible(false);
	QFontMetrics fm(QApplication::font());
	_labelStatusCoords = new QLabel(this);
	_labelStatusCoords->setMinimumWidth(fm.width("Map Tile: 000/00"));
	_labelStatusCoords->setAlignment(Qt::AlignLeading | Qt::AlignBaseline);
	_labelStatusTile = new QLabel(this);
	_ui.statusbar->addPermanentWidget(_labelStatusCoords);
	_ui.statusbar->addPermanentWidget(_labelStatusTile);
	
	_tileset = new Tileset();
	autoLoadTileset();
	_ui.mapWidget->setTileset(_tileset);
	_ui.tileWidget->setTileset(_tileset);
	
	_map = new Map();
	_ui.mapWidget->setMap(_map);
	_ui.objectEditor->setMap(_map);
	
	_viewFilterActions.push_front(_ui.actionShowSearchable);
	_viewFilterActions.push_front(_ui.actionShowPushOnto);
	_viewFilterActions.push_front(_ui.actionShowShootThrough);
	_viewFilterActions.push_front(_ui.actionShowDestructible);
	_viewFilterActions.push_front(_ui.actionShowMoveable);
	_viewFilterActions.push_front(_ui.actionShowHoverable);
	_viewFilterActions.push_front(_ui.actionShowWalkable);
	
	connect(_ui.actionLoadTileset, &QAction::triggered, this, &MainWindow::onLoadTileset);
	connect(_ui.actionQuit, &QAction::triggered, this, &MainWindow::onQuit);
	for (QAction *action : _viewFilterActions) {
		connect(action, &QAction::triggered, this, &MainWindow::onViewFilterChanged);
	}
	connect(_ui.actionZoomIn, &QAction::triggered, _ui.tileWidget, &TileWidget::zoomIn);
	connect(_ui.actionZoomOut, &QAction::triggered, _ui.tileWidget, &TileWidget::zoomOut);
	connect(_ui.actionZoomIn, &QAction::triggered, _ui.mapWidget, &MapWidget::zoomIn);
	connect(_ui.actionZoomOut, &QAction::triggered, _ui.mapWidget, &MapWidget::zoomOut);
	connect(_ui.actionShowObjects, &QAction::toggled, _ui.mapWidget, &MapWidget::setObjectsVisible);
	connect(_ui.actionShowGrid, &QAction::toggled, _ui.mapWidget, &MapWidget::setShowGridLines);
	connect(_ui.actionShowWalkable, &QAction::toggled, _ui.mapWidget, &MapWidget::setShowWalkable);
	connect(_ui.actionShowHoverable, &QAction::toggled, _ui.mapWidget, &MapWidget::setShowHoverable);
	connect(_ui.actionShowMoveable, &QAction::toggled, _ui.mapWidget, &MapWidget::setShowMovable);
	connect(_ui.actionShowDestructible, &QAction::toggled, _ui.mapWidget, &MapWidget::setShowDestructible);
	connect(_ui.actionShowShootThrough, &QAction::toggled, _ui.mapWidget, &MapWidget::setShowShootThrough);
	connect(_ui.actionShowPushOnto, &QAction::toggled, _ui.mapWidget, &MapWidget::setShowPushOnto);
	connect(_ui.actionShowSearchable, &QAction::toggled, _ui.mapWidget, &MapWidget::setShowSearchable);
	connect(_ui.actionShowWalkable, &QAction::toggled, _ui.tileWidget, &TileWidget::setShowWalkable);
	connect(_ui.actionShowHoverable, &QAction::toggled, _ui.tileWidget, &TileWidget::setShowHoverable);
	connect(_ui.actionShowMoveable, &QAction::toggled, _ui.tileWidget, &TileWidget::setShowMovable);
	connect(_ui.actionShowDestructible, &QAction::toggled, _ui.tileWidget, &TileWidget::setShowDestructible);
	connect(_ui.actionShowShootThrough, &QAction::toggled, _ui.tileWidget, &TileWidget::setShowShootThrough);
	connect(_ui.actionShowPushOnto, &QAction::toggled, _ui.tileWidget, &TileWidget::setShowPushOnto);
	connect(_ui.actionShowSearchable, &QAction::toggled, _ui.tileWidget, &TileWidget::setShowSearchable);
	connect(_ui.actionSelect, &QAction::triggered, this, &MainWindow::onActionSelectTriggered);
	connect(_ui.actionDrawTiles, &QAction::triggered, this, &MainWindow::onActionDrawTilesTriggered);
	connect(_ui.actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
	connect(_ui.actionClickEveryTile, &QAction::triggered, _ui.mapWidget, &MapWidget::clickEveryTile);
	
	connect(_ui.mapWidget, &MapWidget::mouseOverTile, this, &MainWindow::onMouseOverTile);
	connect(_ui.mapWidget, &MapWidget::tileClicked, this, &MainWindow::onTileClicked);
	connect(_ui.mapWidget, &MapWidget::tileDragged, this, &MainWindow::onTileClicked);
	connect(_ui.mapWidget, &MapWidget::objectClicked, this, &MainWindow::onObjectClicked);
	connect(_ui.tileWidget, &TileWidget::tileSelected, this, &MainWindow::onTileWidgetTileSelected);
}


MainWindow::~MainWindow() {}


void MainWindow::keyPressEvent(QKeyEvent *event) {
	if (event->key() == Qt::Key_Escape and event->modifiers() == Qt::NoModifier) {
		event->accept();
		if (not _ui.actionSelect->isChecked()) {
			_ui.actionSelect->trigger();
		}
	}
}


void MainWindow::onAbout() {
	QMessageBox::about(this, tr("About %1").arg(QApplication::applicationDisplayName()),
	                   tr(
"<html><h3>%1 %2</h3>"
"<p>Copyright 2021 Benjamin Lutz</p>"
"<p>This program allows the creation and editing of maps for the game "
"<a href=\"http://www.the8bitguy.com/product-category/music/\">Attack of the PETSCII Robots</a> by "
"David Murray, also known as <a href=\"http://the8bitguy.com\">The 8-Bit Guy</a>.</p>"
"<h4>License</h4>"
"<p>This program is free software: you can redistribute it and/or modify it under the terms of the "
"GNU General Public License as published by the Free Software Foundation, either version 3 of the "
"License, or (at your option) any later version.</p>"
"<p>This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
"without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See "
"the GNU General Public License for more details.</p>"
"<p>You should have received a copy of the GNU General Public License along with this program. If "
"not, see <a href=\"https://www.gnu.org/licenses/gpl.html\">"
"https://www.gnu.org/licenses/gpl.html</a>.</p>")
	                   .arg(QApplication::applicationDisplayName(),
	                        QApplication::applicationVersion()));
}


void MainWindow::onActionSelectTriggered() {
	activateTool(_ui.actionSelect);
}


void MainWindow::onActionDrawTilesTriggered() {
	activateTool(_ui.actionDrawTiles);
	updateLabelStatusTile();
}


void MainWindow::onLoadTileset() {
	QSettings settings;
	QString directory = settings.value(SETTINGS_TILESET_DIRECTORY, QDir::homePath()).toString();
	QFileDialog *d = new QFileDialog(this, "Load Tileset...", directory);
	d->setAcceptMode(QFileDialog::AcceptOpen);
	d->setFileMode(QFileDialog::ExistingFile);
	d->setNameFilters({ "tileset.pet", "Any file (*)"});
	int result = d->exec();
	if (result == QFileDialog::Accepted) {
		if (d->selectedFiles().size() > 0) {
			const QString path = d->selectedFiles().at(0);
			settings.setValue(SETTINGS_TILESET_DIRECTORY, d->directory().canonicalPath());
			QString error = _tileset->load(path);
			if (error.isNull()) {
				settings.setValue(SETTINGS_TILESET_PATH, path);
			} else {
				QMessageBox::critical(this, "Error Loading Tileset",
				                      QString("Cannot load tileset: %1").arg(error));
			}
		}
	}
	delete d;
}


void MainWindow::onMouseOverTile(int x, int y) {
	_labelStatusCoords->setText(QString("Map Tile: %1/%2").arg(x).arg(y));
}


void MainWindow::onObjectClicked(int objectNo) {
	if (_ui.actionSelect->isChecked()) {
		_ui.objectEditor->loadObject(objectNo);
	}
}


void MainWindow::onTileClicked(int x, int y) {
	if (_ui.actionDrawTiles->isChecked()) {
		int tileNo = _ui.tileWidget->selectedTile();
		if (0 <= tileNo and tileNo < TILE_COUNT) {
			_map->setTile(x, y, tileNo);
		}
	}
}


void MainWindow::onTileWidgetTileSelected(int tileNo) {
	Q_UNUSED(tileNo);
	if (not _ui.actionDrawTiles->isChecked()) {
		_ui.actionDrawTiles->trigger();
	}
	updateLabelStatusTile();
}


void MainWindow::onQuit() {
	close();
}


void MainWindow::onViewFilterChanged() {
	for (QAction *action : _viewFilterActions) {
		if (action == sender()) { continue; }
		action->setChecked(false);
	}
}


void MainWindow::autoLoadTileset() {
	QSettings settings;
	const QString path = settings.value(SETTINGS_TILESET_PATH).toString();
	if (not path.isNull()) {
		QString error = _tileset->load(path);
		if (error.isNull()) {
			return;
		}
	}
	
	QMetaObject::invokeMethod(this, [&]() {
		static const QString text = QString(
					"<html><p>To use this map editor, you need to load the tileset. It is shipped "
					"with the game <i>Attack of the PETSCII Robots</i>, the file you need is "
					"called <b>tileset.pet</b>.<p><p>Load the tileset now?</p>");
		
		QMessageBox::StandardButton button = QMessageBox::question(this, "Load Tileset?", text);
		if (button == QMessageBox::Yes) {
			onLoadTileset();
		}
		}, Qt::QueuedConnection);
}


void MainWindow::activateTool(QAction *const action) {
	_ui.actionSelect->setChecked(action == _ui.actionSelect);
	_ui.actionDrawTiles->setChecked(action == _ui.actionDrawTiles);
	_ui.mapWidget->setShowSelected(action == _ui.actionSelect);
	_ui.tileWidget->setShowSelected(action == _ui.actionDrawTiles);
	_labelStatusTile->setVisible(action == _ui.actionDrawTiles);
	
	if (not _ui.actionSelect->isChecked()) { _ui.objectEditor->loadObject(-1); }
}


void MainWindow::updateLabelStatusTile() {
	_labelStatusTile->setText(QString("Selected Tile: 0x%1")
	                          .arg(_ui.tileWidget->selectedTile(), 2, 16, QChar('0')));
}
