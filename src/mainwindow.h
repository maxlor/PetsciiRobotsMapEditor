#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QDialog>
#include <QLabel>
#include <QMainWindow>
#include <QSignalMapper>
#include <QSize>
#include <forward_list>
#include "iconfactory.h"
#include "mapcontroller.h"
#include "mapobject.h"

class Map;
class Tileset;


class MainWindow : public QMainWindow {
	Q_OBJECT
	
public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	
protected:
	void closeEvent(QCloseEvent *event) override;
	
private slots:
	void onAbout();
	void onLoadPetTileset();
	void onLoadTilesetFromFile();
	void onMouseOverTile(const QPoint &tile);
	void onObjectClicked(MapObject::id_t objectId);
	void onObjectDragged(MapObject::id_t objectId, QPoint pos);
	void onObjectEditMapClickRequested(const QString &label);
	
	void onNewTriggered();
	void onOpenTriggered();
	void onSaveTriggered();
	void onSaveAsTriggered();
	
	void onSelectAll();
	void onCopyAreaTriggered();
	void onCopyObjectsTriggered();
	void onCutAreaTriggered();
	void onCutObjectsTriggered();
	void onPasteTriggered();
	void onFillTriggered();
	void onRandomizeDirt();
	void onRandomizeGrass();
	
	void onShowObjectsToggled(bool checked);
	
	void onTileCopied(uint8_t tileNo);
	void onTilePressed(const QPoint &tile);
	void onTileDragged(const QPoint &tile);
	void onReleased();
	void onTileWidgetTileSelected(uint8_t tileNo);
	void onToolActionTriggered();
	void onQuit();
	void onViewFilterChanged(bool checked);
	
	void showHowToUse();
	void validateMap();
	void updateMapCountLabels();
	
private:
	bool askSaveChanges();
	
	void autoLoadTileset();
	QString pickTileset();
	QString tilesetPetPath() const;
	
	void activateTool(QAction *const action);
	void copyMap(bool copyTiles, bool copyObjects, bool clear=false);
	bool doSave(const QString &path);
	void placeObject(MapObject::UnitType unitType, const QPoint &position);
	bool save();
	bool saveAs();
	void updateLabelStatusTile();
	QRect workRect() const;
	
	Ui::MainWindow _ui;
	QDialog *_howToUseDialog = nullptr;
	QLabel *_labelHiddenObjectsCount;
	QLabel *_labelMapFeatureCount;
	QLabel *_labelRobotCount;
	QLabel *_labelStatusCoords;
	QLabel *_labelStatusTile;
	Tileset *_tileset = nullptr;
	std::forward_list<QAction*> _viewFilterActions;
	std::forward_list<QAction*> _toolActions;
	MapObject::id_t _currentObject = MapObject::IdNone;
	bool _objectEditMapClickRequested = false;
	IconFactory _iconFactory;
	
	QSize _clipboardSize;
	std::forward_list<uint8_t> _clipboardTiles;
	bool _clipboardTilesValid;
	std::forward_list<MapObject> _clipboardObjects;
	
	MapController *_mapController;
};
#endif // MAINWINDOW_H
