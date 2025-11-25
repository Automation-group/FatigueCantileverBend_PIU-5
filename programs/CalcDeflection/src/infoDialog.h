#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include "../ui/ui_infoWindow.h"

class Dialog : public QDialog {
    Q_OBJECT

public:
    Dialog();

private:
    Ui_Dialog ui;

private Q_SLOTS:
    void on_pushButton_close_clicked(bool checked);
};

#endif
