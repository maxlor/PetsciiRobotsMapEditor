#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

#define STR(x) _STR(x)
#define _STR(x) #x

int main(int argc, char *argv[]) {
	QApplication::setApplicationName("PETSCII Robots Map Editor");
	QApplication::setApplicationVersion(STR(APP_VERSION));
	QApplication::setOrganizationDomain("maxlor.com");
	QApplication::setOrganizationName("Benjamin Lutz");
	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/res/robot-32.png"));
	MainWindow w;
	w.show();
	return a.exec();
}
