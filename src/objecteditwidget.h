#ifndef OBJECTEDITWIDGET_H
#define OBJECTEDITWIDGET_H

#include "ui_objecteditwidget.h"
#include "mapobject.h"
#include "forward_list"

class MapController;


class ObjectEditWidget : public QGroupBox {
	Q_OBJECT
public:
	explicit ObjectEditWidget(QWidget *parent = nullptr);
	
	void loadObject(MapObject::id_t objectId);
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
	void loadDoor(const MapObject &object);
	void loadElevator(const MapObject &object);
	void loadKey(const MapObject &object);
	void loadPlayer(const MapObject &object);
	void loadRobot(const MapObject &object);
	void loadTrashCompactor(const MapObject &object);
	void loadTransporterPad(const MapObject &object);
	void loadWaterRaft(const MapObject &object);
	void loadWeapon(const MapObject &object);
	
	Ui::ObjectEditWidget _ui;
	MapController *_mapController = nullptr;
	MapObject::id_t _objectId = MapObject::IdNone;
	CoordinateWidget *_mapClickRequester = nullptr;
};

#endif // OBJECTEDITWIDGET_H
