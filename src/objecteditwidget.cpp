#include "objecteditwidget.h"
#include <QLoggingCategory>
#include <forward_list>
#include <unordered_map>
#include "constants.h"
#include "map.h"
#include "mapcontroller.h"
#include "multisignalblocker.h"

Q_LOGGING_CATEGORY(lcObjEd, "ObjEd");


ObjectEditWidget::ObjectEditWidget(QWidget *parent) : QGroupBox(parent){
	_ui.setupUi(this);
	
	for (const auto &pair : Map::unitTypes()) {
		switch (MapObject::kind(pair.first)) {
		case MapObject::Kind::Robot:
			_ui.comboRobotType->insertItem(_ui.comboRobotType->count(), pair.second, pair.first);
			break;
		case MapObject::Kind::HiddenObject:
			_ui.comboWeaponType->insertItem(_ui.comboWeaponType->count(), pair.second, pair.first);
			break;
		default:;
		}
	}
	
	for (int i : { 0, 1, 3, 4 }) {
		_ui.comboDoorState->setItemData(i, QColor(Qt::gray), Qt::TextColorRole);
		_ui.comboElevatorState->setItemData(i, QColor(Qt::gray), Qt::TextColorRole);
	}
	
	_ui.coordinatesDoor->setLabel("door position");
	_ui.coordinatesElevator->setLabel("elevator position");
	_ui.coordinatesKey->setLabel("key position");
	_ui.coordinatesPlayer->setLabel("player position");
	_ui.coordinatesRaft->setLabel("raft position");
	_ui.coordinatesRobot->setLabel("robot position");
	_ui.coordinatesTransporter->setLabel("transporter pad position");
	_ui.coordinatesTransporterDest->setLabel("transporter pad destination");
	_ui.coordinatesTrashCompactor->setLabel("trash compactor position");
	_ui.coordinatesWeapon->setLabel("item position");
	
	union {
		QSpinBox *spinBox;
		QComboBox *comboBox;
		QCheckBox *checkBox;
		QRadioButton *radioButton;
		CoordinateWidget *coordinateWidget;
	} u;
	for (QObject *stackedWidgetChild : _ui.stackedWidget->children()) {
		for (QObject *child : stackedWidgetChild->children()) {
			if ((u.spinBox = qobject_cast<QSpinBox*>(child))) {
				connect(u.spinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
				        this, &ObjectEditWidget::store);
			} else if ((u.comboBox = qobject_cast<QComboBox*>(child))) {
				connect(u.comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
				        this, &ObjectEditWidget::store);
			} else if ((u.checkBox = qobject_cast<QCheckBox*>(child))) {
				connect(u.checkBox, &QCheckBox::clicked, this, &ObjectEditWidget::store);
			} else if ((u.radioButton = qobject_cast<QRadioButton*>(child))) {
				connect(u.radioButton, &QCheckBox::clicked, this, &ObjectEditWidget::store);
			} else if ((u.coordinateWidget = qobject_cast<CoordinateWidget*>(child))) {
				connect(u.coordinateWidget, &CoordinateWidget::valueChanged,
				        this, &ObjectEditWidget::store);
				connect(u.coordinateWidget, &CoordinateWidget::mapClickRequested,
				        this, &ObjectEditWidget::onCoordinateMapClickRequested);
			} else if ((qobject_cast<QLabel*>(child))) {
			} else if ((qobject_cast<QLayout*>(child))) {
			} else {
				Q_ASSERT_X(false, Q_FUNC_INFO,
				           QString("not handling %1").arg(child->objectName()).toUtf8().constData());
			}
		}
	}
}


