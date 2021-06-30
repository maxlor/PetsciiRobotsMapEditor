#ifndef OBJECTEDITWIDGET_H
#define OBJECTEDITWIDGET_H

#include "ui_objecteditwidget.h"
#include "forward_list"

class Map;
class MapController;


class ObjectEditWidget : public QGroupBox {
	Q_OBJECT
public:
	explicit ObjectEditWidget(QWidget *parent = nullptr);
	
	void loadObject(int objectNo);
	void setMapController(MapController *mapController);
	
public slots:
	void mapClick(int x, int y);
	void mapClickCancelled();
	
signals:
	void mapClickRequested(const QString &label);
	
private slots:
	void onObjectsChanged();
	void onCoordinateMapClickRequested(const QString &label);
	void store();
	
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
	const Map *map() const;
	
	Ui::ObjectEditWidget _ui;
	MapController *_mapController = nullptr;
	int _objectNo = -1;
	CoordinateWidget *_mapClickRequester = nullptr;
};

#endif // OBJECTEDITWIDGET_H
