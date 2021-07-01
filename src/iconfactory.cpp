#include "iconfactory.h"
#include <QPainter>
#include <QRect>
#include <QSize>
#include <QSvgGenerator>
#include <QTemporaryFile>


/** @class IconFactory
 * Produces icons consisting of a circle with a little bit of text in them.
 * 3-4 characters will fit. The icons are used for some QActions. 
 * 
 * The icons are also cached after their initial creation, so calling
 * #icon() the second time around for the same icon is fast, as long
 * as the IconFactory object hasn't been destroyed in the meantime.
 * 
 * #### Important Note ####
 * 
 * IconFactory creates temporary SVG files, which are read by QIcon. It seems
 * that whenever QIcon is used in a new context where a new icon size is
 * required, the SVG file is reread. However, these SVG files exist only while
 * the IconFactory object lives, they're deleted in the destructor.
 * 
 * This means that **you must keep the IconFactory object around until the icons
 * have been applied to all places where they're going to be applied!**
 */

IconFactory::IconFactory() {}


IconFactory::~IconFactory() {
	for (QTemporaryFile *tf : _tempFiles) {
		delete tf;
	}
}


/** Create an icon with \a text in it.
 * Creates an icon consisting of a circle with \a text in it. After the icon
 * has been created, it's cached.
 */
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
	return icon;
}


QTemporaryFile *IconFactory::tempFile() {
	QTemporaryFile *tf = new QTemporaryFile();
	_tempFiles.push_front(tf);
	return tf;
}
