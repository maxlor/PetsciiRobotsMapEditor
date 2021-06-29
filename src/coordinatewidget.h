#ifndef COORDINATEWIDGET_H
#define COORDINATEWIDGET_H

#include <QString>
#include <QWidget>

class QSpinBox;
class QToolButton;


class CoordinateWidget : public QWidget {
	Q_OBJECT
	Q_PROPERTY(QString label READ label WRITE setLabel)
	Q_PROPERTY(int x READ x WRITE setX NOTIFY valueChanged)
	Q_PROPERTY(int y READ y WRITE setY NOTIFY valueChanged)
public:
	explicit CoordinateWidget(QWidget *parent = nullptr);
	
	const QString &label() const;
	int x() const;
	int y() const;
	
	void setLabel(const QString &label);
	void setX(int x);
	void setY(int y);
	
	void setXY(int x, int y);
	
signals:
	void valueChanged(int x, int y);
	void mapClickRequested(const QString &label);
	
private slots:
	void onGetCoordinatesClicked();
	void onValueChanged();

private:
	QToolButton *_buttonGetCoordinates;	
	QSpinBox *_editX;
	QSpinBox *_editY;
	QString _label;
};

#endif // COORDINATEWIDGET_H
