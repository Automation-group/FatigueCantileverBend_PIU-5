#include "mainwindow.h"
#include <QApplication>
#include <QtGlobal>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
	// OS Windows
    #if defined(Q_OS_WIN)
        app.setWindowIcon(QIcon(":images/icon.ico"));
	#endif
	// OS Linux
	#if defined(Q_OS_LINUX)
        app.setWindowIcon(QIcon(":images/icon.png"));
    #endif
	
    MainWindow mainw;
    mainw.show();
    return app.exec();
}
