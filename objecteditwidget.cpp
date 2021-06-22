#include "objecteditwidget.h"
#include <QLoggingCategory>
#include <unordered_map>
#include "constants.h"
#include "map.h"

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
}


void ObjectEditWidget::loadObject(int objectNo) {
	static const std::unordered_map<Map::Object::Kind, QString> groupNames {
		{ Map::Object::Kind::Robot, "Robot " },
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
	
	Map::Object &object = _map->object(objectNo);
	const Map::Object::Kind kind = object.kind();
	if (objectNo == 0) {
		setTitle("Player");
	} else {
		try {
			setTitle(groupNames.at(kind));
		} catch (std::out_of_range) {
			qCWarning(lcObjEd, "can't edit object %d of kind %d", objectNo, (int)object.kind());
			setVisible(false);
			return;
		}
	}
	
	if (objectNo == PLAYER_OBJ) { loadPlayer(); }
	else switch (kind) {
	case Map::Object::Kind::Door: loadDoor(objectNo); break;
	case Map::Object::Kind::Elevator: loadElevator(objectNo); break;
	case Map::Object::Kind::Key: loadKey(objectNo); break;
	case Map::Object::Kind::HiddenObject: loadWeapon(objectNo); break;
	case Map::Object::Kind::Robot: loadRobot(objectNo); break;
	case Map::Object::Kind::TrashCompator: loadTrashCompactor(objectNo); break;
	case Map::Object::Kind::TransporterPad: loadTransporterPad(objectNo); break;
	case Map::Object::Kind::WaterRaft: loadWaterRaft(objectNo); break;
	default: // TODO remove once all switch cases are implemented
		setVisible(false);
		return;
	}
	
	for (int i = 0; i < _ui.stackedWidget->count(); ++i) {
		QWidget *widget = _ui.stackedWidget->widget(i);
		bool visible = _ui.stackedWidget->currentWidget() == widget;
		for (QObject *obj : widget->children()) {
			QWidget *childWidget = qobject_cast<QWidget*>(obj);
			if (childWidget) { childWidget->setVisible(visible); }
		}
	}
	
	_objectNo = objectNo;
	setVisible(true);
}


void ObjectEditWidget::setMap(Map *map) {
	_map = map;
}


void ObjectEditWidget::loadDoor(int objectNo) {
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageDoor);
	_ui.editDoorX->setValue(object.x);
	_ui.editDoorY->setValue(object.y);
	_ui.comboDoorOrientation->setCurrentIndex(object.a ? 1 : 0);
	_ui.comboDoorState->setCurrentIndex(object.b < 6 ? object.b : 5);
	_ui.comboDoorLock->setCurrentIndex(object.c < 4 ? object.c : 0);
}


void ObjectEditWidget::loadElevator(int objectNo) {
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageElevator);
	_ui.editElevatorX->setValue(object.x);
	_ui.editElevatorY->setValue(object.y);
	_ui.comboElevatorState->setCurrentIndex(object.b < 6 ? object.b : 5);
	_ui.editElevatorFloor->setValue(object.c);
	_ui.editElevatorTotalFloors->setValue(object.d);
}


void ObjectEditWidget::loadKey(int objectNo) {
	const Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageKey);
	_ui.editKeyX->setValue(object.x);
	_ui.editKeyY->setValue(object.y);
	_ui.radioKeySpade->setChecked(object.a == 0);
	_ui.radioKeyHeart->setChecked(object.a == 1);
	_ui.radioKeyStar->setChecked(object.a == 2);
}


void ObjectEditWidget::loadPlayer() {
	const Map::Object &object = _map->object(PLAYER_OBJ);
	_ui.stackedWidget->setCurrentWidget(_ui.pagePlayer);
	_ui.editPlayerX->setValue(object.x);
	_ui.editPlayerY->setValue(object.y);
	_ui.editPlayerHealth->setValue(object.health);
}


void ObjectEditWidget::loadRobot(int objectNo) {
	Map::Object &object = _map->object(objectNo);
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
		object.unitType = _ui.comboRobotType->currentData().toInt();
	}
}


void ObjectEditWidget::loadTrashCompactor(int objectNo) {
	Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageTrashCompactor);
	_ui.editTrashCompactorX->setValue(object.x);
	_ui.editTrashCompactorY->setValue(object.y);
}


void ObjectEditWidget::loadTransporterPad(int objectNo) {
	Map::Object &object = _map->object(objectNo);
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
	Map::Object &object = _map->object(objectNo);
	_ui.stackedWidget->setCurrentWidget(_ui.pageRaft);
	_ui.editRaftX->setValue(object.x);
	_ui.editRaftY->setValue(object.y);
	_ui.radioRaftDirectionLeft->setChecked(object.a == 0);
	_ui.radioRaftDirectionRight->setChecked(object.a == 1);
	_ui.editRaftLeftTurnaround->setValue(object.b);
	_ui.editRaftRightTurnaround->setValue(object.c);
}


void ObjectEditWidget::loadWeapon(int objectNo) {
	Map::Object &object = _map->object(objectNo);
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
		object.unitType = _ui.comboWeaponType->currentData().toInt();
	}
	_ui.editWeaponAmount->setValue(object.a);
	_ui.editWeaponContainerWidth->setValue(object.c);
	_ui.editWeaponContainerHeight->setValue(object.d);
}
