#include "mainwindow.h"
#include <QApplication>
#include <QtGlobal>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    #if Q_OS_WIN
        app.setWindowIcon(QIcon(":images/icon.ico")); // OS Windows
    #else
        app.setWindowIcon(QIcon(":images/icon.png")); // OS Linux (Q_OS_LINUX)
    #endif

    MainWindow mainw;
    mainw.show();
    return app.exec();
}
