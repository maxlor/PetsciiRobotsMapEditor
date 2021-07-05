#ifndef VALIDATIONDIALOG_H
#define VALIDATIONDIALOG_H

#include "ui_validationdialog.h"
#include <QAbstractTableModel>
#include <QDialog>
#include <QModelIndex>
#include <QVariant>
#include "mapcheck.h"


class ValidationDialog : public QDialog {
	Q_OBJECT
public:
	explicit ValidationDialog(MapCheck &mapCheck, QWidget *parent = nullptr);

signals:
	void requestSelectObject(MapObject::id_t objectId);
	
protected:
	void resizeEvent(QResizeEvent *event) override;
	
private slots:
	void onCellClicked(int row, int col);
	
private:
	void loadProblems();
	
	Ui::ValidationDialog _ui;
	MapCheck &_mapCheck;
};

#endif // VALIDATIONDIALOG_H
