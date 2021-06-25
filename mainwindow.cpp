#include "mainwindow.h"
#include <QDir>
#include <QFileDialog>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
#include "iconfactory.h"
#include "map.h"
#include "multisignalblocker.h"
#include "tileset.h"
#include <QtDebug>


static constexpr char SETTINGS_TILESET_DIRECTORY[] = "General/TilesetDirectory";
static constexpr char SETTINGS_TILESET_PATH[] = "General/TilesetPath";
static constexpr char SETTINGS_WINDOW_GEOMETRY[] = "General/WindowGeometry";
static constexpr char SETTINGS_WINDOW_MAXIMIZED[] = "General/WindowMaximized";


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	_ui.setupUi(this);
	_ui.actionSelect->setChecked(true);
	_ui.toolBar->toggleViewAction()->setEnabled(false);
	_ui.actionShowObjects->setChecked(true);
	_ui.scrollAreaMap->setPanButton(Qt::RightButton);
	_ui.objectEditor->setVisible(false);
	_ui.actionPlaceDoor->setIcon(_iconFactory.icon("Door"));
	_ui.actionPlaceElevator->setIcon(_iconFactory.icon("Lift"));
	_ui.actionPlaceHiddenItem->setIcon(_iconFactory.icon("Item"));
	_ui.actionPlaceKey->setIcon(_iconFactory.icon("Key"));
	_ui.actionPlaceTransporterPad->setIcon(_iconFactory.icon("Pad"));
	_ui.actionPlaceTrashCompactor->setIcon(_iconFactory.icon("TC"));
	_ui.actionPlaceWaterRaft->setIcon(_iconFactory.icon("Raft"));
	
	QFontMetrics fm(QApplication::font());
	_labelHiddenObjectsCount = new QLabel(this);
	_labelMapFeatureCount = new QLabel(this);
	_labelRobotCount = new QLabel(this);
	updateMapCountLabels();
	_labelStatusCoords = new QLabel(this);
	_labelStatusCoords->setMinimumWidth(fm.width("Map Tile: 000/00"));
	_labelStatusCoords->setAlignment(Qt::AlignLeading | Qt::AlignBaseline);
	_labelStatusTile = new QLabel(this);
	_ui.statusbar->addPermanentWidget(_labelRobotCount);
	_ui.statusbar->addPermanentWidget(_labelMapFeatureCount);
	_ui.statusbar->addPermanentWidget(_labelHiddenObjectsCount);
	_ui.statusbar->addPermanentWidget(_labelStatusCoords);
	_ui.statusbar->addPermanentWidget(_labelStatusTile);
	
	_tileset = new Tileset();
	autoLoadTileset();
	_ui.mapWidget->setTileset(_tileset);
	_ui.tileWidget->setTileset(_tileset);
	
	_map = new Map();
	_ui.mapWidget->setMap(_map);
	_ui.objectEditor->setMap(_map);
	updateMapCountLabels();
	
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
	connect(_ui.actionDeleteObject, &QAction::triggered, this, &MainWindow::onActionDeleteObjectTriggered);
	for (QAction *action : { _ui.actionPlaceDoor, _ui.actionPlaceElevator, _ui.actionPlaceHiddenItem, 
	                         _ui.actionPlaceKey, _ui.actionPlacePlayer, _ui.actionPlaceRobot,
	                         _ui.actionPlaceTransporterPad, _ui.actionPlaceTrashCompactor,
	                         _ui.actionPlaceWaterRaft }) {
		connect(action, &QAction::triggered, this, &MainWindow::onActionPlaceTriggered);
	}
	
	connect(_ui.actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
	connect(_ui.actionClickEveryTile, &QAction::triggered, _ui.mapWidget, &MapWidget::clickEveryTile);
	
	connect(_ui.mapWidget, &MapWidget::mouseOverTile, this, &MainWindow::onMouseOverTile);
	connect(_ui.mapWidget, &MapWidget::tileClicked, this, &MainWindow::onTileClicked);
	connect(_ui.mapWidget, &MapWidget::tileDragged, this, &MainWindow::onTileDragged);
	connect(_ui.mapWidget, &MapWidget::objectClicked, this, &MainWindow::onObjectClicked);
	connect(_ui.tileWidget, &TileWidget::tileSelected, this, &MainWindow::onTileWidgetTileSelected);
	connect(_ui.objectEditor, &ObjectEditWidget::mapClickRequested, this, &MainWindow::onObjectEditMapClickRequested);
	
	connect(_map, &Map::objectsChanged, this, &MainWindow::onMapObjectChanged);
	
	const QSettings settings;
	const QRect geometry = settings.value(SETTINGS_WINDOW_GEOMETRY).toRect();
	if (geometry.isValid()) { setGeometry(geometry); }
	const bool maximized = settings.value(SETTINGS_WINDOW_MAXIMIZED).toBool();
	if (maximized) { showMaximized(); }
}


