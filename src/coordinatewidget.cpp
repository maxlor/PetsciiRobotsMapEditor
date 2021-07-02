#include "coordinatewidget.h"
#include <QHBoxLayout>
#include <QSpinBox>
#include <QToolButton>
#include "constants.h"


CoordinateWidget::CoordinateWidget(QWidget *parent) : QWidget(parent) {
	_editX = new QSpinBox(this);
	_editY = new QSpinBox(this);
	
	_buttonGetCoordinates = new QToolButton(this);
	_buttonGetCoordinates->setIcon(QIcon(":/target.svg"));
	_buttonGetCoordinates->setToolTip("Set position by clicking on map");
	
	QHBoxLayout *layout = new QHBoxLayout();
	layout->setMargin(0);
	setLayout(layout);
	layout->addWidget(_editX, 0);
	layout->addWidget(_editY, 0);
	layout->addWidget(_buttonGetCoordinates, 0);
	
	connect(_editX, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
	        this, &CoordinateWidget::onValueChanged);
	connect(_editY, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
	        this, &CoordinateWidget::onValueChanged);
	connect(_buttonGetCoordinates, &QToolButton::clicked,
	        this, &CoordinateWidget::onGetCoordinatesClicked);
}


void CoordinateWidget::setRect(const QRect &rect) {
	_editX->setMinimum(rect.left());
	_editX->setMaximum(rect.right());
	_editY->setMinimum(rect.top());
	_editY->setMaximum(rect.bottom());
}


const QString &CoordinateWidget::label() const {
	return _label;
}


int CoordinateWidget::x() const {
	return _editX->value();
}


int CoordinateWidget::y() const {
	return _editY->value();
}


void CoordinateWidget::setLabel(const QString &label) {
	_label = label;
	_buttonGetCoordinates->setToolTip(QString("Set %1 by clicking on map").arg(label));
}


void CoordinateWidget::setX(int x) {
	_editX->setValue(x);
}


void CoordinateWidget::setY(int y) {
	_editY->setValue(y);
}


void CoordinateWidget::setXY(int x, int y) {
	_editX->setValue(x);
	_editY->setValue(y);
}


void CoordinateWidget::onGetCoordinatesClicked() {
	emit mapClickRequested(_label);
}


void CoordinateWidget::onValueChanged() {
	emit valueChanged(x(), y());
}
