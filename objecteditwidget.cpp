#include "objecteditwidget.h"
#include <QLoggingCategory>
#include <forward_list>
#include <unordered_map>
#include "constants.h"
#include "map.h"
#include "multisignalblocker.h"

Q_LOGGING_CATEGORY(lcObjEd, "ObjEd");

ObjectEditWidget::ObjectEditWidget(QWidget *parent) : QGroupBox(parent){
	_ui.setupUi(this);
	
	for (const auto &pair : Map::unitTypes()) {
		switch (Map::Object::kind(pair.first)) {
		case Map::Object::Kind::Robot:
			_ui.comboRobotType->insertItem(_ui.comboRobotType->count(), pair.second, pair.first);
			break;
		case Map::Object::Kind::HiddenObject:
			_ui.comboWeaponType->insertItem(_ui.comboWeaponType->count(), pair.second, pair.first);
			break;
		default:;
		}
	}
	
	for (int i : { 0, 1, 3, 4 }) {
		_ui.comboDoorState->setItemData(i, QColor(Qt::gray), Qt::TextColorRole);
		_ui.comboElevatorState->setItemData(i, QColor(Qt::gray), Qt::TextColorRole);
	}
	
	union {
		QSpinBox *spinBox;
		QComboBox *comboBox;
		QCheckBox *checkBox;
		QRadioButton *radioButton;
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
				connect(u.checkBox, &QCheckBox::toggled, this, &ObjectEditWidget::store);
			} else if ((u.radioButton = qobject_cast<QRadioButton*>(child))) {
				connect(u.radioButton, &QCheckBox::toggled, this, &ObjectEditWidget::store);
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
	static const std::unordered_map<Map::Object::Kind, QString> groupNames {
		{ Map::Object::Kind::Player, "Player" },
		{ Map::Object::Kind::Robot, "Robot" },
		{ Map::Object::Kind::TransporterPad, "Transporter Pad" },
		{ Map::Object::Kind::Door, "Door" },
		{ Map::Object::Kind::TrashCompator, "Trash Compactor" },
		{ Map::Object::Kind::Elevator, "Elevator" },
		{ Map::Object::Kind::WaterRaft, "Water Raft" },
		{ Map::Object::Kind::Key, "Key" },
		{ Map::Object::Kind::HiddenObject, "Hidden Weapon / Item" }
	};
	_objectNo = -1;
	
	if (not _map) {
		qCWarning(lcObjEd, "can't edit object, map not set");
		setVisible(false);
		return;
	}
	
	if (objectNo == -1) {
		setVisible(false);
		return;
	}
	
	const Map::Object &object = _map->object(objectNo);
	const Map::Object::Kind kind = object.kind();
	try {
		setTitle(groupNames.at(kind));
	} catch (std::out_of_range) {
		qCWarning(lcObjEd, "can't edit object %d of kind %d", objectNo, (int)object.kind());
		setVisible(false);
		return;
	}
	
	switch (kind) {
	case Map::Object::Kind::Player: loadPlayer(); break;
	case Map::Object::Kind::Door: loadDoor(objectNo); break;
	case Map::Object::Kind::Elevator: loadElevator(objectNo); break;
	case Map::Object::Kind::Key: loadKey(objectNo); break;
	case Map::Object::Kind::HiddenObject: loadWeapon(objectNo); break;
	case Map::Object::Kind::Robot: loadRobot(objectNo); break;
	case Map::Object::Kind::TrashCompator: loadTrashCompactor(objectNo); break;
	case Map::Object::Kind::TransporterPad: loadTransporterPad(objectNo); break;
	case Map::Object::Kind::WaterRaft: loadWaterRaft(objectNo); break;
	case Map::Object::Kind::Invalid: setVisible(false); break;
	}
	
	_objectNo = objectNo;
	setVisible(true);
}


void ObjectEditWidget::setMap(Map *map) {
	_map = map;
}


void ObjectEditWidget::store() {
	Map::Object object = _map->object(_objectNo);
	if (_ui.stackedWidget->currentWidget() == _ui.pagePlayer) {
		object.x = _ui.editPlayerX->value();
		object.y = _ui.editPlayerY->value();
		object.health = _ui.editPlayerHealth->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageRobot) {
		object.x = _ui.editRobotX->value();
		object.y = _ui.editRobotY->value();
		object.health = _ui.editRobotHealth->value();
		object.unitType = _ui.comboRobotType->currentData().toInt();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageWeapon) {
		object.x = _ui.editWeaponX->value();
		object.y = _ui.editWeaponY->value();
		object.unitType = _ui.comboWeaponType->currentData().toInt();
		object.a = _ui.editWeaponAmount->value();
		object.c = _ui.editWeaponContainerWidth->value();
		object.c = _ui.editWeaponContainerHeight->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageDoor) {
		object.x = _ui.editDoorX->value();
		object.y = _ui.editDoorY->value();
		object.a = _ui.comboDoorOrientation->currentIndex();
		object.b = _ui.comboDoorState->currentIndex();
		object.c = _ui.comboDoorLock->currentIndex();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageKey) {
		object.x = _ui.editKeyX->value();
		object.y = _ui.editKeyY->value();
		if (_ui.radioKeyHeart->isChecked()) { object.a = 1; }
		else if (_ui.radioKeyStar->isChecked()) { object.a = 2; }
		else { object.a = 0; }
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageTrashCompactor) {
		object.x = _ui.editTrashCompactorX->value();
		object.y = _ui.editTrashCompactorY->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageElevator) {
		object.x = _ui.editElevatorX->value();
		object.y = _ui.editElevatorY->value();
		object.b = _ui.comboElevatorState->currentIndex();
		object.c = _ui.editElevatorFloor->value();
		object.d = _ui.editElevatorTotalFloors->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageRaft) {
		object.x = _ui.editRaftX->value();
		object.y = _ui.editRaftY->value();
		object.a = _ui.radioRaftDirectionRight->isChecked() ? 1 : 0;
		object.b = _ui.editRaftLeftTurnaround->value();
		object.c = _ui.editRaftRightTurnaround->value();
	} else if (_ui.stackedWidget->currentWidget() == _ui.pageTransporter) {
		object.x = _ui.editTransporterX->value();
		object.y = _ui.editTransporterY->value();
		object.a = _ui.checkTransporterDisable->isChecked() ? 1 : 0;
		object.b = _ui.radioTransporterCoordinates->isChecked() ? 1 : 0;
		object.c = _ui.editTransporterDestX->value();
		object.d = _ui.editTransporterDestY->value();
	} else {
		Q_ASSERT(false);
	}
	_map->setObject(_objectNo, object);
}


void ObjectEditWidget::loadDoor(int objectNo) {
	MultiSignalBlocker blocker = { _ui.editDoorX, _ui.editDoorY, _ui.comboDoorOrientation,
	                               _ui.comboDoorState, _ui.comboDoorLock };
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageDoor);
	_ui.editDoorX->setValue(object.x);
	_ui.editDoorY->setValue(object.y);
	_ui.comboDoorOrientation->setCurrentIndex(object.a ? 1 : 0);
	_ui.comboDoorState->setCurrentIndex(object.b < 6 ? object.b : 5);
	_ui.comboDoorLock->setCurrentIndex(object.c < 4 ? object.c : 0);
}


