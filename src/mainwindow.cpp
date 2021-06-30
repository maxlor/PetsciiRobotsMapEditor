#include "mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
#include <QStringList>
#include "iconfactory.h"
#include "map.h"
#include "multisignalblocker.h"
#include "tileset.h"
#include <QtDebug>


static constexpr char SETTINGS_TILESET_DIRECTORY[] = "General/TilesetDirectory";
static constexpr char SETTINGS_TILESET_PATH[] = "General/TilesetPath";
static constexpr char SETTINGS_WINDOW_GEOMETRY[] = "General/WindowGeometry";
static constexpr char SETTINGS_WINDOW_MAXIMIZED[] = "General/WindowMaximized";
static constexpr char SETTINGS_MAP_DIRECTORY[] = "General/MapDirectory";
static constexpr char SETTINGS_MAP_PATH[] = "General/MapPath";


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	_ui.setupUi(this);
	_ui.actionSelect->setChecked(true);
	_ui.toolBar->toggleViewAction()->setEnabled(false);
	_ui.actionShowObjects->setChecked(true);
	_ui.scrollAreaMap->setPanButton(Qt::RightButton);
	_ui.objectEditor->setVisible(false);
	
	_ui.actionNew->setIcon(QIcon::fromTheme("document-new"));
	_ui.actionOpen->setIcon(QIcon::fromTheme("document-open"));
	_ui.actionSave->setIcon(QIcon::fromTheme("document-save"));
	_ui.actionSaveAs->setIcon(QIcon::fromTheme("document-save-as"));
	_ui.actionQuit->setIcon(QIcon::fromTheme("application-exit"));
	_ui.actionCopyArea->setIcon(QIcon::fromTheme("edit-copy"));
	_ui.actionCopyObjects->setIcon(QIcon::fromTheme("edit-copy"));
	_ui.actionCutArea->setIcon(QIcon::fromTheme("edit-cut"));
	_ui.actionCutObjects->setIcon(QIcon::fromTheme("edit-cut"));
	_ui.actionPaste->setIcon(QIcon::fromTheme("edit-paste"));
	_ui.actionPlaceDoor->setIcon(_iconFactory.icon("Door"));
	_ui.actionPlaceElevator->setIcon(_iconFactory.icon("Lift"));
	_ui.actionPlaceHiddenItem->setIcon(_iconFactory.icon("Item"));
	_ui.actionPlaceKey->setIcon(_iconFactory.icon("Key"));
	_ui.actionPlaceTransporterPad->setIcon(_iconFactory.icon("Pad"));
	_ui.actionPlaceTrashCompactor->setIcon(_iconFactory.icon("TC"));
	_ui.actionPlaceWaterRaft->setIcon(_iconFactory.icon("Raft"));
	_ui.actionAbout->setIcon(QIcon::fromTheme("help-about"));
	
	QFontMetrics fm(QApplication::font());
	_labelHiddenObjectsCount = new QLabel(this);
	_labelMapFeatureCount = new QLabel(this);
	_labelRobotCount = new QLabel(this);
	updateMapCountLabels();
	_labelStatusCoords = new QLabel(this);
	_labelStatusCoords->setMinimumWidth(fm.width("Map Tile: 000, 00"));
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
	
	_mapController = new MapController(this);
	_ui.mapWidget->setMap(_mapController->map());
	_ui.objectEditor->setMapController(_mapController);
	updateMapCountLabels();
	
	_viewFilterActions = { _ui.actionShowWalkable, _ui.actionShowHoverable,
	                       _ui.actionShowMoveable, _ui.actionShowDestructible,
	                       _ui.actionShowShootThrough, _ui.actionShowPushOnto,
	                       _ui.actionShowSearchable };
	_toolActions = { _ui.actionSelect, _ui.actionSelectArea, _ui.actionDrawTiles,
	                 _ui.actionFloodFill, _ui.actionDeleteObject,
	                 _ui.actionPlaceDoor, _ui.actionPlaceElevator, _ui.actionPlaceHiddenItem, 
	                 _ui.actionPlaceKey, _ui.actionPlacePlayer, _ui.actionPlaceRobot,
                     _ui.actionPlaceTransporterPad, _ui.actionPlaceTrashCompactor,
                     _ui.actionPlaceWaterRaft };
	
	connect(_ui.actionNew, &QAction::triggered, this, &MainWindow::onNewTriggered);
	connect(_ui.actionOpen, &QAction::triggered, this, &MainWindow::onOpenTriggered);
	connect(_ui.actionSave, &QAction::triggered, this, &MainWindow::onSaveTriggered);
	connect(_ui.actionSaveAs, &QAction::triggered, this, &MainWindow::onSaveAsTriggered);
	connect(_ui.actionLoadTileset, &QAction::triggered, this, &MainWindow::onLoadTileset);
	connect(_ui.actionQuit, &QAction::triggered, this, &MainWindow::onQuit);
	connect(_ui.actionCopyArea, &QAction::triggered, this, &MainWindow::onCopyAreaTriggered);
	connect(_ui.actionCopyObjects, &QAction::triggered, this, &MainWindow::onCopyObjectsTriggered);
	connect(_ui.actionCutArea, &QAction::triggered, this, &MainWindow::onCutAreaTriggered);
	connect(_ui.actionCutObjects, &QAction::triggered, this, &MainWindow::onCutObjectsTriggered);
	connect(_ui.actionPaste, &QAction::triggered, this, &MainWindow::onPasteTriggered);
	connect(_ui.actionFill, &QAction::triggered, this, &MainWindow::onFillTriggered);
	
	for (QAction *action : _viewFilterActions) {
		connect(action, &QAction::triggered, this, &MainWindow::onViewFilterChanged);
	}
	connect(_ui.actionZoomIn, &QAction::triggered, _ui.tileWidget, &TileWidget::zoomIn);
	connect(_ui.actionZoomOut, &QAction::triggered, _ui.tileWidget, &TileWidget::zoomOut);
	connect(_ui.actionZoomIn, &QAction::triggered, _ui.mapWidget, &MapWidget::zoomIn);
	connect(_ui.actionZoomOut, &QAction::triggered, _ui.mapWidget, &MapWidget::zoomOut);
	connect(_ui.actionShowObjects, &QAction::toggled, this, &MainWindow::onShowObjectsToggled);
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
	for (QAction *action : _toolActions) {
		connect(action, &QAction::triggered, this, &MainWindow::onToolActionTriggered);
	}
	
	connect(_ui.actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
	connect(_ui.actionClickEveryTile, &QAction::triggered, _ui.mapWidget, &MapWidget::clickEveryTile);
	
	connect(_ui.mapWidget, &MapWidget::mouseOverTile, this, &MainWindow::onMouseOverTile);
	connect(_ui.mapWidget, &MapWidget::objectClicked, this, &MainWindow::onObjectClicked);
	connect(_ui.mapWidget, &MapWidget::objectDragged, this, &MainWindow::onObjectDragged);
	connect(_ui.mapWidget, &MapWidget::tilePressed, this, &MainWindow::onTilePressed);
	connect(_ui.mapWidget, &MapWidget::tileDragged, this, &MainWindow::onTileDragged);
	connect(_ui.mapWidget, &MapWidget::tileReleased, this, &MainWindow::onTileReleased);
	connect(_ui.tileWidget, &TileWidget::tileSelected, this, &MainWindow::onTileWidgetTileSelected);
	connect(_ui.objectEditor, &ObjectEditWidget::mapClickRequested, this, &MainWindow::onObjectEditMapClickRequested);
	
	connect(_mapController->map(), &Map::objectsChanged, this, &MainWindow::onMapObjectChanged);
	connect(_mapController->map(), &Map::tilesChanged, this, &MainWindow::onMapTilesChanged);
	
	const QSettings settings;
	const QRect geometry = settings.value(SETTINGS_WINDOW_GEOMETRY).toRect();
	if (geometry.isValid()) { setGeometry(geometry); }
	const bool maximized = settings.value(SETTINGS_WINDOW_MAXIMIZED).toBool();
	if (maximized) { showMaximized(); }
	
	// reload map that was last open, fail silently.
	const QString path = settings.value(SETTINGS_MAP_PATH).toString();
	if (not path.isEmpty()) {
		QString error = _mapController->load(path);
		if (error.isNull()) {
			_path = path;
		} else {
			_mapController->clear();
		}
		_changed = false;
	}
	
	activateTool(_ui.actionSelect);
}


