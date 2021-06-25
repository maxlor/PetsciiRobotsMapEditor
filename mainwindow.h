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
	
protected:
	void keyPressEvent(QKeyEvent *event) override;
	
private slots:
	void onAbout();
	void onActionDeleteObjectTriggered();
	void onActionSelectTriggered();
	void onActionDrawTilesTriggered();
	void onActionPlaceTriggered();
	void onLoadTileset();
	void onMapObjectChanged();
	void onMouseOverTile(int x, int y);
	void onObjectClicked(int objectNo);
	void onObjectEditMapClickRequested(const QString &label);
	void onTileClicked(int x, int y);
	void onTileDragged(int x, int y);
	void onTileWidgetTileSelected(int tileNo);
	void onQuit();
	void onViewFilterChanged();
	
private:
	void autoLoadTileset();
	void activateTool(QAction *const action);
	void placeObject(int x, int y, int unitType, int a = 0, int b = 0, int c = 0, int d = 0, int health = 0);
	void placeRobot(int x, int y);
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
	int _currentObject = -1;
	bool _objectEditMapClickRequested = false;
	IconFactory _iconFactory;
};
#endif // MAINWINDOW_H
