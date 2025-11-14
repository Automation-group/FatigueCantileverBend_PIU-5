#include <QtGui>
#include <QSettings>
#include <QtCore>
#include <cmath>
#include <chrono>

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

    // Инициализация сигналов и слотов
    // Выбор направления приложения силы
    ui->comboBox_directionForce->addItem("по оси x");
    ui->comboBox_directionForce->addItem("по оси y");
    // Выбор сечения поводка
    connect(ui->comboBox_selectType_p, SIGNAL(activated(int)), this, SLOT(slotSelectType_p(int)));
    ui->comboBox_selectType_p->addItem("прямоугольная");
    ui->comboBox_selectType_p->addItem("круглая");
    // Учёт калибровки
    connect(ui->checkBox_calibCoeff, SIGNAL(toggled(bool)), this, SLOT(slotCalibCoeffChecked(bool)));

    // Загрузка настроек
    settings_CalcDeflection = new QSettings("settings.conf",QSettings::IniFormat);
    loadSettings();
}

void MainWindow::slotCalibCoeffChecked(bool checked) {
    if(checked){
        ui->doubleSpinBox_coeff_k->setEnabled(true);
        ui->doubleSpinBox_coeff_b->setEnabled(true);
        ui->label_coeff_k->setEnabled(true);
        ui->label_coeff_b->setEnabled(true);
    } else {
        ui->doubleSpinBox_coeff_k->setEnabled(false);
        ui->doubleSpinBox_coeff_b->setEnabled(false);
        ui->label_coeff_k->setEnabled(false);
        ui->label_coeff_b->setEnabled(false);
    }
}

void MainWindow::slotSelectType_p(int index){
    if(index == 0) {
        // Парамет Dp делаем не активным и не видимым
        ui->doubleSpinBox_Dp->setEnabled(false);
        ui->doubleSpinBox_Dp->setVisible(false);
        ui->label_Dp->setVisible(false);
        ui->label_Dp_mm->setVisible(false);
        // Парамет a_p делаем не активным и не видимым
        ui->doubleSpinBox_a_p->setEnabled(true);
        ui->doubleSpinBox_a_p->setVisible(true);
        ui->label_a_p->setVisible(true);
        ui->label_a_p_mm->setVisible(true);
        // Парамет b_p делаем не активным и не видимым
        ui->doubleSpinBox_b_p->setEnabled(true);
        ui->doubleSpinBox_b_p->setVisible(true);
        ui->label_b_p->setVisible(true);
        ui->label_b_p_mm->setVisible(true);
    }
    if(index == 1) {
        // Парамет Dp делаем активным и видимым
        ui->doubleSpinBox_Dp->setEnabled(true);
        ui->doubleSpinBox_Dp->setVisible(true);
        ui->label_Dp->setVisible(true);
        ui->label_Dp_mm->setVisible(true);
        // Парамет a_p делаем активным и видимым
        ui->doubleSpinBox_a_p->setEnabled(false);
        ui->doubleSpinBox_a_p->setVisible(false);
        ui->label_a_p->setVisible(false);
        ui->label_a_p_mm->setVisible(false);
        // Парамет b_p делаем активным и видимым
        ui->doubleSpinBox_b_p->setEnabled(false);
        ui->doubleSpinBox_b_p->setVisible(false);
        ui->label_b_p->setVisible(false);
        ui->label_b_p_mm->setVisible(false);
    }
}

