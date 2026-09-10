#include "main-window.h"

#include <QtGui/QScreen>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	//srand(time(NULL));

	MainWindow mainWin;

    // Open on the second screen if there is one, otherwise the main screen.
    const QList<QScreen*> screens = QGuiApplication::screens();
    QScreen* targetScreen = (screens.size() > 1) ? screens.at(1) : QGuiApplication::primaryScreen();

    mainWin.move(targetScreen->availableGeometry().topLeft());
    mainWin.showMaximized();
	return app.exec();
}
