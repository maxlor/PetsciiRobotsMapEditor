#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QLabel>
#include <QMainWindow>
#include <QSignalMapper>
#include <forward_list>
#include "iconfactory.h"

class Map;
class Tileset;


class MainWindow : public QMainWindow {
	Q_OBJECT
	
public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	
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
	void onTileClicked(const QPoint &tile);
	void onTileDragged(const QPoint &tile);
	void onTileWidgetTileSelected(int tileNo);
	void onToolActionTriggered();
	void onQuit();
	void onViewFilterChanged();
	
private:
	bool askSaveChanges();
	void autoLoadTileset();
	void activateTool(QAction *const action);
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
	Map *_map = nullptr;
	Tileset *_tileset = nullptr;
	std::forward_list<QAction*> _viewFilterActions;
	std::forward_list<QAction*> _toolActions;
	int _currentObject = -1;
	bool _objectEditMapClickRequested = false;
	IconFactory _iconFactory;
	bool _changed = false;
	QString _path;
};
#endif // MAINWINDOW_H
