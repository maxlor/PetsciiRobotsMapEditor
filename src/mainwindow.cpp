#include "mainwindow.h"
#include <QApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QStringList>
#include <QTextBrowser>
#include <QVBoxLayout>
#include "iconfactory.h"
#include "map.h"
#include "mapcheck.h"
#include "multisignalblocker.h"
#include "tileset.h"
#include "util.h"
#include "validationdialog.h"


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
	
	QMenu *loadTilesetMenu = new QMenu("Load Tileset", _ui.menuFile);
	_ui.menuFile->insertMenu(_ui.actionQuit, loadTilesetMenu);
	_ui.menuFile->insertSeparator(_ui.actionQuit);
	loadTilesetMenu->addAction(_ui.actionLoadPetTileset);
	loadTilesetMenu->addAction(_ui.actionLoadTilesetFromFile);
	QMenu *randomizeMenu = _ui.menuEdit->addMenu("Randomize");
	randomizeMenu->addAction(_ui.actionRandomizeDirt);
	randomizeMenu->addAction(_ui.actionRandomizeGrass);
	
	QFontMetrics fm(QApplication::font());
	_labelHiddenObjectsCount = new QLabel(this);
	_labelMapFeatureCount = new QLabel(this);
	_labelRobotCount = new QLabel(this);
	_labelStatusCoords = new QLabel(this);
	_labelStatusCoords->setMinimumWidth(fm.width("Map Tile: 000, 00"));
	_labelStatusCoords->setAlignment(Qt::AlignLeading | Qt::AlignBaseline);
	_labelStatusTile = new QLabel(this);
	_ui.statusbar->addPermanentWidget(_labelRobotCount);
	_ui.statusbar->addPermanentWidget(_labelMapFeatureCount);
	_ui.statusbar->addPermanentWidget(_labelHiddenObjectsCount);
	_ui.statusbar->addPermanentWidget(_labelStatusCoords);
	_ui.statusbar->addPermanentWidget(_labelStatusTile);
	
	_ui.actionHighlightWalkable->setData(Tile::Walkable);
	_ui.actionHighlightHoverable->setData(Tile::Hoverable);
	_ui.actionHighlightMoveable->setData(Tile::Movable);
	_ui.actionHighlightDestructible->setData(Tile::Destructible);
	_ui.actionHighlightShootThrough->setData(Tile::ShootThrough);
	_ui.actionHighlightPushOnto->setData(Tile::PushOnto);
	_ui.actionHighlightSearchable->setData(Tile::Searchable);
	
	_tileset = new Tileset();
	autoLoadTileset();
	_ui.mapWidget->setTileset(_tileset);
	_ui.tileWidget->setTileset(_tileset);
	
	_mapController = new MapController(this);
	_ui.mapWidget->setMap(_mapController->map());
	_ui.objectEditor->setMapController(_mapController);
	updateMapCountLabels();
	_ui.menuEdit->insertSeparator(_ui.menuEdit->actions().at(0));
	_ui.menuEdit->insertAction(_ui.menuEdit->actions().at(0), _mapController->redoAction());
	_ui.menuEdit->insertAction(_ui.menuEdit->actions().at(0), _mapController->undoAction());
	_ui.toolBar->insertSeparator(_ui.toolBar->actions().at(0));
	_ui.toolBar->insertAction(_ui.toolBar->actions().at(0), _mapController->redoAction());
	_ui.toolBar->insertAction(_ui.toolBar->actions().at(0), _mapController->undoAction());
	
	_viewFilterActions = { _ui.actionHighlightWalkable, _ui.actionHighlightHoverable,
	                       _ui.actionHighlightMoveable, _ui.actionHighlightDestructible,
	                       _ui.actionHighlightShootThrough, _ui.actionHighlightPushOnto,
	                       _ui.actionHighlightSearchable };
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
	connect(_ui.actionLoadPetTileset, &QAction::triggered, this, &MainWindow::onLoadPetTileset);
	connect(_ui.actionLoadTilesetFromFile, &QAction::triggered, this, &MainWindow::onLoadTilesetFromFile);
	connect(_ui.actionQuit, &QAction::triggered, this, &MainWindow::onQuit);
	connect(_ui.actionSelectAll, &QAction::triggered, this, &MainWindow::onSelectAll);
	connect(_ui.actionCopyArea, &QAction::triggered, this, &MainWindow::onCopyAreaTriggered);
	connect(_ui.actionCopyObjects, &QAction::triggered, this, &MainWindow::onCopyObjectsTriggered);
	connect(_ui.actionCutArea, &QAction::triggered, this, &MainWindow::onCutAreaTriggered);
	connect(_ui.actionCutObjects, &QAction::triggered, this, &MainWindow::onCutObjectsTriggered);
	connect(_ui.actionPaste, &QAction::triggered, this, &MainWindow::onPasteTriggered);
	connect(_ui.actionFill, &QAction::triggered, this, &MainWindow::onFillTriggered);
	connect(_ui.actionRandomizeDirt, &QAction::triggered, this, &MainWindow::onRandomizeDirt);
	connect(_ui.actionRandomizeGrass, &QAction::triggered, this, &MainWindow::onRandomizeGrass);
	
	for (QAction *action : _viewFilterActions) {
		connect(action, &QAction::triggered, this, &MainWindow::onViewFilterChanged);
	}
	connect(_ui.actionValidateMap, &QAction::triggered, this, &MainWindow::validateMap);
	connect(_ui.actionZoomIn, &QAction::triggered, _ui.tileWidget, &TileWidget::zoomIn);
	connect(_ui.actionZoomOut, &QAction::triggered, _ui.tileWidget, &TileWidget::zoomOut);
	connect(_ui.actionZoomIn, &QAction::triggered, _ui.mapWidget, &MapWidget::zoomIn);
	connect(_ui.actionZoomOut, &QAction::triggered, _ui.mapWidget, &MapWidget::zoomOut);
	connect(_ui.actionShowObjects, &QAction::toggled, this, &MainWindow::onShowObjectsToggled);
	connect(_ui.actionShowObjects, &QAction::toggled, _ui.mapWidget, &MapWidget::setObjectsVisible);
	connect(_ui.actionShowGrid, &QAction::toggled, _ui.mapWidget, &MapWidget::setShowGridLines);
	
	for (QAction *action : _toolActions) {
		connect(action, &QAction::triggered, this, &MainWindow::onToolActionTriggered);
	}
	
	connect(_ui.actionHowToUse, &QAction::triggered, this, &MainWindow::showHowToUse);
	connect(_ui.actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
	connect(_ui.actionClickEveryTile, &QAction::triggered, _ui.mapWidget, &MapWidget::clickEveryTile);
	
	connect(_ui.mapWidget, &MapWidget::mouseOverTile, this, &MainWindow::onMouseOverTile);
	connect(_ui.mapWidget, &MapWidget::objectClicked, this, &MainWindow::onObjectClicked);
	connect(_ui.mapWidget, &MapWidget::objectDragged, this, &MainWindow::onObjectDragged);
	connect(_ui.mapWidget, &MapWidget::tileCopied, this, &MainWindow::onTileCopied);
	connect(_ui.mapWidget, &MapWidget::tilePressed, this, &MainWindow::onTilePressed);
	connect(_ui.mapWidget, &MapWidget::tileDragged, this, &MainWindow::onTileDragged);
	connect(_ui.mapWidget, &MapWidget::released, this, &MainWindow::onReleased);
	connect(_ui.tileWidget, &TileWidget::tileSelected, this, &MainWindow::onTileWidgetTileSelected);
	connect(_ui.objectEditor, &ObjectEditWidget::mapClickRequested, this, &MainWindow::onObjectEditMapClickRequested);
	
	connect(_mapController->map(), &Map::objectsChanged, this, &MainWindow::updateMapCountLabels);
	
	const QSettings settings;
	const QRect geometry = settings.value(SETTINGS_WINDOW_GEOMETRY).toRect();
	if (geometry.isValid()) { setGeometry(geometry); }
	const bool maximized = settings.value(SETTINGS_WINDOW_MAXIMIZED).toBool();
	if (maximized) { showMaximized(); }
	
	// reload map that was last open, fail silently.
	const QString path = settings.value(SETTINGS_MAP_PATH).toString();
	if (not path.isEmpty()) {
		_mapController->load(path);
	}
	
	activateTool(_ui.actionSelect);
}


