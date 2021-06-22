#include "multisignalblocker.h"
#include <QObject>


MultiSignalBlocker::MultiSignalBlocker(std::initializer_list<QObject *> objects)
	: _objects(objects) {
	for (QObject *obj : _objects) {
		obj->blockSignals(true);
	}
}


MultiSignalBlocker::~MultiSignalBlocker() {
	for (QObject *obj : _objects) {
		obj->blockSignals(false);
	}
}