void ObjectEditWidget::loadObject(int objectNo) {
	static const std::unordered_map<MapObject::Kind, QString> groupNames {
		{ MapObject::Kind::Player, "Player" },
		{ MapObject::Kind::Robot, "Robot" },
		{ MapObject::Kind::TransporterPad, "Transporter Pad" },
		{ MapObject::Kind::Door, "Door" },
		{ MapObject::Kind::TrashCompactor, "Trash Compactor" },
		{ MapObject::Kind::Elevator, "Elevator" },
		{ MapObject::Kind::WaterRaft, "Water Raft" },
		{ MapObject::Kind::Key, "Key" },
		{ MapObject::Kind::HiddenObject, "Hidden Weapon / Item" }
	};
	_objectNo = -1;
	
	if (not map()) {
		qCWarning(lcObjEd, "can't edit object, map not set");
		setVisible(false);
		return;
	}
	
	if (objectNo == -1) {
		setVisible(false);
		return;
	}
	
	const MapObject &object = map()->object(objectNo);
	const MapObject::Kind kind = object.kind();
	try {
		setTitle(groupNames.at(kind));
	} catch (std::out_of_range) {
		qCWarning(lcObjEd, "can't edit object %d of unit type %d", objectNo, object.unitType);
		setVisible(false);
		return;
	}
	
	switch (kind) {
	case MapObject::Kind::Player: loadPlayer(); break;
	case MapObject::Kind::Door: loadDoor(objectNo); break;
	case MapObject::Kind::Elevator: loadElevator(objectNo); break;
	case MapObject::Kind::Key: loadKey(objectNo); break;
	case MapObject::Kind::HiddenObject: loadWeapon(objectNo); break;
	case MapObject::Kind::Robot: loadRobot(objectNo); break;
	case MapObject::Kind::TrashCompactor: loadTrashCompactor(objectNo); break;
	case MapObject::Kind::TransporterPad: loadTransporterPad(objectNo); break;
	case MapObject::Kind::WaterRaft: loadWaterRaft(objectNo); break;
	case MapObject::Kind::Invalid: setVisible(false); break;
	}
	
	_objectNo = objectNo;
	setVisible(true);
}


void ObjectEditWidget::setMapController(MapController *mapController) {
	if (_mapController) {
		_mapController->map()->disconnect(this);
	}
	_mapController = mapController;
	
	for (QObject *stackedWidgetChild : _ui.stackedWidget->children()) {
		for (QObject *child : stackedWidgetChild->children()) {
			CoordinateWidget *coordinateWidget = qobject_cast<CoordinateWidget*>(child);
			if (coordinateWidget) {
				coordinateWidget->setRect(_mapController->map()->rect());
			}
		}
	}
	
	connect(_mapController->map(), &Map::objectsChanged, this, &ObjectEditWidget::onObjectsChanged);
}


void ObjectEditWidget::mapClick(int x, int y) {
	if (_mapClickRequester) {
		_mapClickRequester->setXY(x, y);
		_mapClickRequester = nullptr;
	}
}


void ObjectEditWidget::mapClickCancelled() {
	_mapClickRequester = nullptr;
}


void ObjectEditWidget::onObjectsChanged() {
	if (_objectNo != OBJECT_NONE) {
		loadObject(_objectNo);
	}
}


void ObjectEditWidget::onCoordinateMapClickRequested(const QString &label) {
	_mapClickRequester = qobject_cast<CoordinateWidget*>(sender());
	if (_mapClickRequester) {
		emit mapClickRequested(label);
	}
}


