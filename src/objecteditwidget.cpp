#include "objecteditwidget.h"
#include <QLoggingCategory>
#include "map.h"
#include "mapcontroller.h"
#include "multisignalblocker.h"
#include "util.h"

Q_LOGGING_CATEGORY(lcObjEd, "ObjEd");


ObjectEditWidget::ObjectEditWidget(QWidget *parent) : QGroupBox(parent){
	_ui.setupUi(this);
	
	for (MapObject::UnitType unitType : MapObject::unitTypes()) {
		if (MapObject::group(unitType) == MapObject::Group::Robots) {
			_ui.comboRobotType->insertItem(_ui.comboRobotType->count(),
			                               capitalize(MapObject::toString(unitType)),
			                               MapObject::unitType_t(unitType));
		} else if (MapObject::group(unitType) == MapObject::Group::HiddenObjects and
		           unitType != MapObject::UnitType::Key) {
			_ui.comboWeaponType->insertItem(_ui.comboWeaponType->count(),
			                                capitalize(MapObject::toString(unitType)),
			                                MapObject::unitType_t(unitType));
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


void ObjectEditWidget::loadObject(MapObject::id_t objectId) {
	_objectId = MapObject::IdNone;
	
	if (not _mapController) {
		qCWarning(lcObjEd, "can't edit object, _mapController not set");
		setVisible(false);
		return;
	}
	
	if (objectId == -1) {
		setVisible(false);
		return;
	}
	
	const MapObject &object = _mapController->map()->object(objectId);
	
	switch (object.unitType) {
	case MapObject::UnitType::None: setVisible(false); return;
	case MapObject::UnitType::Player: loadPlayer(object); break;
	case MapObject::UnitType::HoverbotLR:
	case MapObject::UnitType::HoverbotUD:
	case MapObject::UnitType::HoverbotAttack:
	case MapObject::UnitType::Evilbot:
	case MapObject::UnitType::RollerbotUD:
	case MapObject::UnitType::RollerbotLR: loadRobot(object); break;
	case MapObject::UnitType::TransporterPad: loadTransporterPad(object); break;
	case MapObject::UnitType::Door: loadDoor(object); break;
	case MapObject::UnitType::TrashCompactor: loadTrashCompactor(object); break;
	case MapObject::UnitType::Elevator: loadElevator(object); break;
	case MapObject::UnitType::WaterRaft: loadWaterRaft(object); break;
	case MapObject::UnitType::Key: loadKey(object); break;
	case MapObject::UnitType::TimeBomb:
	case MapObject::UnitType::EMP:
	case MapObject::UnitType::Pistol:
	case MapObject::UnitType::PlasmaGun:
	case MapObject::UnitType::Medkit:
	case MapObject::UnitType::Magnet: loadWeapon(object); break;
	}
	
	_objectId = objectId;
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
	if (_objectId != MapObject::IdNone) {
		loadObject(_objectId);
	}
}


void ObjectEditWidget::onCoordinateMapClickRequested(const QString &label) {
	_mapClickRequester = qobject_cast<CoordinateWidget*>(sender());
	if (_mapClickRequester) {
		emit mapClickRequested(label);
	}
}


void ObjectEditWidget::store() {
	MapObject object = _mapController->map()->object(_objectId);
	if (_ui.stackedWidget->currentWidget() == _ui.pagePlayer) {
		object.x = _ui.coordinatesPlayer->x();
		object.y = _ui.coordinatesPlayer->y();
		object.health = _ui.editPlayerHealth->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageRobot) {
		object.x = _ui.coordinatesRobot->x();
		object.y = _ui.coordinatesRobot->y();
		object.health = _ui.editRobotHealth->value();
		object.unitType = MapObject::UnitType(_ui.comboRobotType->currentData().toUInt());
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageWeapon) {
		object.x = _ui.coordinatesWeapon->x();
		object.y = _ui.coordinatesWeapon->y();
		object.unitType = MapObject::UnitType(_ui.comboWeaponType->currentData().toUInt());
		object.a = _ui.editWeaponAmount->value();
		object.c = _ui.editWeaponContainerWidth->value() - 1;
		object.d = _ui.editWeaponContainerHeight->value() - 1;
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
	_mapController->setObject(_objectId, object);
}


void ObjectEditWidget::loadDoor(const MapObject &object) {
	MultiSignalBlocker blocker = { _ui.coordinatesDoor, _ui.comboDoorOrientation,
	                               _ui.comboDoorState, _ui.comboDoorLock };
	setTitle(capitalize(MapObject::toString(object.unitType)));
	_ui.stackedWidget->setCurrentWidget(_ui.pageDoor);
	_ui.coordinatesDoor->setXY(object.x, object.y);
	_ui.comboDoorOrientation->setCurrentIndex(object.a ? 1 : 0);
	_ui.comboDoorState->setCurrentIndex(object.b < 6 ? object.b : 5);
	_ui.comboDoorLock->setCurrentIndex(object.c < 4 ? object.c : 0);
}


void ObjectEditWidget::loadElevator(const MapObject &object) {
	MultiSignalBlocker blocker = { _ui.coordinatesElevator, _ui.comboElevatorState,
	                               _ui.editElevatorFloor, _ui.editElevatorTotalFloors };
	setTitle(capitalize(MapObject::toString(object.unitType)));
	_ui.stackedWidget->setCurrentWidget(_ui.pageElevator);
	_ui.coordinatesElevator->setXY(object.x, object.y);
	_ui.comboElevatorState->setCurrentIndex(object.b < 6 ? object.b : 5);
	_ui.editElevatorFloor->setValue(object.c);
	_ui.editElevatorTotalFloors->setValue(object.d);
}


void ObjectEditWidget::loadKey(const MapObject &object) {
	MultiSignalBlocker blocker = { _ui.coordinatesKey, _ui.radioKeySpade, _ui.radioKeyHeart,
	                               _ui.radioKeyStar, _ui.editKeyContainerWidth,
	                               _ui.editKeyContainerHeight };
	setTitle(capitalize(MapObject::toString(object.unitType)));
	_ui.stackedWidget->setCurrentWidget(_ui.pageKey);
	_ui.coordinatesKey->setXY(object.x, object.y);
	_ui.radioKeySpade->setChecked(object.a == 0);
	_ui.radioKeyHeart->setChecked(object.a == 1);
	_ui.radioKeyStar->setChecked(object.a == 2);
	_ui.editKeyContainerWidth->setValue(object.c);
	_ui.editKeyContainerHeight->setValue(object.d);
}


void ObjectEditWidget::loadPlayer(const MapObject &object) {
	MultiSignalBlocker blocker = { _ui.coordinatesPlayer, _ui.editPlayerHealth };
	setTitle(capitalize(MapObject::toString(object.unitType)));
	_ui.stackedWidget->setCurrentWidget(_ui.pagePlayer);
	_ui.coordinatesPlayer->setXY(object.x, object.y);
	_ui.editPlayerHealth->setValue(object.health);
}


void ObjectEditWidget::loadRobot(const MapObject &object) {
	MultiSignalBlocker blocker = { _ui.coordinatesRobot, _ui.editRobotHealth,
	                               _ui.comboRobotType };
	QString title = capitalize(MapObject::toString(object.group()));
	if (title.endsWith('s')) { title.chop(1); }
	setTitle(title);
	_ui.stackedWidget->setCurrentWidget(_ui.pageRobot);
	_ui.coordinatesRobot->setXY(object.x, object.y);
	_ui.editRobotHealth->setValue(object.health);
	bool found = false;
	for (int i = 0; i < _ui.comboRobotType->count(); ++i) {
		if (MapObject::UnitType(_ui.comboRobotType->itemData(i).toUInt()) == object.unitType) {
			_ui.comboRobotType->setCurrentIndex(i);
			found = true;
			break;
		}
	}
	if (not found) {
		_ui.comboRobotType->setCurrentIndex(0);
		qCInfo(lcObjEd, "robot unit type %hhu not found, assuming %d (%s)", object.unitType,
		       _ui.comboRobotType->currentData().toInt(),
		       _ui.comboRobotType->currentText().toUtf8().constData());
	}
}


void ObjectEditWidget::loadTrashCompactor(const MapObject &object) {
	MultiSignalBlocker blocker = { _ui.coordinatesTrashCompactor };
	setTitle(capitalize(MapObject::toString(object.unitType)));
	_ui.stackedWidget->setCurrentWidget(_ui.pageTrashCompactor);
	_ui.coordinatesTrashCompactor->setXY(object.x, object.y);
}


void ObjectEditWidget::loadTransporterPad(const MapObject &object) {
	MultiSignalBlocker blocker = { _ui.coordinatesTransporter, _ui.checkTransporterDisable,
	                               _ui.radioTransporterEOL, _ui.radioTransporterCoordinates,
	                               _ui.coordinatesTransporterDest };
	setTitle(capitalize(MapObject::toString(object.unitType)));
	_ui.stackedWidget->setCurrentWidget(_ui.pageTransporter);
	_ui.coordinatesTransporter->setXY(object.x, object.y);
	_ui.checkTransporterDisable->setChecked(object.a == 1);
	_ui.radioTransporterEOL->setChecked(object.b == 0);
	_ui.radioTransporterCoordinates->setChecked(object.b == 1);
	_ui.coordinatesTransporterDest->setXY(object.c, object.d);
}


void ObjectEditWidget::loadWaterRaft(const MapObject &object) {
	MultiSignalBlocker blocker = { _ui.coordinatesRaft, _ui.radioRaftDirectionLeft,
	                               _ui.radioRaftDirectionRight, _ui.editRaftLeftTurnaround,
	                               _ui.editRaftRightTurnaround };
	setTitle(capitalize(MapObject::toString(object.unitType)));
	_ui.stackedWidget->setCurrentWidget(_ui.pageRaft);
	_ui.coordinatesRaft->setXY(object.x, object.y);
	_ui.radioRaftDirectionLeft->setChecked(object.a == 0);
	_ui.radioRaftDirectionRight->setChecked(object.a == 1);
	_ui.editRaftLeftTurnaround->setValue(object.b);
	_ui.editRaftRightTurnaround->setValue(object.c);
}


void ObjectEditWidget::loadWeapon(const MapObject &object) {
	MultiSignalBlocker blocker = { _ui.coordinatesWeapon, _ui.comboWeaponType,
	                               _ui.editWeaponAmount, _ui.editWeaponContainerWidth,
	                               _ui.editWeaponContainerHeight };
	QString title = capitalize(MapObject::toString(object.group()));
	if (title.endsWith('s')) { title.chop(1); }
	setTitle(title);
	_ui.stackedWidget->setCurrentWidget(_ui.pageWeapon);
	_ui.coordinatesWeapon->setXY(object.x, object.y);
	bool found = false;
	for (int i = 0; i < _ui.comboWeaponType->count(); ++i) {
		if (MapObject::UnitType(_ui.comboWeaponType->itemData(i).toUInt()) == object.unitType) {
			_ui.comboWeaponType->setCurrentIndex(i);
			found = true;
			break;
		}
	}
	if (not found) {
		_ui.comboWeaponType->setCurrentIndex(0);
		qCInfo(lcObjEd, "hidden item unit type %hhu not found, assuming %d (%s)", object.unitType,
		       _ui.comboWeaponType->currentData().toInt(),
		       _ui.comboWeaponType->currentText().toUtf8().constData());
	}
	_ui.editWeaponAmount->setValue(object.a);
	_ui.editWeaponContainerWidth->setValue(object.c + 1);
	_ui.editWeaponContainerHeight->setValue(object.d + 1);
}
