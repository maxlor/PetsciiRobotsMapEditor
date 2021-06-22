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
	
private slots:
	void onAbout();
	void onMouseOverTile(int x, int y);
	void onQuit();
	void onViewFilterChanged();
	
private:
	Ui::MainWindow _ui;
	QLabel *_labelStatusCoords;
	Map *_map = nullptr;
	Tileset *_tileset = nullptr;
	std::forward_list<QAction*> _viewFilterActions;
	int _currentObject = -1;
};
#endif // MAINWINDOW_H
