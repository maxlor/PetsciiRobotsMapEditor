#include "iconfactory.h"
#include <QPainter>
#include <QRect>
#include <QSize>
#include <QSvgGenerator>
#include <QTemporaryFile>
#include <QtDebug>


IconFactory::IconFactory() {}


IconFactory::~IconFactory() {
	for (QTemporaryFile *tf : _tempFiles) {
		delete tf;
	}
}


const QIcon &IconFactory::icon(const QString text) {
	const std::string sText = text.toStdString();
	auto it = _cache.find(sText);
	if (it != _cache.end()) {
		return it->second;
	}
	
	QTemporaryFile *tf = tempFile();
	
	QSvgGenerator svgGen;
	svgGen.setOutputDevice(tf);
	svgGen.setSize(QSize(128, 128));
	svgGen.setViewBox(QRect(0, 0, 128, 128));
	QPainter painter(&svgGen);
	
	static const QRectF r(8, 8, 112, 112);
	
	painter.setPen(QPen(Qt::black, 8));
	painter.setBrush(Qt::NoBrush);
	painter.drawEllipse(r);
	
	QFont font("Nimbus Sans Narrow", -1, QFont::Bold);
	font.setPixelSize(56);
	painter.setFont(font);
	painter.drawText(r, Qt::AlignCenter, text);
	
	painter.end();
	
	auto pair = _cache.try_emplace(sText, tf->fileName());
	Q_ASSERT(pair.second);
	QIcon &icon = pair.first->second;
	icon.pixmap(128, 128); // force reading of SVG file
	return icon;
}


QTemporaryFile *IconFactory::tempFile() {
	QTemporaryFile *tf = new QTemporaryFile();
	_tempFiles.push_front(tf);
	return tf;
}
