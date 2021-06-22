#ifndef OBJECTEDITWIDGET_H
#define OBJECTEDITWIDGET_H

#include "ui_objecteditwidget.h"

class Map;


class ObjectEditWidget : public QGroupBox {
	Q_OBJECT
public:
	explicit ObjectEditWidget(QWidget *parent = nullptr);
	
	void loadObject(int objectNo);
	void setMap(Map *map);
	
private:
	void loadDoor(int objectNo);
	void loadElevator(int objectNo);
	void loadKey(int objectNo);
	void loadPlayer();
	void loadRobot(int objectNo);
	void loadTrashCompactor(int objectNo);
	void loadTransporterPad(int objectNo);
	void loadWaterRaft(int objectNo);
	void loadWeapon(int objectNo);
	
	Ui::ObjectEditWidget _ui;
	Map *_map = nullptr;
	int _objectNo = -1;
};

#endif // OBJECTEDITWIDGET_H
