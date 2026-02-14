#include "qte.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	//srand(time(NULL));

	MainAmplificationWindow mainWin;

    // To display the app in second screen
    QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.size() > 1) {
        QRect screenGeometry = screens.at(1)->geometry();
        mainWin.setGeometry(screenGeometry.x() + 50, screenGeometry.y() + 50, 800, 600);
    }

    mainWin.showMaximized();
	return app.exec();
}
