#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <QIcon>

#define STR(x) _STR(x)
#define _STR(x) #x

int main(int argc, char *argv[]) {
	QApplication::setApplicationName("PETSCII Robots Map Editor");
	QApplication::setApplicationVersion(STR(APP_VERSION));
	QApplication::setOrganizationDomain("maxlor.com");
	QApplication::setOrganizationName("Benjamin Lutz");
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/res/robot-32.png"));
	
	int fontId = -1;
	fontId = QFontDatabase::addApplicationFont(QApplication::applicationDirPath() +
	                                           "/NimbusSansNarrow-Bold.otf");
	Q_ASSERT(fontId != -1);
	
	MainWindow w;
	w.show();
	return a.exec();
}