void MainWindow::loadSettings(){
    // Параметры окна
    this->restoreGeometry(settings_CalcDeflection->value("settings/mainWindowGeometry").toByteArray());
    this->restoreState(settings_CalcDeflection->value("settings/mainWindowState").toByteArray());

    // Параметры образца
    ui->comboBox_directionForce->setCurrentIndex(settings_CalcDeflection->value("settings/dir_force").value<int>()); // направление силы (по оси y или x)
    ui->doubleSpinBox_Lp->setValue(settings_CalcDeflection->value("settings/Lp").value<double>()); // длина рабочей части [мм]
    ui->doubleSpinBox_H->setValue(settings_CalcDeflection->value("settings/H_p").value<double>()); // ширина образца [мм]
    ui->doubleSpinBox_b->setValue(settings_CalcDeflection->value("settings/b").value<double>()); // толщина образца [мм]
    ui->doubleSpinBox_h->setValue(settings_CalcDeflection->value("settings/h").value<double>()); // ширина рабочей части [мм]
    ui->doubleSpinBox_E1->setValue(settings_CalcDeflection->value("settings/E1").value<double>()); // модуль Юнга образца [ГПа]

    // Параметры поводка
    ui->comboBox_selectType_p->setCurrentIndex(settings_CalcDeflection->value("settings/type_p").value<int>()); // сечение поводка (прямоугольное или круглое)
    slotSelectType_p(settings_CalcDeflection->value("settings/type_p").value<int>());
    ui->doubleSpinBox_Lh->setValue(settings_CalcDeflection->value("settings/Lh").value<double>()); // длина поводка [мм]
    ui->doubleSpinBox_a_p->setValue(settings_CalcDeflection->value("settings/a_p").value<double>()); // толщина поводка [мм] (прямоугольное сечение)
    ui->doubleSpinBox_b_p->setValue(settings_CalcDeflection->value("settings/b_p").value<double>()); // высота поводка [мм] (прямоугольное сечение)
    ui->doubleSpinBox_Dp->setValue(settings_CalcDeflection->value("settings/Dp").value<double>()); // диаметр поводка [мм] (круголое сечение)
    ui->doubleSpinBox_E2->setValue(settings_CalcDeflection->value("settings/E2").value<double>()); // модуль Юнга поводка из титана [ГПа]

    // Результаты калибровки и параметры испытания
    ui->doubleSpinBox_coeff_k->setValue(settings_CalcDeflection->value("settings/coeff_k").value<double>()); // чувствительность или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    ui->doubleSpinBox_coeff_b->setValue(settings_CalcDeflection->value("settings/coeff_b").value<double>()); // смещение или коэффициент b при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    ui->doubleSpinBox_stress->setValue(settings_CalcDeflection->value("settings/stress").value<double>()); // напряжение на поверхности образца в эксперименте [МПа]

    // Дополнительные параметры
    ui->checkBox_force1N->setChecked(settings_CalcDeflection->value("settings/cb_force1N").value<bool>()); // выводить данные прогиба при силе 1 Н
    ui->checkBox_calcTime->setChecked(settings_CalcDeflection->value("settings/cb_calcTime").value<bool>()); // показать время расчёта
    ui->checkBox_calibCoeff->setChecked(settings_CalcDeflection->value("settings/cb_calibCoeff").value<bool>()); // вывести коэффициенты калибровки
}

