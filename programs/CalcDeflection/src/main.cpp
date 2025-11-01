#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":images/icon.png")); // иконка приложения

    MainWindow mainw;
    mainw.show();
    return app.exec();
}