void ObjectEditWidget::store() {
	MapObject object = map()->object(_objectNo);
	if (_ui.stackedWidget->currentWidget() == _ui.pagePlayer) {
		object.x = _ui.coordinatesPlayer->x();
		object.y = _ui.coordinatesPlayer->y();
		object.health = _ui.editPlayerHealth->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageRobot) {
		object.x = _ui.coordinatesRobot->x();
		object.y = _ui.coordinatesRobot->y();
		object.health = _ui.editRobotHealth->value();
		object.unitType = _ui.comboRobotType->currentData().toInt();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageWeapon) {
		object.x = _ui.coordinatesWeapon->x();
		object.y = _ui.coordinatesWeapon->y();
		object.unitType = _ui.comboWeaponType->currentData().toInt();
		object.a = _ui.editWeaponAmount->value();
		object.c = _ui.editWeaponContainerWidth->value();
		object.d = _ui.editWeaponContainerHeight->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageDoor) {
		object.x = _ui.coordinatesDoor->x();
		object.y = _ui.coordinatesDoor->y();
		object.a = _ui.comboDoorOrientation->currentIndex();
		object.b = _ui.comboDoorState->currentIndex();
		object.c = _ui.comboDoorLock->currentIndex();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageKey) {
		object.x = _ui.coordinatesKey->x();
		object.y = _ui.coordinatesKey->y();
		if (_ui.radioKeyHeart->isChecked()) { object.a = 1; }
		else if (_ui.radioKeyStar->isChecked()) { object.a = 2; }
		else { object.a = 0; }
		object.c = _ui.editKeyContainerWidth->value();
		object.d = _ui.editKeyContainerHeight->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageTrashCompactor) {
		object.x = _ui.coordinatesTrashCompactor->x();
		object.y = _ui.coordinatesTrashCompactor->y();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageElevator) {
		object.x = _ui.coordinatesElevator->x();
		object.y = _ui.coordinatesElevator->y();
		object.b = _ui.comboElevatorState->currentIndex();
		object.c = _ui.editElevatorFloor->value();
		object.d = _ui.editElevatorTotalFloors->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageRaft) {
		object.x = _ui.coordinatesRaft->x();
		object.y = _ui.coordinatesRaft->y();
		object.a = _ui.radioRaftDirectionRight->isChecked() ? 1 : 0;
		object.b = _ui.editRaftLeftTurnaround->value();
		object.c = _ui.editRaftRightTurnaround->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageTransporter) {
		object.x = _ui.coordinatesTransporter->x();
		object.y = _ui.coordinatesTransporter->y();
		object.a = _ui.checkTransporterDisable->isChecked() ? 1 : 0;
		object.b = _ui.radioTransporterCoordinates->isChecked() ? 1 : 0;
		object.c = _ui.coordinatesTransporterDest->x();
		object.d = _ui.coordinatesTransporterDest->y();
	} else {
		Q_ASSERT(false);
	}
	_mapController->setObject(_objectNo, object);
}


void ObjectEditWidget::loadDoor(int objectNo) {
	MultiSignalBlocker blocker = { _ui.coordinatesDoor, _ui.comboDoorOrientation,
	                               _ui.comboDoorState, _ui.comboDoorLock };
	const MapObject &object = map()->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageDoor);
	_ui.coordinatesDoor->setXY(object.x, object.y);
	_ui.comboDoorOrientation->setCurrentIndex(object.a ? 1 : 0);
	_ui.comboDoorState->setCurrentIndex(object.b < 6 ? object.b : 5);
	_ui.comboDoorLock->setCurrentIndex(object.c < 4 ? object.c : 0);
}


void ObjectEditWidget::loadElevator(int objectNo) {
	MultiSignalBlocker blocker = { _ui.coordinatesElevator, _ui.comboElevatorState,
	                               _ui.editElevatorFloor, _ui.editElevatorTotalFloors };
	const MapObject &object = map()->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageElevator);
	_ui.coordinatesElevator->setXY(object.x, object.y);
	_ui.comboElevatorState->setCurrentIndex(object.b < 6 ? object.b : 5);
	_ui.editElevatorFloor->setValue(object.c);
	_ui.editElevatorTotalFloors->setValue(object.d);
}


void ObjectEditWidget::loadKey(int objectNo) {
	MultiSignalBlocker blocker = { _ui.coordinatesKey, _ui.radioKeySpade, _ui.radioKeyHeart,
	                               _ui.radioKeyStar, _ui.editKeyContainerWidth,
	                               _ui.editKeyContainerHeight };
	const MapObject &object = map()->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageKey);
	_ui.coordinatesKey->setXY(object.x, object.y);
	_ui.radioKeySpade->setChecked(object.a == 0);
	_ui.radioKeyHeart->setChecked(object.a == 1);
	_ui.radioKeyStar->setChecked(object.a == 2);
	_ui.editKeyContainerWidth->setValue(object.c);
	_ui.editKeyContainerHeight->setValue(object.d);
}


void ObjectEditWidget::loadPlayer() {
	MultiSignalBlocker blocker = { _ui.coordinatesPlayer, _ui.editPlayerHealth };
	const MapObject &object = map()->object(PLAYER_OBJ);
	_ui.stackedWidget->setCurrentWidget(_ui.pagePlayer);
	_ui.coordinatesPlayer->setXY(object.x, object.y);
	_ui.editPlayerHealth->setValue(object.health);
}


