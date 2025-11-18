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
    void setFormatText();
    // Формат шрифта по умолчанию
    struct defaultTextFormat{
        int formatTextSize = 9;
        bool formatTextBold = false;
        bool formatTextItalic = false;
    } text;

protected:
    void closeEvent(QCloseEvent *close);

protected slots:
    void slotSelectType_p(int index); // выбор типа поводка
    void slotCalibCoeffChecked(bool checked); // задать коэффициенты калибровки
    void slotIntegStepChecked(bool checked); // задать шаг интегрирования
    void slotFormatTextSize(); // размер шрифта
    void slotFormatTextBold(bool checked); // шрифт обычный/жирный
    void slotFormatTextItalic(bool checked); // шрифт обычный/курсив

private Q_SLOTS:

    void on_pushButton_calc_clicked(bool checked); // провести расчёт
    void on_pushButton_clear_clicked(bool checked); // очистить результаты расчёта
};
#endif // MAINWINDOW_H