MainWindow::~MainWindow() {
	QSettings settings;
	settings.setValue(SETTINGS_WINDOW_GEOMETRY, geometry());
	settings.setValue(SETTINGS_WINDOW_MAXIMIZED, isMaximized());
}


void MainWindow::closeEvent(QCloseEvent *event) {
	if (not _mapController->map()->isModified() or askSaveChanges()) {
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


void MainWindow::onLoadPetTileset() {
	const QString path = tilesetPetPath();
	
	if (path.isEmpty()) {
		QMessageBox::critical(this, "Error Loading Tileset",
		                      "Cannot find the bundled tileset (tileset.pet).");
		return;
	}
	
	QString error = _tileset->load(path);
	if (error.isNull()) {
		QSettings().remove(SETTINGS_TILESET_PATH);
	} else {
		QMessageBox::critical(this, "Error Loading Tileset",
		                      "Could not load the tileset: " + error);
	}
}


void MainWindow::onLoadTilesetFromFile() {
	QSettings settings;
	QString path = pickTileset();
	if (path.isNull()) { return; }
	QString error = _tileset->load(path);
	if (error.isNull()) {
		settings.setValue(SETTINGS_TILESET_PATH, path);
	} else {
		QMessageBox::critical(this, "Error Loading Tileset",
		                      QString("Cannot load tileset: %1").arg(error));
	}
}


void MainWindow::onMouseOverTile(const QPoint &tile) {
	_labelStatusCoords->setText(QString("Map Tile: %1, %2").arg(tile.x()).arg(tile.y()));
}


void MainWindow::onObjectClicked(MapObject::id_t objectId) {
	if (_ui.actionSelect->isChecked() and not _objectEditMapClickRequested) {
		_ui.objectEditor->loadObject(objectId);
		_ui.mapWidget->markObject(objectId);
	} else if (_ui.actionDeleteObject->isChecked() and objectId != MapObject::IdNone) {
		_mapController->deleteObject(objectId);
	}
}


void MainWindow::onObjectDragged(MapObject::id_t objectId, QPoint pos) {
	if (_ui.actionSelect->isChecked()) {
		_mapController->moveObject(objectId, pos);
	}
}


void MainWindow::onObjectEditMapClickRequested(const QString &label) {
	_ui.statusbar->showMessage(QString("click on map to set %1").arg(label));
	_objectEditMapClickRequested = true;
}


void MainWindow::onNewTriggered() {
	if (not _mapController->map()->isModified() or askSaveChanges()) {
		_mapController->clear();
	}
}


void MainWindow::onOpenTriggered() {
	if (not _mapController->map()->isModified() or askSaveChanges()) {
		QSettings settings;
		QString directory = settings.value(SETTINGS_MAP_DIRECTORY, QDir::homePath()).toString();		
		QFileDialog dialog(this, "Open Map...", directory);
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		dialog.setFileMode(QFileDialog::ExistingFile);
		dialog.setNameFilters({"PETSCII Robot Maps (*.petmap)", "All Files (*)"});
		int result = dialog.exec();
		if (result == QFileDialog::Accepted) {
			if (dialog.selectedFiles().size() > 0) {
				QString path = dialog.selectedFiles().at(0);
				settings.setValue(SETTINGS_MAP_DIRECTORY, dialog.directory().canonicalPath());
				QString error = _mapController->load(path);
				if (error.isNull()) {
					settings.setValue(SETTINGS_MAP_PATH, path);
				} else {
					QMessageBox::critical(this, "Error Opening Map",
					                      QString("Cannot open map: %1").arg(error));
				}
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


void MainWindow::onSelectAll() {
	activateTool(_ui.actionSelectArea);
	_ui.mapWidget->selectAll();
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
		if (object.group() == MapObject::Group::Invalid) { continue; }
		MapObject::id_t objectId = _mapController->map()->nextAvailableObjectId(object.group());
		if (objectId != MapObject::IdNone) {
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
	QRect rect = workRect();
	
	_mapController->beginUndoGroup();
	for (int y = rect.top(); y <= rect.bottom(); ++y) {
		for (int x = rect.left(); x <= rect.right(); ++x) {
			_mapController->setTile({x, y}, _ui.tileWidget->selectedTile());
		}
	}
	_mapController->endUndoGroup();
}


void MainWindow::onRandomizeDirt() {
	_mapController->randomizeDirt(workRect());
}


void MainWindow::onRandomizeGrass() {
	_mapController->randomizeGrass(workRect());
}


void MainWindow::onShowObjectsToggled(bool checked) {
	if (_ui.actionSelectArea->isChecked()) {
		_ui.actionCopyObjects->setEnabled(checked);
		_ui.actionCutObjects->setEnabled(checked);
	}
}


void MainWindow::onTileCopied(uint8_t tileNo) {
	if (_objectEditMapClickRequested) {
		_ui.statusbar->clearMessage();
		_ui.objectEditor->mapClickCancelled();
		_objectEditMapClickRequested = false;
	}
	
	if (not (_ui.actionDrawTiles->isChecked() or _ui.actionFloodFill->isChecked())) {
		activateTool(_ui.actionDrawTiles);
	}
	_ui.tileWidget->selectTile(tileNo);
}


void MainWindow::onTilePressed(const QPoint &tile) {
	if (_objectEditMapClickRequested) {
		_ui.statusbar->clearMessage();
		_ui.objectEditor->mapClick(tile.x(), tile.y());
		_objectEditMapClickRequested = false;
	} else if (_ui.actionDrawTiles->isChecked()) {
		_mapController->beginUndoGroup();
		_mapController->setTile(tile, _ui.tileWidget->selectedTile());
	} else if (_ui.actionFloodFill->isChecked()) {
		_mapController->floodFill(tile, _ui.tileWidget->selectedTile());
	} else if (_ui.actionPlaceDoor->isChecked()) {
		placeObject(MapObject::UnitType::Door, tile);
	} else if (_ui.actionPlaceElevator->isChecked()) {
		placeObject(MapObject::UnitType::Elevator, tile);
	} else if (_ui.actionPlaceHiddenItem->isChecked()) {
		placeObject(MapObject::UnitType::TimeBomb, tile);
	} else if (_ui.actionPlaceKey->isChecked()) {
		placeObject(MapObject::UnitType::Key, tile);
	} else if (_ui.actionPlacePlayer->isChecked()) {
		placeObject(MapObject::UnitType::Player, tile);
	} else if (_ui.actionPlaceRobot->isChecked()) {
		placeObject(MapObject::UnitType::HoverbotLR, tile);
	} else if (_ui.actionPlaceTransporterPad->isChecked()) {
		placeObject(MapObject::UnitType::TransporterPad, tile);
	} else if (_ui.actionPlaceTrashCompactor->isChecked()) {
		placeObject(MapObject::UnitType::TrashCompactor, tile);
	} else if (_ui.actionPlaceWaterRaft->isChecked()) {
		placeObject(MapObject::UnitType::WaterRaft, tile);
	}
}


void MainWindow::onTileDragged(const QPoint &tile) {
	if (_ui.actionDrawTiles->isChecked()) {
		const uint8_t tileNo = _ui.tileWidget->selectedTile();
		_mapController->setTile(tile, tileNo);
	}
}


void MainWindow::onReleased() {
	if (_ui.actionDrawTiles->isChecked()) {
		_mapController->endUndoGroup();
	}
	_mapController->incrementMergeCounter();
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


void MainWindow::onViewFilterChanged(bool checked) {
	QAction *senderAction = qobject_cast<QAction*>(sender());
	Q_ASSERT(senderAction);
	for (QAction *action : _viewFilterActions) {
		if (action != senderAction) {
			action->setChecked(false);
		}
	}
	
	const Tile::Attribute highlightAttribute = checked ?
	            static_cast<Tile::Attribute>(senderAction->data().toUInt()) : Tile::None;
	_ui.mapWidget->setHighlightAttribute(highlightAttribute);
	_ui.tileWidget->setHighlightAttribute(highlightAttribute);
}


void MainWindow::showHowToUse() {
	if (_howToUseDialog == nullptr) {
		_howToUseDialog = new QDialog(this);
		_howToUseDialog->setWindowTitle("Browser");
		
		QVBoxLayout *layout = new QVBoxLayout();
		_howToUseDialog->setLayout(layout);
		
		QTextBrowser *browser = new QTextBrowser(_howToUseDialog);
		browser->setReadOnly(true);
		browser->setOpenExternalLinks(true);
		layout->addWidget(browser, 1);
		
		QFile file(":/help.html");
		if (file.open(QFile::ReadOnly)) {
			browser->setHtml(file.readAll());
		} else {
			browser->setPlainText("cannot load help: " + file.errorString());
		}
		file.close();
		_howToUseDialog->setWindowTitle(browser->documentTitle());
		
		QDialogButtonBox *buttons = new QDialogButtonBox(_howToUseDialog);
		buttons->addButton(QDialogButtonBox::Close);
		connect(buttons, &QDialogButtonBox::rejected, _howToUseDialog, &QDialog::hide);
		layout->addWidget(buttons);
		
		static const int width = 800;
		static const int height = 600;
		const QRect &parentGeom = geometry();
		int x = qMax(0, parentGeom.x() + (parentGeom.width() - width) / 2);
		int y = qMax(0, parentGeom.y() + (parentGeom.height() - height) / 2);
		_howToUseDialog->setGeometry(x, y, width, height);
	}
	
	_howToUseDialog->show();
}


void MainWindow::validateMap() {
	MapCheck mapCheck(*_mapController, *_tileset);
	if (mapCheck.problems().empty()) {
		QMessageBox::information(this, "No Problems Found", "No problems have been found.");
	} else {
		ValidationDialog dialog(mapCheck, this);
		connect(&dialog, &ValidationDialog::requestSelectObject, this, &MainWindow::onObjectClicked);
		if (dialog.exec() == QDialog::Accepted) {
			mapCheck.fixAll();
		}
	}
}


void MainWindow::updateMapCountLabels() {
	static constexpr int maxHiddenItemCount = MapObject::IdHiddenMax - MapObject::IdHiddenMin + 1;
	static constexpr int maxMapFeatureCount = MapObject::IdMapFeatureMax - MapObject::IdMapFeatureMin + 1;
	static constexpr int maxRobotCount = MapObject::IdRobotMax - MapObject::IdRobotMin + 1;
	
	const Map &map = *_mapController->map();
	
	_labelRobotCount->setText(QString("Robots: %1/%2").arg(map.robotCount())
	                          .arg(maxRobotCount));
	_labelMapFeatureCount->setText(QString("Map features: %1/%2").arg(map.mapFeatureCount())
	                               .arg(maxMapFeatureCount));
	_labelHiddenObjectsCount->setText(QString("Hidden objects: %1/%2").arg(map.hiddenItemCount())
	                                  .arg(maxHiddenItemCount));
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
		path = tilesetPetPath();
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


QString MainWindow::pickTileset() {
	QSettings settings;
	QString directory = settings.value(SETTINGS_TILESET_DIRECTORY, QDir::homePath()).toString();
	QFileDialog dialog(this, "Load Tileset...", directory);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilters({ "tileset.pet", "Any file (*)"});
	int result = dialog.exec();
	if (result == QFileDialog::Accepted) {
		if (dialog.selectedFiles().size() > 0) {
			settings.setValue(SETTINGS_TILESET_DIRECTORY, dialog.directory().canonicalPath());
			const QString path = dialog.selectedFiles().at(0);
			return path;
		}
	}
	return QString();
}


QString MainWindow::tilesetPetPath() const {
	QStringList directories = {
#ifdef Q_OS_UNIX
		QString("%1/../share/%2").arg(QApplication::applicationDirPath(),
		                              QFileInfo(QApplication::applicationFilePath()).baseName()),
	    QString("%1/../share").arg(QApplication::applicationDirPath()),
#endif
		QApplication::applicationDirPath(),
	};
	for (const QString &dir : directories) {
		const QString path = dir + "/tileset.pet";
		if (QFile::exists(path)) {
			return path;
		}
	}
	return QString();
}


void MainWindow::activateTool(QAction *const action) {
	for (QAction *toolAction : _toolActions) {
		toolAction->setChecked(action == toolAction);
	}
	
	const bool showTileSelected = action == _ui.actionDrawTiles or action == _ui.actionFloodFill or
	        action == _ui.actionSelectArea;
	
	_ui.mapWidget->setDragMode(action == _ui.actionSelectArea ? 
	                           MapWidget::DragMode::Area : MapWidget::DragMode::Single);
	_ui.mapWidget->setShowSelected(action == _ui.actionSelect);
	_ui.mapWidget->clearSelection();
	_ui.tileWidget->setShowSelected(showTileSelected);
	
	_ui.statusbar->showMessage(action->toolTip());
	updateLabelStatusTile();
	_labelStatusTile->setVisible(showTileSelected);
	
	if (not _ui.actionSelect->isChecked()) {
		_ui.objectEditor->loadObject(MapObject::IdNone);
		_ui.mapWidget->markObject(MapObject::IdNone);
	}
	
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
		for (MapObject::id_t objectId = MapObject::IdMax; objectId >= MapObject::IdMin; --objectId) {
			MapObject object = _mapController->map()->object(objectId);
			if (rect.contains(object.pos())) {
				object.x -= rect.left();
				object.y -= rect.top();
				_clipboardObjects.push_front(object);
				if (clear) { _mapController->deleteObject(objectId); }
			}
		}
	}
}


bool MainWindow::doSave(const QString &path) {
	QString error = _mapController->save(path);
	if (not error.isNull()) {
		QMessageBox::critical(this, "Cannot Save", "Saving failed: " + error);
		return false;
	}

	_ui.statusbar->showMessage(QString("Saved to %1").arg(path), 5000);
	return true;
}


void MainWindow::placeObject(MapObject::UnitType unitType, const QPoint &position) {
	QString error;
	MapObject object(unitType);
	object.x = position.x();
	object.y = position.y();
	if (object.unitType == MapObject::UnitType::WaterRaft) {
		object.b = qMax(0, position.x() - 1);
		object.c = qMin(_mapController->map()->width() - 1, position.x() + 1);
	}
	MapObject::id_t objectId = _mapController->newObject(object, &error);
	
	if (error.isNull()) {
		_ui.actionSelect->trigger();
		_ui.objectEditor->loadObject(objectId);
		_ui.mapWidget->markObject(objectId);
	} else {
		QString unitTypeS = MapObject::toString(unitType);
		QMessageBox::warning(this, "Cannot Place " + capitalize(unitTypeS),
		                     QString("Cannot place %1: %2").arg(unitTypeS, error));
	}
}


bool MainWindow::save() {
	const QString path = _mapController->map()->path();
	return path.isNull() ? saveAs() : doSave(path);
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


QRect MainWindow::workRect() const {
	QRect r = _ui.mapWidget->selectedArea();
	if (r.isNull()) {
		r = _mapController->map()->rect();
	}
	return r;
}
