#include "validationdialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QSpacerItem>


ValidationDialog::ValidationDialog(MapCheck &mapCheck, QWidget *parent)
    : QDialog(parent), _mapCheck(mapCheck) {
	
	_ui.setupUi(this);
	_ui.buttons->button(QDialogButtonBox::Ok)->setText("Fix All");
	
	_ui.table->setColumnCount(2);
	
	connect(_ui.table, &QTableWidget::cellClicked, this, &ValidationDialog::onCellClicked);
	
	loadProblems();
}


void ValidationDialog::resizeEvent(QResizeEvent *event) {
	Q_UNUSED(event);
	_ui.table->setColumnWidth(1, 100);
	_ui.table->setColumnWidth(0, _ui.table->contentsRect().width() - _ui.table->columnWidth(1));
}


void ValidationDialog::onCellClicked(int row, int col) {
	Q_UNUSED(col);
	if (0 <= row and size_t(row) < _mapCheck.problems().size()) {
		emit requestSelectObject(_mapCheck.problems()[row].objectId);
	}
}


void ValidationDialog::loadProblems() {
	static const QIcon info(":/info.svg");
	static const QIcon warning(":/caution.svg");
	static const QIcon error(":/stop.svg");
	
	_ui.table->clearContents();
	_ui.table->setRowCount(_mapCheck.problems().size());
	
	int row = 0;
	for (const MapCheck::Problem &problem : _mapCheck.problems()) {
		const QIcon *icon;
		switch (problem.severity) {
		case MapCheck::Severity::Silent: Q_ASSERT(false); continue;
		case MapCheck::Severity::Info: icon = &info; break;
		case MapCheck::Severity::Warning: icon = &warning; break;
		case MapCheck::Severity::Error: icon = &error; break;
		}
		_ui.table->setItem(row, 0, new QTableWidgetItem(*icon, problem.text));
		
		if (problem.fix) {
			QToolButton *button = new QToolButton(_ui.table);
			connect(button, &QToolButton::clicked, button, [&]() {
				problem.fix();
				_mapCheck.check();
				loadProblems();
			});
			button->setText("Fix");
			_ui.table->setCellWidget(row, 1, button);
		}
		
		++row;
	}
}

