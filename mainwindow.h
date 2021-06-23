#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QLabel>
#include <QMainWindow>
#include <forward_list>

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
	void onActionSelectTriggered();
	void onActionDrawTilesTriggered();
	void onLoadTileset();
	void onMouseOverTile(int x, int y);
	void onObjectClicked(int objectNo);
	void onTileClicked(int x, int y);
	void onTileWidgetTileSelected(int tileNo);
	void onQuit();
	void onViewFilterChanged();
	
private:
	void autoLoadTileset();
	void activateTool(QAction *const action);
	void updateLabelStatusTile();
	
	Ui::MainWindow _ui;
	QLabel *_labelStatusCoords;
	QLabel *_labelStatusTile;
	Map *_map = nullptr;
	Tileset *_tileset = nullptr;
	std::forward_list<QAction*> _viewFilterActions;
	int _currentObject = -1;
};
#endif // MAINWINDOW_H