MainWindow::~MainWindow() {
	QSettings settings;
	settings.setValue(SETTINGS_WINDOW_GEOMETRY, geometry());
	settings.setValue(SETTINGS_WINDOW_MAXIMIZED, isMaximized());
}


void MainWindow::closeEvent(QCloseEvent *event) {
	if (not _changed or askSaveChanges()) {
		event->accept();
	} else {
		event->ignore();
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


void MainWindow::onLoadTileset() {
	QSettings settings;
	QString directory = settings.value(SETTINGS_TILESET_DIRECTORY, QDir::homePath()).toString();
	QFileDialog dialog(this, "Load Tileset...", directory);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilters({ "tileset.pet", "Any file (*)"});
	int result = dialog.exec();
	if (result == QFileDialog::Accepted) {
		if (dialog.selectedFiles().size() > 0) {
			const QString path = dialog.selectedFiles().at(0);
			settings.setValue(SETTINGS_TILESET_DIRECTORY, dialog.directory().canonicalPath());
			QString error = _tileset->load(path);
			if (error.isNull()) {
				settings.setValue(SETTINGS_TILESET_PATH, path);
			} else {
				QMessageBox::critical(this, "Error Loading Tileset",
				                      QString("Cannot load tileset: %1").arg(error));
			}
		}
	}
}


void MainWindow::onMapObjectChanged() {
	updateMapCountLabels();
	_changed = true;
}


void MainWindow::onMapTilesChanged() {
	_changed = true;
}


void MainWindow::onMouseOverTile(const QPoint &tile) {
	_labelStatusCoords->setText(QString("Map Tile: %1, %2").arg(tile.x()).arg(tile.y()));
}


void MainWindow::onObjectClicked(int objectNo) {
	if (_ui.actionSelect->isChecked() and not _objectEditMapClickRequested) {
		_ui.objectEditor->loadObject(objectNo);
	} else if (_ui.actionDeleteObject->isChecked()) {
		_mapController->deleteObject(objectNo);
	}
}


void MainWindow::onObjectDragged(int objectNo, QPoint pos) { 
	if (_ui.actionSelect->isChecked()) {
		_mapController->moveObject(objectNo, pos);
	}
}


void MainWindow::onObjectEditMapClickRequested(const QString &label) {
	_ui.statusbar->showMessage(QString("click on map to set %1").arg(label));
	_objectEditMapClickRequested = true;
}


void MainWindow::onNewTriggered() {
	if (not _changed or askSaveChanges()) {
		_mapController->clear();
		_path.clear();
		_changed = false;
	}
}


void MainWindow::onOpenTriggered() {
	if (not _changed or askSaveChanges()) {
		QSettings settings;
		QString directory = settings.value(SETTINGS_MAP_DIRECTORY, QDir::homePath()).toString();		
		QFileDialog dialog(this, "Open Map...", directory);
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		dialog.setFileMode(QFileDialog::ExistingFile);
		dialog.setNameFilters({"PETSCII Robot Maps (*.petmap)", "All Files (*)"});
		int result = dialog.exec();
		if (result == QFileDialog::Accepted) {
			if (dialog.selectedFiles().size() > 0) {
				_path = dialog.selectedFiles().at(0);
				settings.setValue(SETTINGS_MAP_DIRECTORY, dialog.directory().canonicalPath());
				QString error = _mapController->load(_path);
				if (error.isNull()) {
					settings.setValue(SETTINGS_MAP_PATH, _path);
				} else {
					QMessageBox::critical(this, "Error Opening Map",
					                      QString("Cannot open map: %1").arg(error));
				}
				_changed = false;
			}
		}
	}
}


void MainWindow::onSaveTriggered() {
	save();
}


void MainWindow::onSaveAsTriggered() {
	saveAs();
}


void MainWindow::onCopyAreaTriggered() {
	copyMap(true, _ui.actionShowObjects->isChecked());
}


void MainWindow::onCopyObjectsTriggered() {
	copyMap(false, _ui.actionShowObjects->isChecked());
}


void MainWindow::onCutAreaTriggered() {
	copyMap(true, _ui.actionShowObjects->isChecked(), true);
}


void MainWindow::onCutObjectsTriggered() {
	copyMap(false, _ui.actionShowObjects->isChecked(), true);
}


void MainWindow::onPasteTriggered() {
	const QRect &rect = _ui.mapWidget->selectedArea();
	if (rect.isNull()) {
		QMessageBox::warning(this, "Cannot Paste",
		                     "Cannot paste, because no destination has been chosen. Please select "
		                     "a target area first.");
		return;
	}
	
	if (_clipboardTilesValid) {
		auto it = _clipboardTiles.begin();
		for (int y = 0; y < _clipboardSize.height(); ++y) {
			for (int x = 0; x < _clipboardSize.width(); ++x) {
				Q_ASSERT(it != _clipboardTiles.end());
				_mapController->setTile(rect.topLeft() + QPoint(x, y), *it++);
			}
		}
	}
	bool addedAllObjects = true;
	for (auto it = _clipboardObjects.begin(); it != _clipboardObjects.end(); ++it) {
		MapObject object = *it;
		if (object.kind() == MapObject::Kind::Invalid) { continue; }
		int objectId = _mapController->map()->nextAvailableObjectId(object.kind());
		if (objectId != OBJECT_NONE) {
			object.x += rect.left();
			object.y += rect.top();
			_mapController->setObject(objectId, object);
		} else {
			addedAllObjects = false;
		}
	}
	
	if (not addedAllObjects) {
		QMessageBox::information(this, "Not All Objects Were Pasted",
		                         "Not all objects were pasted because not enough unused object "
		                         "slots were available");
	}
}


void MainWindow::onFillTriggered() {
	QRect rect = _ui.mapWidget->selectedArea();
	if (rect.isNull()) {
		rect = QRect(0, 0, MAP_WIDTH, MAP_HEIGHT);
	}
	
	for (int y = rect.top(); y <= rect.bottom(); ++y) {
		for (int x = rect.left(); x <= rect.right(); ++x) {
			_mapController->setTile({x, y}, _ui.tileWidget->selectedTile());
		}
	}
}


void MainWindow::onShowObjectsToggled(bool checked) {
	if (_ui.actionSelectArea->isChecked()) {
		_ui.actionCopyObjects->setEnabled(checked);
		_ui.actionCutObjects->setEnabled(checked);
	}
}


void MainWindow::onTilePressed(const QPoint &tile) {
	int newObjectId = -1;
	if (_objectEditMapClickRequested) {
		_ui.statusbar->clearMessage();
		_ui.objectEditor->mapClick(tile.x(), tile.y());
		_objectEditMapClickRequested = false;
	} else if (_ui.actionDrawTiles->isChecked()) {
		_mapController->beginTileDrawing();
		_mapController->setTile(tile, _ui.tileWidget->selectedTile());
	} else if (_ui.actionFloodFill->isChecked()) {
		_mapController->floodFill(tile, _ui.tileWidget->selectedTile());
	} else if (_ui.actionPlaceDoor->isChecked()) {
		newObjectId = placeObject(tile, OBJECT_DOOR, 0, 5);
	} else if (_ui.actionPlaceElevator->isChecked()) {
		newObjectId = placeObject(tile, OBJECT_ELEVATOR, 0, 2, 1, 1);
	} else if (_ui.actionPlaceHiddenItem->isChecked()) {
		newObjectId = placeObject(tile, OBJECT_TIME_BOMB, 10);
	} else if (_ui.actionPlaceKey->isChecked()) {
		newObjectId = placeObject(tile, OBJECT_KEY, 0, 0, 1, 1);
	} else if (_ui.actionPlacePlayer->isChecked()) {
		MapObject playerObject = _mapController->map()->object(PLAYER_OBJ);
		playerObject.x = tile.x();
		playerObject.y = tile.y();
		playerObject.unitType = UNITTYPE_PLAYER;
		playerObject.a = playerObject.b = playerObject.c = playerObject.d = 0;
		if (playerObject.health == 0) { playerObject.health = 12; }
		_mapController->setObject(PLAYER_OBJ, playerObject);
		newObjectId = PLAYER_OBJ;
	} else if (_ui.actionPlaceRobot->isChecked()) {
		newObjectId = placeObject(tile, ROBOT_HOVERBOT_LR, 0, 0, 0, 0, 10);
	} else if (_ui.actionPlaceTransporterPad->isChecked()) {
		newObjectId = placeObject(tile, OBJECT_TRANSPORTER, 1);
	} else if (_ui.actionPlaceTrashCompactor->isChecked()) {
		newObjectId = placeObject(tile, OBJECT_TRASH_COMPACTOR);
	} else if (_ui.actionPlaceWaterRaft->isChecked()) {
		newObjectId = placeObject(tile, OBJECT_WATER_RAFT, 0, 0,
		                          tile.x() - 1, tile.x() + 1);
	}
	
	if (newObjectId >= OBJECT_MIN) {
		_ui.actionSelect->trigger();
		_ui.objectEditor->loadObject(newObjectId);
	}
}


void MainWindow::onTileDragged(const QPoint &tile) {
	if (_ui.actionDrawTiles->isChecked()) {
		const uint8_t tileNo = _ui.tileWidget->selectedTile();
		_mapController->setTile(tile, tileNo);
	}
}


void MainWindow::onTileReleased() {
	if (_ui.actionDrawTiles->isChecked()) {
		_mapController->endTileDrawing();
	}
}


void MainWindow::onTileWidgetTileSelected(uint8_t tileNo) {
	Q_UNUSED(tileNo);
	if (not (_ui.actionDrawTiles->isChecked() or _ui.actionFloodFill->isChecked() or
	         _ui.actionSelectArea->isChecked())) {
		_ui.actionDrawTiles->trigger();
	}
	updateLabelStatusTile();
}


void MainWindow::onToolActionTriggered() {
	QAction *action = qobject_cast<QAction*>(sender());
	Q_ASSERT(action);
	activateTool(action);
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


bool MainWindow::askSaveChanges() {
	static const auto buttons = QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel;
	const auto button =  QMessageBox::question(this, "New Map", "There are unsaved changes. Do you "
	                                           "want to save the map?", buttons);
	if (button == QMessageBox::Save) {
		return save();
	} else if (button == QMessageBox::Discard) {
		return true;
	} else {
		return false;
	}
}


void MainWindow::autoLoadTileset() {
	QSettings settings;
	QString path;
	if (settings.contains(SETTINGS_TILESET_PATH)) {
		path = settings.value(SETTINGS_TILESET_PATH).toString();
	} else {
		QStringList directories = {
#ifdef Q_OS_UNIX
			QString("%1/../share/%2").arg(QApplication::applicationDirPath(),
			                              QFileInfo(QApplication::applicationFilePath()).baseName()),
		    QString("%1/../share").arg(QApplication::applicationDirPath()),
#endif
			QApplication::applicationDirPath(),
		};
		qDebug() << Q_FUNC_INFO << directories;
		for (const QString &dir : directories) {
			const QString maybePath = dir + "/tileset.pet";
			if (QFile::exists(maybePath)) {
				path = maybePath;
				break;
			}
		}
	}
	
	if (path.isEmpty()) {
		QMetaObject::invokeMethod(this, [=]() {
			QMessageBox::critical(this, "Error Loading Tileset",
			                      "Cannot find the tileset (file \"tileset.pet\"). Please load it "
			                      "manually.");
		}, Qt::QueuedConnection);
		return;
	}
	
	QString error = _tileset->load(path);
	if (not error.isNull()) {
		QMetaObject::invokeMethod(this, [=]() {
			QMessageBox::critical(this, "Error Loading Tileset",
								  "Could not load the tileset: " + error);
		}, Qt::QueuedConnection);
	}
}


void MainWindow::activateTool(QAction *const action) {
	for (QAction *toolAction : _toolActions) {
		toolAction->setChecked(action == toolAction);
	}
	
	const bool showTileSelected = action == _ui.actionDrawTiles or action == _ui.actionFloodFill or
	        action == _ui.actionSelectArea;
	
	_ui.mapWidget->setDragMode(action == _ui.actionSelectArea ? 
	                           MapWidget::DragMode::Area : MapWidget::DragMode::Object);
	_ui.mapWidget->setShowSelected(action == _ui.actionSelect);
	_ui.mapWidget->clearSelection();
	_ui.tileWidget->setShowSelected(showTileSelected);
	
	_ui.statusbar->showMessage(action->toolTip());
	updateLabelStatusTile();
	_labelStatusTile->setVisible(showTileSelected);
	
	if (not _ui.actionSelect->isChecked()) { _ui.objectEditor->loadObject(-1); }
	
	if (_objectEditMapClickRequested) {
		_ui.statusbar->clearMessage();
		_ui.objectEditor->mapClickCancelled();
		_objectEditMapClickRequested = false;
	}
	
	for (QAction *action : { _ui.actionCopyArea, _ui.actionCopyObjects, _ui.actionCutArea,
	                         _ui.actionCutObjects, _ui.actionPaste, _ui.actionFill }) {
		action->setEnabled(_ui.actionSelectArea->isChecked());
	}
}


void MainWindow::copyMap(bool copyTiles, bool copyObjects, bool clear) {
	const QRect &rect = _ui.mapWidget->selectedArea();
	if (rect.isNull()) { return; }
	
	_clipboardSize = rect.size();
	_clipboardTiles.clear();
	_clipboardTilesValid = false;
	_clipboardObjects.clear();
	
	if (copyTiles) {
		for (int y = rect.bottom(); y >= rect.top(); --y) {
			for (int x = rect.right(); x >= rect.left(); --x) {
				_clipboardTiles.push_front(_mapController->map()->tileNo({x, y}));
				if (clear) { _mapController->setTile({x, y}, 0); }
			}
		}
		_clipboardTilesValid = true;
	}
	if (copyObjects) {
		for (int objectNo = OBJECT_MAX; objectNo >= OBJECT_MIN; --objectNo) {
			MapObject object = _mapController->map()->object(objectNo);
			if (rect.contains(object.pos())) {
				object.x -= rect.left();
				object.y -= rect.top();
				_clipboardObjects.push_front(object);
				if (clear) { _mapController->deleteObject(objectNo); }
			}
		}
	}
}


bool MainWindow::doSave(const QString &path) {
	QFile file(path);
	if (not file.open(QFile::WriteOnly)) {
		QMessageBox::critical(this, "Cannot Save", QString("Cannot open \"%1\" for writing: %2")
		                      .arg(path, file.errorString()));
		return false;
	}
	
	qint64 bytesWritten = file.write(_mapController->map()->data());
	if (bytesWritten == -1) {
		QMessageBox::critical(this, "Cannot Save", QString("Cannot write to \"%1\": %2")
		                      .arg(path, file.errorString()));
		return false;
	} else if (bytesWritten != MAP_BYTES) {
		QMessageBox::critical(this, "Cannot Save", QString("Cannot write to \"%1\": short "
		                      "write, only %2 out of %3 bytes were written")
		                      .arg(path).arg(bytesWritten).arg(MAP_BYTES));
		return false;
	}
	
	file.close();
	_changed = false;
	_ui.statusbar->showMessage(QString("Saved to %1").arg(path), 5000);
	return true;
}


int MainWindow::placeObject(const QPoint &position, int unitType, int a, int b, int c, int d, int health) {
	const MapObject::Kind kind = MapObject::kind(unitType);
	const int objectNo = _mapController->map()->nextAvailableObjectId(kind);
	if (objectNo >= OBJECT_MIN) {
		MapObject object(unitType);
		object.x = position.x();
		object.y = position.y();
		object.a = a;
		object.b = b;
		object.c = c;
		object.d = d;
		object.health = health;
		_mapController->setObject(objectNo, object);
	} else {
		const QString &type = MapObject::toString(kind);
		const QString &category = MapObject::category(kind);
		
		QMessageBox::warning(this, QString("Cannot Add %1%2").arg(type.left(1).toUpper(), type.mid(1)),
		                     QString("Cannot add %1: the map already contains the maximum number "
		                             "of %2").arg(type).arg(category));
	}
	return objectNo;
}


void MainWindow::placeRobot(int x, int y) {
	const int objectNo = _mapController->map()->nextAvailableObjectId(MapObject::Kind::Robot);
	if (objectNo == -1) {
		QMessageBox::warning(this, "Cannot add robot",
		                     "Cannot add robot: the maximum number of robots has already been "
		                     "reached");
		return;
	}
	MapObject object(ROBOT_HOVERBOT_LR);
	object.x = x;
	object.y = y;
	object.health = 10;
	_mapController->setObject(objectNo, object);
}


bool MainWindow::save() {
	if (_path.isNull()) {
		return saveAs();
	}
	
	return doSave(_path);
}


bool MainWindow::saveAs() {
	QSettings settings;
	QFileDialog dialog(this);
	
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDirectory(settings.value(SETTINGS_MAP_DIRECTORY).toString());
	dialog.setNameFilters({"PETSCII Robot Maps (*.petmap)", "All Files (*)"});
	if (dialog.exec() == QFileDialog::Accepted and dialog.selectedFiles().count() > 0) {
		settings.setValue(SETTINGS_MAP_DIRECTORY, dialog.directory().canonicalPath());
		const QString path = dialog.selectedFiles().at(0);
		if (doSave(path)) {
			_path = path;
			settings.setValue(SETTINGS_MAP_PATH, path);
			return true;
		}
	}
	return false;
}


void MainWindow::updateLabelStatusTile() {
	_labelStatusTile->setText(QString("Selected Tile: 0x%1")
	                          .arg(_ui.tileWidget->selectedTile(), 2, 16, QChar('0')));
}


void MainWindow::updateMapCountLabels() {
	static constexpr int maxHiddenItemCount = HIDDEN_OBJECT_MAX - HIDDEN_OBJECT_MIN + 1;
	static constexpr int maxMapFeatureCount = MAP_FEATURE_MAX - MAP_FEATURE_MIN + 1;
	static constexpr int maxRobotCount = ROBOT_MAX - ROBOT_MIN + 1;
	
	const Map &map = *_mapController->map();
	
	_labelRobotCount->setText(QString("Robots: %1/%2").arg(map.robotCount())
	                          .arg(maxRobotCount));
	_labelMapFeatureCount->setText(QString("Map features: %1/%2").arg(map.mapFeatureCount())
	                               .arg(maxMapFeatureCount));
	_labelHiddenObjectsCount->setText(QString("Hidden objects: %1/%2").arg(map.hiddenItemCount())
	                                  .arg(maxHiddenItemCount));
}
