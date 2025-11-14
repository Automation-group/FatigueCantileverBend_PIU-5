#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSettings>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    unsigned int calcResultId = 0;
    QSettings *settings_CalcDeflection; // настройки программы
    void saveSettings();
    void loadSettings();

protected:
    void closeEvent(QCloseEvent *close);

protected slots:
    void slotSelectType_p(int index);
    void slotCalibCoeffChecked(bool checked);

private Q_SLOTS:

    void on_pushButton_calc_clicked(bool checked);
    void on_pushButton_clear_clicked(bool checked);
};
#endif // MAINWINDOW_H
