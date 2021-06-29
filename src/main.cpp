#include "mainwindow.h"
#include <QApplication>
#include <QFile>
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
	a.setWindowIcon(QIcon(":/robot-32.png"));
	
	const QString fontFile = QApplication::applicationDirPath() + "/NimbusSansNarrow-Bold.otf";
	if (QFile::exists(fontFile)) {
		int fontId = QFontDatabase::addApplicationFont(fontFile);
		Q_UNUSED(fontId);
		Q_ASSERT(fontId != -1);
	}
	
	MainWindow w;
	w.show();
	return a.exec();
}
