#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setApplicationName("KC_Edit");
	app.setApplicationVersion("Version 1.0");
	app.setOrganizationName("djRickyH");
	app.setApplicationDisplayName("KC_EDIT");

	MainWindow window;
	window.show();

	return app.exec();
}
