#ifndef SCROLLAREA_H
#define SCROLLAREA_H

#include <QPoint>
#include <QPointF>
#include <QScrollArea>


class ScrollArea : public QScrollArea {
	Q_OBJECT
	Q_PROPERTY(Qt::MouseButton panButton READ panButton WRITE setPanButton)
	
public:
	ScrollArea(QWidget *parent = nullptr);
	
	Qt::MouseButton panButton() const;
	void setPanButton(Qt::MouseButton panButton);
	
protected:
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	
	Qt::MouseButton _panButton = Qt::LeftButton;
	Qt::MouseButton _panningButton = Qt::NoButton;
	bool _panning = false;
	QPointF _pressPos;
	QPoint _pressScrollbarValues;
};

#endif // SCROLLAREA_H
