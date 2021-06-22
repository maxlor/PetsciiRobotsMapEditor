#include "mainwindow.h"
#include <QMessageBox>
#include "map.h"
#include "multisignalblocker.h"
#include "tileset.h"
#include <QtDebug>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	_ui.setupUi(this);
	_ui.actionShowObjects->setChecked(true);
	_ui.objectEditor->setVisible(false);
	_labelStatusCoords = new QLabel(this);
	_ui.statusbar->addPermanentWidget(_labelStatusCoords);
	
	_tileset = new Tileset();
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
	
	connect(_ui.actionQuit, &QAction::triggered, this, &MainWindow::onQuit);
	for (QAction *action : _viewFilterActions) {
		connect(action, &QAction::triggered, this, &MainWindow::onViewFilterChanged);
	}
	connect(_ui.actionZoomIn, &QAction::triggered, _ui.tileWidget, &TileWidget::zoomIn);
	connect(_ui.actionZoomOut, &QAction::triggered, _ui.tileWidget, &TileWidget::zoomOut);
	connect(_ui.actionZoomIn, &QAction::triggered, _ui.mapWidget, &MapWidget::zoomIn);
	connect(_ui.actionZoomOut, &QAction::triggered, _ui.mapWidget, &MapWidget::zoomOut);
	connect(_ui.actionShowObjects, &QAction::toggled, _ui.mapWidget, &MapWidget::setObjectsVisible);
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
	
	connect(_ui.mapWidget, &MapWidget::mouseOverTile, this, &MainWindow::onMouseOverTile);
	connect(_ui.mapWidget, &MapWidget::objectClicked, _ui.objectEditor, &ObjectEditWidget::loadObject);
	connect(_ui.actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
	connect(_ui.actionClickEveryTile, &QAction::triggered, _ui.mapWidget, &MapWidget::clickEveryTile);
}


MainWindow::~MainWindow() {}


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


void MainWindow::onMouseOverTile(int x, int y) {
	_labelStatusCoords->setText(QString("%1/%2").arg(x).arg(y));
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