MainWindow::~MainWindow() {
	QSettings settings;
	settings.setValue(SETTINGS_WINDOW_GEOMETRY, geometry());
	settings.setValue(SETTINGS_WINDOW_MAXIMIZED, isMaximized());
}


void MainWindow::keyPressEvent(QKeyEvent *event) {
	if (event->key() == Qt::Key_Escape and event->modifiers() == Qt::NoModifier) {
		event->accept();
		if (not _ui.actionSelect->isChecked()) {
			_ui.actionSelect->trigger();
		}
		if (_objectEditMapClickRequested) {
			_ui.statusbar->clearMessage();
			_ui.objectEditor->mapClickCancelled();
			_objectEditMapClickRequested = false;
		}
	} else {
		QMainWindow::keyPressEvent(event);
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


void MainWindow::onActionDeleteObjectTriggered() {
	activateTool(_ui.actionDeleteObject);
	_ui.statusbar->showMessage("Delete object");
}


void MainWindow::onActionSelectTriggered() {
	activateTool(_ui.actionSelect);
	_ui.statusbar->clearMessage();
}


void MainWindow::onActionDrawTilesTriggered() {
	activateTool(_ui.actionDrawTiles);
	_ui.statusbar->showMessage("Draw tile");
	updateLabelStatusTile();
}


void MainWindow::onActionPlaceTriggered() {
	QAction *action = qobject_cast<QAction*>(sender());
	Q_ASSERT(action);
	activateTool(action);
	_ui.statusbar->showMessage(action->toolTip());
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


void MainWindow::onMapObjectChanged() {
	updateMapCountLabels();
}


void MainWindow::onMouseOverTile(int x, int y) {
	_labelStatusCoords->setText(QString("Map Tile: %1/%2").arg(x).arg(y));
}


void MainWindow::onObjectClicked(int objectNo) {
	if (_ui.actionSelect->isChecked() and not _objectEditMapClickRequested) {
		_ui.objectEditor->loadObject(objectNo);
	} else if (_ui.actionDeleteObject->isChecked()) {
		_map->setObject(objectNo, Map::Object());
		_map->compact();
	}
}


void MainWindow::onObjectEditMapClickRequested(const QString &label) {
	_ui.statusbar->showMessage(QString("click on map to set %1").arg(label));
	_objectEditMapClickRequested = true;
}


void MainWindow::onTileClicked(int x, int y) {
	if (_objectEditMapClickRequested) {
		_ui.statusbar->clearMessage();
		_ui.objectEditor->mapClick(x, y);
		_objectEditMapClickRequested = false;
	} else if (_ui.actionDrawTiles->isChecked()) {
		int tileNo = _ui.tileWidget->selectedTile();
		if (0 <= tileNo and tileNo < TILE_COUNT) {
			_map->setTile(x, y, tileNo);
		}
	} else if (_ui.actionPlaceDoor->isChecked()) {
		placeObject(x, y, OBJECT_DOOR, 0, 5);
	} else if (_ui.actionPlaceElevator->isChecked()) {
		placeObject(x, y, OBJECT_ELEVATOR, 0, 2, 1, 1);
	} else if (_ui.actionPlaceHiddenItem->isChecked()) {
		placeObject(x, y, OBJECT_TIME_BOMB, 10);
	} else if (_ui.actionPlaceKey->isChecked()) {
		placeObject(x, y, OBJECT_KEY);
	} else if (_ui.actionPlacePlayer->isChecked()) {
		Map::Object playerObject = _map->object(PLAYER_OBJ);
		playerObject.x = x;
		playerObject.y = y;
		playerObject.a = playerObject.b = playerObject.c = playerObject.d = 0;
		if (playerObject.health == 0) { playerObject.health = 12; }
		_map->setObject(PLAYER_OBJ, playerObject);
	} else if (_ui.actionPlaceRobot->isChecked()) {
		placeObject(x, y, ROBOT_HOVERBOT_LR, 0, 0, 0, 0, 10);
	} else if (_ui.actionPlaceTransporterPad->isChecked()) {
		placeObject(x, y, OBJECT_TRANSPORTER, 1);
	} else if (_ui.actionPlaceTrashCompactor->isChecked()) {
		placeObject(x, y, OBJECT_TRASH_COMPACTOR);
	} else if (_ui.actionPlaceWaterRaft->isChecked()) {
		placeObject(x, y, OBJECT_WATER_RAFT, 0, 0, x - 1, x + 1);
	}
}


void MainWindow::onTileDragged(int x, int y) {
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
	for (QAction *toolAction : { _ui.actionSelect, _ui.actionDrawTiles, _ui.actionDeleteObject,
	     _ui.actionPlaceDoor, _ui.actionPlaceElevator, _ui.actionPlaceHiddenItem,
	     _ui.actionPlaceKey, _ui.actionPlacePlayer, _ui.actionPlaceRobot,
	     _ui.actionPlaceTransporterPad, _ui.actionPlaceTrashCompactor, _ui.actionPlaceWaterRaft }) {
		toolAction->setChecked(action == toolAction);
	}
	_ui.mapWidget->setShowSelected(action == _ui.actionSelect);
	_ui.tileWidget->setShowSelected(action == _ui.actionDrawTiles);
	_labelStatusTile->setVisible(action == _ui.actionDrawTiles);
	
	if (not _ui.actionSelect->isChecked()) { _ui.objectEditor->loadObject(-1); }
}


void MainWindow::placeObject(int x, int y, int unitType, int a, int b, int c, int d, int health) {
	const Map::Object::Kind kind = Map::Object::kind(unitType);
	const int objectNo = _map->nextAvailableObjectId(kind);
	if (objectNo == -1) {
		const QString &type = Map::Object::toString(kind);
		const QString &category = Map::Object::category(kind);
		
		QMessageBox::warning(this, QString("Cannot Add %1%2").arg(type.left(1).toUpper(), type.mid(1)),
		                     QString("Cannot add %1: the map already contains the maximum number "
		                             "of %2").arg(type).arg(category));
		return;
	}
	Map::Object object(unitType);
	object.x = x;
	object.y = y;
	object.a = a;
	object.b = b;
	object.c = c;
	object.d = d;
	object.health = health;
	_map->setObject(objectNo, object);
}


void MainWindow::placeRobot(int x, int y) {
	const int objectNo = _map->nextAvailableObjectId(Map::Object::Kind::Robot);
	if (objectNo == -1) {
		QMessageBox::warning(this, "Cannot add robot",
		                     "Cannot add robot: the maximum number of robots has already been "
		                     "reached");
		return;
	}
	Map::Object object(ROBOT_HOVERBOT_LR);
	object.x = x;
	object.y = y;
	object.health = 10;
	_map->setObject(objectNo, object);
}


void MainWindow::updateLabelStatusTile() {
	_labelStatusTile->setText(QString("Selected Tile: 0x%1")
	                          .arg(_ui.tileWidget->selectedTile(), 2, 16, QChar('0')));
}


void MainWindow::updateMapCountLabels() {
	if (_map == nullptr) {
		_labelHiddenObjectsCount->clear();
		_labelMapFeatureCount->clear();
		_labelRobotCount->clear();
		return;
	}
	
	static constexpr int maxHiddenItemCount = HIDDEN_OBJECT_MAX - HIDDEN_OBJECT_MIN + 1;
	static constexpr int maxMapFeatureCount = MAP_FEATURE_MAX - MAP_FEATURE_MIN + 1;
	static constexpr int maxRobotCount = ROBOT_MAX - ROBOT_MIN + 1;
	
	_labelRobotCount->setText(QString("Robots: %1/%2").arg(_map->robotCount())
	                          .arg(maxRobotCount));
	_labelMapFeatureCount->setText(QString("Map features: %1/%2").arg(_map->mapFeatureCount())
	                               .arg(maxMapFeatureCount));
	_labelHiddenObjectsCount->setText(QString("Hidden objects: %1/%2").arg(_map->hiddenItemCount())
	                                  .arg(maxHiddenItemCount));
}