void MainWindow::saveSettings(){
    // Параметры окна
    settings_CalcDeflection->setValue("settings/mainWindowGeometry", saveGeometry());
    settings_CalcDeflection->setValue("settings/mainWindowState", saveState());

    // Параметры образца
    settings_CalcDeflection->setValue("settings/dir_force", ui->comboBox_directionForce->currentIndex()); // направление силы (по оси y или x)
    settings_CalcDeflection->setValue("settings/Lp", ui->doubleSpinBox_Lp->value()); // длина рабочей части [мм]
    settings_CalcDeflection->setValue("settings/H_p", ui->doubleSpinBox_H->value()); // ширина образца [мм]
    settings_CalcDeflection->setValue("settings/b", ui->doubleSpinBox_b->value()); // толщина образца [мм]
    settings_CalcDeflection->setValue("settings/h", ui->doubleSpinBox_h->value()); // ширина рабочей части [мм]
    settings_CalcDeflection->setValue("settings/E1", ui->doubleSpinBox_E1->value()); // модуль Юнга образца [ГПа]

    // Параметры поводка
    settings_CalcDeflection->setValue("settings/type_p", ui->comboBox_selectType_p->currentIndex()); // сечение поводка (прямоугольное или круглое)
    settings_CalcDeflection->setValue("settings/Lh", ui->doubleSpinBox_Lh->value()); // длина поводка [мм]
    settings_CalcDeflection->setValue("settings/a_p", ui->doubleSpinBox_a_p->value()); // толщина поводка [мм] (прямоугольное сечение)
    settings_CalcDeflection->setValue("settings/b_p", ui->doubleSpinBox_b_p->value()); // высота поводка [мм] (прямоугольное сечение)
    settings_CalcDeflection->setValue("settings/Dp", ui->doubleSpinBox_Dp->value()); // диаметр поводка [мм] (круголое сечение)
    settings_CalcDeflection->setValue("settings/E2", ui->doubleSpinBox_E2->value()); // модуль Юнга поводка из титана [ГПа]

    // Результаты калибровки и параметры испытания
    settings_CalcDeflection->setValue("settings/coeff_k", ui->doubleSpinBox_coeff_k->value()); // чувствительность или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    settings_CalcDeflection->setValue("settings/coeff_b", ui->doubleSpinBox_coeff_b->value()); // смещение или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    settings_CalcDeflection->setValue("settings/stress", ui->doubleSpinBox_stress->value()); // напряжение на поверхности образца в эксперименте [МПа]

    // Дополнительные параметры
    settings_CalcDeflection->setValue("settings/cb_force1N", ui->checkBox_force1N->isChecked()); // выводить данные прогиба при силе 1 Н
    settings_CalcDeflection->setValue("settings/cb_calcTime", ui->checkBox_calcTime->isChecked()); // показать время расчёта
    settings_CalcDeflection->setValue("settings/cb_calibCoeff", ui->checkBox_calibCoeff->isChecked()); // вывести коэффициенты калибровки
}