void ObjectEditWidget::loadRobot(int objectNo) {
	MultiSignalBlocker blocker = { _ui.coordinatesRobot, _ui.editRobotHealth,
	                               _ui.comboRobotType };
	const MapObject &object = map()->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageRobot);
	_ui.coordinatesRobot->setXY(object.x, object.y);
	_ui.editRobotHealth->setValue(object.health);
	bool found = false;
	for (int i = 0; i < _ui.comboRobotType->count(); ++i) {
		if (_ui.comboRobotType->itemData(i).toInt() == object.unitType) {
			_ui.comboRobotType->setCurrentIndex(i);
			found = true;
			break;
		}
	}
	if (not found) {
		_ui.comboRobotType->setCurrentIndex(0);
		qCInfo(lcObjEd, "robot unit type %d not found, assuming %d (%s)", object.unitType,
		       _ui.comboRobotType->currentData().toInt(),
		       _ui.comboRobotType->currentText().toUtf8().constData());
	}
}


void ObjectEditWidget::loadTrashCompactor(int objectNo) {
	MultiSignalBlocker blocker = { _ui.coordinatesTrashCompactor };
	const MapObject &object = map()->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageTrashCompactor);
	_ui.coordinatesTrashCompactor->setXY(object.x, object.y);
}


void ObjectEditWidget::loadTransporterPad(int objectNo) {
	MultiSignalBlocker blocker = { _ui.coordinatesTransporter, _ui.checkTransporterDisable,
	                               _ui.radioTransporterEOL, _ui.radioTransporterCoordinates,
	                               _ui.coordinatesTransporterDest };
	const MapObject &object = map()->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageTransporter);
	_ui.coordinatesTransporter->setXY(object.x, object.y);
	_ui.checkTransporterDisable->setChecked(object.a == 1);
	_ui.radioTransporterEOL->setChecked(object.b == 0);
	_ui.radioTransporterCoordinates->setChecked(object.b == 1);
	_ui.coordinatesTransporterDest->setXY(object.c, object.d);
}


void ObjectEditWidget::loadWaterRaft(int objectNo) {
	MultiSignalBlocker blocker = { _ui.coordinatesRaft, _ui.radioRaftDirectionLeft,
	                               _ui.radioRaftDirectionRight, _ui.editRaftLeftTurnaround,
	                               _ui.editRaftRightTurnaround };
	const MapObject &object = map()->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageRaft);
	_ui.coordinatesRaft->setXY(object.x, object.y);
	_ui.radioRaftDirectionLeft->setChecked(object.a == 0);
	_ui.radioRaftDirectionRight->setChecked(object.a == 1);
	_ui.editRaftLeftTurnaround->setValue(object.b);
	_ui.editRaftRightTurnaround->setValue(object.c);
}


void ObjectEditWidget::loadWeapon(int objectNo) {
	MultiSignalBlocker blocker = { _ui.coordinatesWeapon, _ui.comboWeaponType,
	                               _ui.editWeaponAmount, _ui.editWeaponContainerWidth,
	                               _ui.editWeaponContainerHeight };
	const MapObject &object = map()->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageWeapon);
	_ui.coordinatesWeapon->setXY(object.x, object.y);
	bool found = false;
	for (int i = 0; i < _ui.comboWeaponType->count(); ++i) {
		if (_ui.comboWeaponType->itemData(i).toInt() == object.unitType) {
			_ui.comboWeaponType->setCurrentIndex(i);
			found = true;
			break;
		}
	}
	if (not found) {
		_ui.comboWeaponType->setCurrentIndex(0);
		qCInfo(lcObjEd, "hidden item unit type %d not found, assuming %d (%s)", object.unitType,
		       _ui.comboWeaponType->currentData().toInt(),
		       _ui.comboWeaponType->currentText().toUtf8().constData());
	}
	_ui.editWeaponAmount->setValue(object.a);
	_ui.editWeaponContainerWidth->setValue(object.c);
	_ui.editWeaponContainerHeight->setValue(object.d);
}


const Map *ObjectEditWidget::map() const {
	if (_mapController) { return _mapController->map(); }
	return nullptr;
}
