#include <QtGui>
#include <QSettings>
#include <QtCore>
#include <cmath>
#include <ctime>

#include "mainwindow.h"
#include "../ui/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("CalcDeflection");

    // Загрузка иконок для кнопок и прочих изображений
    QPixmap pixItem;
    pixItem.load(":/images/drawingSampleAndLever.png");
    ui->label_plan->setPixmap(pixItem);

    // Загрузка настроек
    settings_CalcDeflection = new QSettings("settings.conf",QSettings::IniFormat);
    loadSettings();
}

void MainWindow::loadSettings(){
    // Параметры окна
    this->restoreGeometry(settings_CalcDeflection->value("settings/mainWindowGeometry").toByteArray());
    this->restoreState(settings_CalcDeflection->value("settings/mainWindowState").toByteArray());

    // Параметры образца
    ui->doubleSpinBox_Lp->setValue(settings_CalcDeflection->value("settings/Lp").value<double>()); // длина рабочей части [мм]
    ui->doubleSpinBox_H->setValue(settings_CalcDeflection->value("settings/H_p").value<double>()); // ширина образца [мм]
    ui->doubleSpinBox_b->setValue(settings_CalcDeflection->value("settings/b").value<double>()); // толщина образца [мм]
    ui->doubleSpinBox_h->setValue(settings_CalcDeflection->value("settings/h").value<double>()); // ширина рабочей части [мм]
    ui->doubleSpinBox_E1->setValue(settings_CalcDeflection->value("settings/E1").value<double>()); // модуль Юнга образца [ГПа]

    // Параметры поводка
    ui->doubleSpinBox_Lh->setValue(settings_CalcDeflection->value("settings/Lh").value<double>()); // длина поводка [мм]
    ui->doubleSpinBox_a_p->setValue(settings_CalcDeflection->value("settings/a_p").value<double>()); // толщина поводка [мм]
    ui->doubleSpinBox_b_p->setValue(settings_CalcDeflection->value("settings/b_p").value<double>()); // высота поводка [мм]
    ui->doubleSpinBox_E2->setValue(settings_CalcDeflection->value("settings/E2").value<double>()); // модуль Юнга поводка из титана [ГПа]

    // Результаты калибровки и параметры испытания
    ui->doubleSpinBox_coeff_k->setValue(settings_CalcDeflection->value("settings/coeff_k").value<double>()); // чувствительность или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    ui->doubleSpinBox_coeff_b->setValue(settings_CalcDeflection->value("settings/coeff_b").value<double>()); // смещение или коэффициент b при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    ui->doubleSpinBox_stress->setValue(settings_CalcDeflection->value("settings/stress").value<double>()); // напряжение на поверхности образца в эксперименте [МПа]
}

void MainWindow::saveSettings(){
    // Параметры окна
    settings_CalcDeflection->setValue("settings/mainWindowGeometry", saveGeometry());
    settings_CalcDeflection->setValue("settings/mainWindowState", saveState());

    // Параметры образца
    settings_CalcDeflection->setValue("settings/Lp", ui->doubleSpinBox_Lp->value()); // длина рабочей части [мм]
    settings_CalcDeflection->setValue("settings/H_p", ui->doubleSpinBox_H->value()); // ширина образца [мм]
    settings_CalcDeflection->setValue("settings/b", ui->doubleSpinBox_b->value()); // толщина образца [мм]
    settings_CalcDeflection->setValue("settings/h", ui->doubleSpinBox_h->value()); // ширина рабочей части [мм]
    settings_CalcDeflection->setValue("settings/E1", ui->doubleSpinBox_E1->value()); // модуль Юнга образца [ГПа]

    // Параметры поводка
    settings_CalcDeflection->setValue("settings/Lh", ui->doubleSpinBox_Lh->value()); // длина поводка [мм]
    settings_CalcDeflection->setValue("settings/a_p", ui->doubleSpinBox_a_p->value()); // толщина поводка [мм]
    settings_CalcDeflection->setValue("settings/b_p", ui->doubleSpinBox_b_p->value()); // высота поводка [мм]
    settings_CalcDeflection->setValue("settings/E2", ui->doubleSpinBox_E2->value()); // модуль Юнга поводка из титана [ГПа]

    // Результаты калибровки и параметры испытания
    settings_CalcDeflection->setValue("settings/coeff_k", ui->doubleSpinBox_coeff_k->value()); // чувствительность или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    settings_CalcDeflection->setValue("settings/coeff_b", ui->doubleSpinBox_coeff_b->value()); // смещение или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    settings_CalcDeflection->setValue("settings/stress", ui->doubleSpinBox_stress->value()); // напряжение на поверхности образца в эксперименте [МПа]

}

