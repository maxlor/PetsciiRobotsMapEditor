#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
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
	void onLoadTileset();
	void onMapObjectChanged();
	void onMapTilesChanged();
	void onMouseOverTile(const QPoint &tile);
	void onObjectClicked(int objectNo);
	void onObjectDragged(int objectNo, QPoint pos);
	void onObjectEditMapClickRequested(const QString &label);
	void onNewTriggered();
	void onOpenTriggered();
	void onSaveTriggered();
	void onSaveAsTriggered();
	void onCopyAreaTriggered();
	void onCopyObjectsTriggered();
	void onCutAreaTriggered();
	void onCutObjectsTriggered();
	void onPasteTriggered();
	void onFillTriggered();
	void onShowObjectsToggled(bool checked);
	
	void onTilePressed(const QPoint &tile);
	void onTileDragged(const QPoint &tile);
	void onTileReleased();
	void onTileWidgetTileSelected(uint8_t tileNo);
	void onToolActionTriggered();
	void onQuit();
	void onViewFilterChanged();
	
private:
	bool askSaveChanges();
	void autoLoadTileset();
	void activateTool(QAction *const action);
	void copyMap(bool copyTiles, bool copyObjects, bool clear=false);
	bool doSave(const QString &path);
	int placeObject(const QPoint &position, int unitType, int a = 0, int b = 0, int c = 0, int d = 0, int health = 0);
	void placeRobot(int x, int y);
	bool save();
	bool saveAs();
	void updateLabelStatusTile();
	void updateMapCountLabels();
	
	Ui::MainWindow _ui;
	QLabel *_labelHiddenObjectsCount;
	QLabel *_labelMapFeatureCount;
	QLabel *_labelRobotCount;
	QLabel *_labelStatusCoords;
	QLabel *_labelStatusTile;
	Tileset *_tileset = nullptr;
	std::forward_list<QAction*> _viewFilterActions;
	std::forward_list<QAction*> _toolActions;
	int _currentObject = -1;
	bool _objectEditMapClickRequested = false;
	IconFactory _iconFactory;
	bool _changed = false;
	QString _path;
	
	QSize _clipboardSize;
	std::forward_list<uint8_t> _clipboardTiles;
	bool _clipboardTilesValid;
	std::forward_list<MapObject> _clipboardObjects;
	
	MapController *_mapController;
};
#endif // MAINWINDOW_H