void ObjectEditWidget::loadElevator(int objectNo) {
	MultiSignalBlocker blocker = { _ui.editElevatorX, _ui.editElevatorY, _ui.comboElevatorState,
	                               _ui.editElevatorFloor, _ui.editElevatorTotalFloors };
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageElevator);
	_ui.editElevatorX->setValue(object.x);
	_ui.editElevatorY->setValue(object.y);
	_ui.comboElevatorState->setCurrentIndex(object.b < 6 ? object.b : 5);
	_ui.editElevatorFloor->setValue(object.c);
	_ui.editElevatorTotalFloors->setValue(object.d);
}


void ObjectEditWidget::loadKey(int objectNo) {
	MultiSignalBlocker blocker = { _ui.editKeyX, _ui.editKeyY, _ui.radioKeySpade,
	                               _ui.radioKeyHeart, _ui.radioKeyStar };
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageKey);
	_ui.editKeyX->setValue(object.x);
	_ui.editKeyY->setValue(object.y);
	_ui.radioKeySpade->setChecked(object.a == 0);
	_ui.radioKeyHeart->setChecked(object.a == 1);
	_ui.radioKeyStar->setChecked(object.a == 2);
}


void ObjectEditWidget::loadPlayer() {
	MultiSignalBlocker blocker = { _ui.editPlayerX, _ui.editPlayerY, _ui.editPlayerHealth };
	const Map::Object &object = _map->object(PLAYER_OBJ);
	_ui.stackedWidget->setCurrentWidget(_ui.pagePlayer);
	_ui.editPlayerX->setValue(object.x);
	_ui.editPlayerY->setValue(object.y);
	_ui.editPlayerHealth->setValue(object.health);
}