void MainWindow::on_pushButton_calc_clicked(bool checked){
    // Время начала отсчёта
    auto start_time_us = std::chrono::high_resolution_clock::now();

    // Номер расчёта
    calcResultId++;
    int direction_force = ui->comboBox_directionForce->currentIndex();
    int leash_section = ui->comboBox_selectType_p->currentIndex();

    // Параметры образца
    double Lp = ui->doubleSpinBox_Lp->value()/1000.0; // длина рабочей части [м]
    double H = ui->doubleSpinBox_H->value()/1000.0; // ширина образца [м]
    double b = ui->doubleSpinBox_b->value()/1000.0; // толщина образца [м]
    double h = ui->doubleSpinBox_h->value()/1000.0; // ширина рабочей части [м]
    double E_1 = ui->doubleSpinBox_E1->value()*1.0e+9; // модуль Юнга образца [Па]

    double R = (Lp*Lp+(H-h)*(H-h))/(4.0*(H-h)); // радиус образца [м]
    double z_0 = Lp/2.0; // расстояние от центра радиуса до 0 по оси z [м]
    double y_0 = R + h/2.0; // расстояние от центра радиуса до 0 по оси y [м]

    // Параметры поводка
    double Lh = ui->doubleSpinBox_Lh->value()/1000.0;  // длина поводка [м]
    double a_p = ui->doubleSpinBox_a_p->value()/1000.0; // толщина прямоугольного поводка [м]
    double b_p = ui->doubleSpinBox_b_p->value()/1000.0; // высота прямоугольного поводка [м]
    double Dp = ui->doubleSpinBox_Dp->value()/1000.0; // диаметр круглого поводка [м]
    double E_2 = ui->doubleSpinBox_E2->value()*1.0e+9; // модуль Юнга поводка из титана [Па]
    double P = 1.0; // сила приложенная к концу поводка [H] (используется в расчёте и измерять её не нужно)
    double L = Lp + Lh; // суммарная длина поводка и образца [м]

    // Параметры интегрирования
    double dz = L/1.0e+6; // шаг интегрирования
    double W_fi = 0.0; // промежуточный результат интегрирования
    double W_tau = 0.0; // максимальное отклонение [м]
    double Wzmax = 0.0; // максимальное напряжение на поверхности рабочей части образца при приложении силы P [м]
    double I2_z = 0.0;
    if(leash_section == 0) I2_z = b_p*b_p*b_p*a_p/12.0; // момент инерции сечения поводка A<=z<=L (поводок прямоугольного сечения)
    if(leash_section == 1) I2_z = M_PI*Dp*Dp*Dp*Dp/64.0; // момент инерции сечения поводка A<=z<=L (поводок круглого сечения)

    double I1_z_const = 0.0;
    if (direction_force == 0) I1_z_const = 2.0*b*b*b/3.0; // смещение перпендикулярно ширине образца
    if (direction_force == 1) I1_z_const = 2.0*b/3.0; // смещение перпендикулярно радиусу образца
    double R_square = R*R;
    double P_E2_I2z_const = P/(E_2*I2_z);
    double z = 0.0;
    while (z < L+dz) {
        if (z < Lp) {
            double y = y_0-sqrt(R_square-(z-z_0)*(z-z_0));
            double I1_z = 0.0;
            if (direction_force == 0) I1_z = I1_z_const*y; // смещение перпендикулярно ширине образца
            if (direction_force == 1) I1_z = I1_z_const*y*y*y; // смещение перпендикулярно радиусу образца
            double Wz = y*P*(L-z)/(I1_z*1.0e+6);
            if (Wz > Wzmax) Wzmax = Wz;

            W_fi += dz*P*(L-z)/(E_1*I1_z);
            W_tau += W_fi*dz;
        } else {
            W_fi += dz*P_E2_I2z_const*(L-z);
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
    if (leash_section == 0) {
        ui->textBrowser->append("Толщина поводка a_p = "+QString::number(a_p*1000.0)+" мм," );
        ui->textBrowser->append("Высота поводка b_p = "+QString::number(b_p*1000.0)+" мм," );
    }
    if (leash_section == 1) {
        ui->textBrowser->append("Диаметр поводка Dp = "+QString::number(Dp*1000.0)+" мм," );
    }
    ui->textBrowser->append("Модуль Юнга поводка E2 = "+QString::number(E_2/1.0e+9)+" ГПа\n" );

    // Рассчитанные значения
    ui->textBrowser->append("РЕЗУЛЬТАТЫ РАСЧЁТА №"+QString::number(calcResultId));
    if(ui->checkBox_calibCoeff->isChecked()) {
        ui->textBrowser->append("Чувствительность = "+QString::number(sens)+" ед.ацп/мкм," );
    }
    if(ui->checkBox_force1N->isChecked()){
        ui->textBrowser->append("v max = "+QString::number(W_tau*1000.0)+" мм при приложении силы P = "+QString::number(P)+" Н,");
        ui->textBrowser->append("σ max = "+QString::number(Wzmax)+" МПа при приложении силы P = "+QString::number(P)+" Н,");
    }
    ui->textBrowser->append("Прогиб v при "+QString::number(stress/1e+6)+" МПа равен "+QString::number(deflection*1000)+" мм,");
    ui->textBrowser->append("Сила на конце поводка при прогибе v равна "+QString::number(P*stress/(Wzmax*1e+6))+ " Н");

    // Время окончания отсчёта
    auto finish_time_us = std::chrono::high_resolution_clock::now()-start_time_us;
    if(ui->checkBox_calcTime->isChecked()) {
        ui->textBrowser->append("==================================================");
        ui->textBrowser->append("Время расчёта: "+QString::number(
            std::chrono::duration_cast<std::chrono::microseconds>(finish_time_us).count()/1.0e+3,'f',3)+" мс");
    }
    ui->textBrowser->append("==================================================\n");
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