void MainWindow::on_pushButton_calc_clicked(bool checked){
    // Время начала программы
    double startCalc = clock();

    // Номер расчёта
    calcResultId++;

    // Параметры образца
    double Lp = ui->doubleSpinBox_Lp->value()/1000.0; // длина рабочей части [м]
    double H = ui->doubleSpinBox_H->value()/1000.0; // ширина образца [м]
    double b = ui->doubleSpinBox_b->value()/1000.0; // толщина образца [м]
    double h = ui->doubleSpinBox_h->value()/1000.0; // ширина рабочей части [м]
    double E_1 = ui->doubleSpinBox_E1->value()*1.0e+9; // модуль Юнга образца [Па]

    static double R = (Lp*Lp+(H-h)*(H-h))/(4.0*(H-h)); // радиус образца [м]
    static double z_0 = Lp/2.0; // расстояние от центра радиуса до 0 по оси z [м]
    static double y_0 = R + h/2.0; // расстояние от центра радиуса до 0 по оси y [м]

    // Параметры поводка
    double Lh = ui->doubleSpinBox_Lh->value()/1000.0;  // длина поводка [м]
    double a_p = ui->doubleSpinBox_a_p->value()/1000.0; // толщина поводка [м]
    double b_p = ui->doubleSpinBox_b_p->value()/1000.0; // высота поводка [м]

    // Перед типом double ко всем переменным ниже нужно добавить static
    // В этом случае время расчёта уменьшится в 10 раз
    double E_2 = ui->doubleSpinBox_E2->value()*1.0e+9; // модуль Юнга поводка из титана [Па]
    double P = 1.0; // сила приложенная к концу поводка [H] (используется в расчёте и измерять её не нужно)
    double L = Lp + Lh; // суммарная длина поводка и образца [м]

    // Параметры интегрирования
    double dz = L/1.0e+6; // шаг интегрирования
    double W_fi = 0.0; // промежуточный результат интегрирования
    double W_tau = 0.0; // максимальное отклонение [м]
    double Wzmax = 0.0; // максимальное напряжение на поверхности рабочей части образца при отклонееии S_tau [Па]
    double I2_z = b_p*b_p*b_p*a_p/12.0; // момент инерции сечения поводка A<=z<=L

    double I1_z_const = 2.0*b*b*b/3.0;
    double z=0.0;
    while (z < L+dz) {
        if (z < Lp) {
            double y = y_0-sqrt(R*R-(z-z_0)*(z-z_0));
            double I1_z = I1_z_const*y;
            double Wz = ((P*(L-z))/I1_z)*y/1.0e+6;
            if (Wz > Wzmax) Wzmax = Wz;

            W_fi += (P*(L-z)/(E_1*I1_z))*dz;
            W_tau += W_fi*dz;
        } else {
            W_fi += (P*(L-z)/(E_2*I2_z))*dz;
            W_tau += W_fi*dz;
        }
        z += dz;
    }
    // Результаты калибровки и параметры испытания
    double sens = ui->doubleSpinBox_coeff_k->value(); // чувствительность или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    double stress = ui->doubleSpinBox_stress->value()*1e+6; // напряжение на поверхности образца в эксперименте [МПа]
    double deflection = W_tau*stress/(Wzmax*1e+6); // прогиб образца [м]

    // Размеры и модуль Юнга образца
    ui->textBrowser->append("ОБРАЗЕЦ (РАСЧЁТ №"+QString::number(calcResultId)+")");
    ui->textBrowser->append("Длина рабочей части Lp = "+QString::number(Lp*1000.0)+" мм," );
    ui->textBrowser->append("Ширина образца H = "+QString::number(H*1000.0)+" мм," );
    ui->textBrowser->append("Толщина образца b = "+QString::number(b*1000.0)+" мм," );
    ui->textBrowser->append("Ширина рабочей части h = "+QString::number(h*1000.0)+" мм," );
    ui->textBrowser->append("Модуль Юнга образца E1 = "+QString::number(E_1/1.0e+9)+" ГПа\n" );

    // Размеры и модуль Юнга поводка
    ui->textBrowser->append("ПОВОДОК (РАСЧЁТ №"+QString::number(calcResultId)+")");
    ui->textBrowser->append("Длина поводка Lh = "+QString::number(Lh*1000.0)+" мм," );
    ui->textBrowser->append("Толщина поводка a_p = "+QString::number(a_p*1000.0)+" мм," );
    ui->textBrowser->append("Высота поводка b_p = "+QString::number(b_p*1000.0)+" мм," );
    ui->textBrowser->append("Модуль Юнга поводка E2 = "+QString::number(E_2/1.0e+9)+" ГПа\n" );

    // Рассчитанные значения
    ui->textBrowser->append("РЕЗУЛЬТАТЫ РАСЧЁТА №"+QString::number(calcResultId));
    ui->textBrowser->append("Чувствительность = "+QString::number(sens)+" ед.ацп/мкм," );
    ui->textBrowser->append("v max = "+QString::number(W_tau*1000.0)+" мм при приложении силы P = "+QString::number(P)+" Н,");
    ui->textBrowser->append("σ max = "+QString::number(Wzmax)+" МПа при приложении силы P = "+QString::number(P)+" Н,");
    ui->textBrowser->append("Прогиб v при "+QString::number(stress/1e+6)+" МПа равен "+QString::number(deflection*1000)+" мм,");
    ui->textBrowser->append("Сила на конце поводка при прогибе v равна "+QString::number(P*stress/(Wzmax*1e+6))+ " Н,");

    // Время окончания расчёта
    double finishCalc = clock();
    ui->textBrowser->append("Время расчёта: "+QString::number((finishCalc-startCalc)/CLOCKS_PER_SEC)+" сек.\n");
}

void MainWindow::on_pushButton_clear_clicked(bool checked) {
    ui->textBrowser->clear();
    calcResultId = 0;
}

void MainWindow::closeEvent(QCloseEvent *close) {
    saveSettings();
}

MainWindow::~MainWindow() {
    delete ui;
}