void ObjectEditWidget::loadRobot(int objectNo) {
	MultiSignalBlocker blocker = { _ui.editRobotX, _ui.editRobotY, _ui.editRobotHealth,
	                               _ui.comboRobotType };
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageRobot);
	_ui.editRobotX->setValue(object.x);
	_ui.editRobotY->setValue(object.y);
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
	MultiSignalBlocker blocker = { _ui.editTrashCompactorX, _ui.editTrashCompactorY };
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageTrashCompactor);
	_ui.editTrashCompactorX->setValue(object.x);
	_ui.editTrashCompactorY->setValue(object.y);
}


void ObjectEditWidget::loadTransporterPad(int objectNo) {
	MultiSignalBlocker blocker = { _ui.editTransporterX, _ui.editTransporterY,
	                               _ui.checkTransporterDisable, _ui.radioTransporterEOL,
	                               _ui.radioTransporterCoordinates, _ui.editTransporterDestX,
	                               _ui.editTransporterDestY };
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageTransporter);
	_ui.editTransporterX->setValue(object.x);
	_ui.editTransporterY->setValue(object.y);
	_ui.checkTransporterDisable->setChecked(object.a == 1);
	_ui.radioTransporterEOL->setChecked(object.b == 0);
	_ui.radioTransporterCoordinates->setChecked(object.b == 1);
	_ui.editTransporterDestX->setValue(object.c);
	_ui.editTransporterDestY->setValue(object.d);
}


void ObjectEditWidget::loadWaterRaft(int objectNo) {
	MultiSignalBlocker blocker = { _ui.editRaftX, _ui.editRaftY, _ui.radioRaftDirectionLeft,
	                               _ui.radioRaftDirectionRight, _ui.editRaftLeftTurnaround,
	                               _ui.editRaftRightTurnaround };
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageRaft);
	_ui.editRaftX->setValue(object.x);
	_ui.editRaftY->setValue(object.y);
	_ui.radioRaftDirectionLeft->setChecked(object.a == 0);
	_ui.radioRaftDirectionRight->setChecked(object.a == 1);
	_ui.editRaftLeftTurnaround->setValue(object.b);
	_ui.editRaftRightTurnaround->setValue(object.c);
}


void ObjectEditWidget::loadWeapon(int objectNo) {
	MultiSignalBlocker blocker = { _ui.editWeaponX, _ui.editWeaponY, _ui.comboWeaponType,
	                               _ui.editWeaponAmount, _ui.editWeaponContainerWidth,
	                               _ui.editWeaponContainerHeight };
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageWeapon);
	_ui.editWeaponX->setValue(object.x);
	_ui.editWeaponY->setValue(object.y);
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
