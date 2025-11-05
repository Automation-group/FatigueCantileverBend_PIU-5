#include "mainwindow.h"
#include <QApplication>
#include <QtGlobal>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    #if Q_OS_WIN
        app.setWindowIcon(QIcon(":images/icon.ico")); // Windows (Q_OS_WIN)
    #else
        app.setWindowIcon(QIcon(":images/icon.png")); // Linux (Q_OS_LINUX)
    #endif

    MainWindow mainw;
    mainw.show();
    return app.exec();
}
