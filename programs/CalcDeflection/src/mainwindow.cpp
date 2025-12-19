#include <QtCore>
#include <QtGui>
#include <QInputDialog>
#include <QMessageBox>
#include <QtSvg>
#include <cmath>
#include <chrono>

#include "mainwindow.h"
#include "infoDialog.h"
#include "../ui/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("CalcDeflection");

    // Инициализация сигналов и слотов
    // Настройка шрифта
    connect(ui->actionFormatTextSize, SIGNAL(triggered()), this, SLOT(slotFormatTextSize()));
    connect(ui->actionFormatTextBold, SIGNAL(toggled(bool)), this, SLOT(slotFormatTextBold(bool)));
    connect(ui->actionFormatTextItalic, SIGNAL(toggled(bool)), this, SLOT(slotFormatTextItalic(bool)));
    ui->actionFormatTextBold->setIcon(SvgConvPixmap(":images/format-text-bold-symbolic.svg", 256, 256));
    ui->actionFormatTextItalic->setIcon(SvgConvPixmap(":images/format-text-italic-symbolic.svg", 256, 256));
	
    // Выбор направления приложения силы
    connect(ui->comboBox_directionForce, SIGNAL(activated(int)), this, SLOT(slotDirectionForce(int)));
    ui->comboBox_directionForce->addItem("по оси x");
    ui->comboBox_directionForce->addItem("по оси y");
    // Выбор сечения поводка
    connect(ui->comboBox_selectType_p, SIGNAL(activated(int)), this, SLOT(slotSelectType_p(int)));
    ui->comboBox_selectType_p->addItem("прямоугольная");
    ui->comboBox_selectType_p->addItem("круглая");
    // Учёт калибровки
    connect(ui->checkBox_calibCoeff, SIGNAL(toggled(bool)), this, SLOT(slotCalibCoeffChecked(bool)));
    // Шага интегрирования
    connect(ui->checkBox_integStep, SIGNAL(toggled(bool)), this, SLOT(slotIntegStepChecked(bool)));

    // Информация о программе и алгоритм расчёта
    connect(ui->actionHelp, SIGNAL(triggered()), this, SLOT(slotHelp()));
    connect(ui->actionProgInfo, SIGNAL(triggered()), this, SLOT(slotProgInfo()));
    ui->actionHelp->setIcon(SvgConvPixmap(":images/menu_help.svg", 256, 256));
    ui->actionProgInfo->setIcon(SvgConvPixmap(":images/menu_info.svg", 256, 256));

    // Загрузка настроек
    settings_CalcDeflection = new QSettings("settings.conf",QSettings::IniFormat);
    loadSettings();
	
	// Глобальные настройка шрифта
	//this->setFont(QFont("Arial", 10, QFont::Bold, true));
}

// Шрифта утолщённый
void MainWindow::slotFormatTextBold(bool checked) {
    text.formatTextBold = checked;
    setFormatText();
}
// Шрифта курсив
void MainWindow::slotFormatTextItalic(bool checked) {
    text.formatTextItalic = checked;
    setFormatText();
}
// Задать размер шрифта
void MainWindow::slotFormatTextSize(){
	bool ok{};
    const QStringList fontSizes{"8", "9", "10", "11", "12",
                                "13", "14", "15", "16", "17",
                                "18", "19", "20"};
    QString fontSize = QInputDialog::getItem(this, "Размер шрифта", "Размер шрифта: ", fontSizes, 1, false, &ok);
	if(ok) {
        text.formatTextSize = QVariant(fontSize).toInt();
        ui->actionFormatTextSize->setText("Размер "+fontSize);
		setFormatText();
	}
}
// Задать шрифт
void MainWindow::setFormatText() {
    QFont mainFont;
    mainFont.setFamily(mainFont.defaultFamily()); // задание системного шрифта по умолчанию
    mainFont.setPointSize(text.formatTextSize); // размер шрифта в px
    mainFont.setBold(text.formatTextBold); // утолщённый
    mainFont.setItalic(text.formatTextItalic); // курсив
    this->setFont(mainFont);
}

// Задать шаг интегрирования
void MainWindow::slotIntegStepChecked(bool checked) {
    if(checked) {
        ui->spinBox_integStep->setEnabled(true);
    } else {
        ui->spinBox_integStep->setEnabled(false);
    }
}

// Задать коэффициенты калибровки
void MainWindow::slotCalibCoeffChecked(bool checked) {
    if(checked) {
        ui->doubleSpinBox_coeff_k->setEnabled(true);
		ui->doubleSpinBox_coeff_k->setVisible(true);
        ui->doubleSpinBox_coeff_b->setEnabled(true);
		ui->doubleSpinBox_coeff_b->setVisible(true);
        ui->label_coeff_k->setEnabled(true);
		ui->label_coeff_k->setVisible(true);
        ui->label_coeff_b->setEnabled(true);
		ui->label_coeff_b->setVisible(true);
    } else {
        ui->doubleSpinBox_coeff_k->setEnabled(false);
		ui->doubleSpinBox_coeff_k->setVisible(false);
        ui->doubleSpinBox_coeff_b->setEnabled(false);
		ui->doubleSpinBox_coeff_b->setVisible(false);
        ui->label_coeff_k->setEnabled(false);
		ui->label_coeff_k->setVisible(false);
        ui->label_coeff_b->setEnabled(false);
		ui->label_coeff_b->setVisible(false);
    }
}

// Выбор чертежа в зависимости от
// направления приложения силы и формы сечения поводка
void MainWindow::setDrawing() {
    int direction_force = ui->comboBox_directionForce->currentIndex();
    int leash_section = ui->comboBox_selectType_p->currentIndex();
    QString nameDrawing = "";
    if (!direction_force) {
        if (!leash_section) nameDrawing = ":/drawing/sampleRadiusWorkingPart_rectangularLeash_force_X-axis.svg";
        else nameDrawing = ":/drawing/sampleRadiusWorkingPart_circularLeash_force_X-axis.svg";
    } else {
        if (!leash_section) nameDrawing = ":/drawing/sampleRadiusWorkingPart_rectangularLeash_force_Y-axis.svg";
        else nameDrawing = ":/drawing/sampleRadiusWorkingPart_circularLeash_force_Y-axis.svg";
    }
    // Загрузка чертежа
    ui->label_plan->setPixmap(SvgConvPixmap(nameDrawing, 400, 555));
}

// Конвертор svg в QPixmap
QPixmap MainWindow::SvgConvPixmap(QString name, int size_x, int size_y) {
	QPixmap pixImage(size_x, size_y);
	pixImage.fill(Qt::transparent); // прозрачный фон
	
	// Конвертирование
	QSvgRenderer renderer(name);
	QPainter painter(&pixImage);
	renderer.render(&painter);
    return pixImage;
}

// Выбор направления приложения силы
void MainWindow::slotDirectionForce(int index) {
    QPixmap pixItem;
    if(!index) {
        // Чертёжи для силы приложенной по оси x
        pixItem.load(":/images/drawingSampleAndLever.png");
        ui->label_plan->setPixmap(pixItem);
    }
    else {
        // Чертёжи для силы приложенной по оси y
        pixItem.load(":/images/drawingSampleAndLever.png");
        ui->label_plan->setPixmap(pixItem);
    }
    setDrawing();
}

// Выбор прямоугольного или круглого сечения поводка
void MainWindow::slotSelectType_p(int index){
    if(index == 0) {
        // Парамет Dp делаем не активным и не видимым
        ui->doubleSpinBox_D_p->setEnabled(false);
        ui->doubleSpinBox_D_p->setVisible(false);
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
        ui->doubleSpinBox_D_p->setEnabled(true);
        ui->doubleSpinBox_D_p->setVisible(true);
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
    setDrawing();
}

void MainWindow::loadSettings(){
    // Параметры окна
    this->restoreGeometry(settings_CalcDeflection->value("settings/mainWindowGeometry").toByteArray());
    this->restoreState(settings_CalcDeflection->value("settings/mainWindowState").toByteArray());
    // Параметры шрифта
    text.formatTextSize = settings_CalcDeflection->value("settings/font/fontSize").value<int>();
    text.formatTextBold = settings_CalcDeflection->value("settings/font/fontBold").value<bool>();
    text.formatTextItalic = settings_CalcDeflection->value("settings/font/fontItalic").value<bool>();
    ui->actionFormatTextSize->setText("Размер "+QString::number(text.formatTextSize));
    ui->actionFormatTextBold->setChecked(text.formatTextBold);
    ui->actionFormatTextItalic->setChecked(text.formatTextItalic);
    setFormatText();

    // Параметры образца
    ui->comboBox_directionForce->setCurrentIndex(settings_CalcDeflection->value("settings/dir_force").value<int>()); // направление силы (по оси y или x)
    ui->doubleSpinBox_L_s->setValue(settings_CalcDeflection->value("settings/L_s").value<double>()); // длина рабочей части [мм]
    ui->doubleSpinBox_H_s->setValue(settings_CalcDeflection->value("settings/H_s").value<double>()); // ширина образца [мм]
    ui->doubleSpinBox_b_s->setValue(settings_CalcDeflection->value("settings/b_s").value<double>()); // толщина образца [мм]
    ui->doubleSpinBox_h_s->setValue(settings_CalcDeflection->value("settings/hs").value<double>()); // ширина рабочей части [мм]
    ui->doubleSpinBox_E1->setValue(settings_CalcDeflection->value("settings/E1").value<double>()); // модуль Юнга образца [ГПа]

    // Параметры поводка
    ui->comboBox_selectType_p->setCurrentIndex(settings_CalcDeflection->value("settings/type_p").value<int>()); // сечение поводка (прямоугольное или круглое)
    slotSelectType_p(settings_CalcDeflection->value("settings/type_p").value<int>());
    ui->doubleSpinBox_L_p->setValue(settings_CalcDeflection->value("settings/L_p").value<double>()); // длина поводка [мм]
    ui->doubleSpinBox_a_p->setValue(settings_CalcDeflection->value("settings/a_p").value<double>()); // толщина поводка [мм] (прямоугольное сечение)
    ui->doubleSpinBox_b_p->setValue(settings_CalcDeflection->value("settings/b_p").value<double>()); // высота поводка [мм] (прямоугольное сечение)
    ui->doubleSpinBox_D_p->setValue(settings_CalcDeflection->value("settings/D_p").value<double>()); // диаметр поводка [мм] (круголое сечение)
    ui->doubleSpinBox_E2->setValue(settings_CalcDeflection->value("settings/E2").value<double>()); // модуль Юнга поводка из титана [ГПа]

    // Результаты калибровки и параметры испытания
    ui->doubleSpinBox_coeff_k->setValue(settings_CalcDeflection->value("settings/coeff_k").value<double>()); // чувствительность или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    ui->doubleSpinBox_coeff_b->setValue(settings_CalcDeflection->value("settings/coeff_b").value<double>()); // смещение или коэффициент b при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    ui->doubleSpinBox_stress->setValue(settings_CalcDeflection->value("settings/stress").value<double>()); // напряжение на поверхности образца в эксперименте [МПа]
	
    // Дополнительные параметры
    ui->checkBox_force1N->setChecked(settings_CalcDeflection->value("settings/cb_force1N").value<bool>()); // выводить данные прогиба при силе 1 Н
    ui->checkBox_calcTime->setChecked(settings_CalcDeflection->value("settings/cb_calcTime").value<bool>()); // показать время расчёта
    ui->checkBox_calibCoeff->setChecked(settings_CalcDeflection->value("settings/cb_calibCoeff").value<bool>()); // вывести коэффициенты калибровки
    ui->checkBox_integStep->setChecked(settings_CalcDeflection->value("settings/cb_integStep").value<bool>()); // задать шаг интегрирования
    slotIntegStepChecked(settings_CalcDeflection->value("settings/cb_integStep").value<bool>());
	ui->spinBox_integStep->setValue(settings_CalcDeflection->value("settings/integStep").value<int>()); // шаг интегрирования по умолчанию L/1.0e+6
	slotCalibCoeffChecked(settings_CalcDeflection->value("settings/cb_calibCoeff").value<bool>());

    // Выбор чертежа подвизной системы
    setDrawing();
}

void MainWindow::saveSettings(){
    // Параметры окна
    settings_CalcDeflection->setValue("settings/mainWindowGeometry", saveGeometry());
    settings_CalcDeflection->setValue("settings/mainWindowState", saveState());
    // Параметры шрифта
    settings_CalcDeflection->setValue("settings/font/fontSize", text.formatTextSize);
    settings_CalcDeflection->setValue("settings/font/fontBold", text.formatTextBold);
    settings_CalcDeflection->setValue("settings/font/fontItalic", text.formatTextItalic);

    // Параметры образца
    settings_CalcDeflection->setValue("settings/dir_force", ui->comboBox_directionForce->currentIndex()); // направление силы (по оси y или x)
    settings_CalcDeflection->setValue("settings/L_s", ui->doubleSpinBox_L_s->value()); // длина рабочей части [мм]
    settings_CalcDeflection->setValue("settings/H_s", ui->doubleSpinBox_H_s->value()); // ширина образца [мм]
    settings_CalcDeflection->setValue("settings/b_s", ui->doubleSpinBox_b_s->value()); // толщина образца [мм]
    settings_CalcDeflection->setValue("settings/hs", ui->doubleSpinBox_h_s->value()); // ширина рабочей части [мм]
    settings_CalcDeflection->setValue("settings/E1", ui->doubleSpinBox_E1->value()); // модуль Юнга образца [ГПа]

    // Параметры поводка
    settings_CalcDeflection->setValue("settings/type_p", ui->comboBox_selectType_p->currentIndex()); // сечение поводка (прямоугольное или круглое)
    settings_CalcDeflection->setValue("settings/L_p", ui->doubleSpinBox_L_p->value()); // длина поводка [мм]
    settings_CalcDeflection->setValue("settings/a_p", ui->doubleSpinBox_a_p->value()); // толщина поводка [мм] (прямоугольное сечение)
    settings_CalcDeflection->setValue("settings/b_p", ui->doubleSpinBox_b_p->value()); // высота поводка [мм] (прямоугольное сечение)
    settings_CalcDeflection->setValue("settings/D_p", ui->doubleSpinBox_D_p->value()); // диаметр поводка [мм] (круголое сечение)
    settings_CalcDeflection->setValue("settings/E2", ui->doubleSpinBox_E2->value()); // модуль Юнга поводка из титана [ГПа]

    // Результаты калибровки и параметры испытания
    settings_CalcDeflection->setValue("settings/coeff_k", ui->doubleSpinBox_coeff_k->value()); // чувствительность или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    settings_CalcDeflection->setValue("settings/coeff_b", ui->doubleSpinBox_coeff_b->value()); // смещение или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    settings_CalcDeflection->setValue("settings/stress", ui->doubleSpinBox_stress->value()); // напряжение на поверхности образца в эксперименте [МПа]

    // Дополнительные параметры
    settings_CalcDeflection->setValue("settings/cb_force1N", ui->checkBox_force1N->isChecked()); // выводить данные прогиба при силе 1 Н
    settings_CalcDeflection->setValue("settings/cb_calcTime", ui->checkBox_calcTime->isChecked()); // показать время расчёта
    settings_CalcDeflection->setValue("settings/cb_calibCoeff", ui->checkBox_calibCoeff->isChecked()); // вывести коэффициенты калибровки
    settings_CalcDeflection->setValue("settings/cb_integStep", ui->checkBox_integStep->isChecked()); // задать шаг интегрирования
    settings_CalcDeflection->setValue("settings/integStep", ui->spinBox_integStep->value()); // шаг интегрирования по умолчанию L/1.0e+6
}

void MainWindow::on_pushButton_calc_clicked(bool checked){
    // Время начала отсчёта
    auto start_time_us = std::chrono::high_resolution_clock::now();

    // Номер расчёта
    calcResultId++;
    int direction_force = ui->comboBox_directionForce->currentIndex();
    int leash_section = ui->comboBox_selectType_p->currentIndex();

    // Параметры образца
    double L_s = ui->doubleSpinBox_L_s->value()/1000.0; // длина рабочей части [м]
    double H_s = ui->doubleSpinBox_H_s->value()/1000.0; // ширина образца [м]
    double b_s = ui->doubleSpinBox_b_s->value()/1000.0; // толщина образца [м]
    double hs = ui->doubleSpinBox_h_s->value()/1000.0; // ширина рабочей части [м]
    double E_1 = ui->doubleSpinBox_E1->value()*1.0e+9; // модуль Юнга образца [Па]

    // Обработка ошибок ввода данных
    if(H_s-hs <= 0) {
        ui->textBrowser->append("Ошибка H_s < h_s");
        return;
    }
    double R_s = (L_s*L_s+(H_s-hs)*(H_s-hs))/(4.0*(H_s-hs)); // радиус образца [м]
    double z_0 = L_s/2.0; // расстояние от центра радиуса до 0 по оси z [м]
    double y_0 = R_s + hs/2.0; // расстояние от центра радиуса до 0 по оси y [м]

    // Параметры поводка
    double L_p = ui->doubleSpinBox_L_p->value()/1000.0;  // длина поводка [м]
    double a_p = ui->doubleSpinBox_a_p->value()/1000.0; // толщина прямоугольного поводка [м]
    double b_p = ui->doubleSpinBox_b_p->value()/1000.0; // ширина прямоугольного поводка [м]
    double D_p = ui->doubleSpinBox_D_p->value()/1000.0; // диаметр круглого поводка [м]
    double E_2 = ui->doubleSpinBox_E2->value()*1.0e+9; // модуль Юнга поводка из титана [Па]
    double P_force = 1.0; // сила приложенная к концу поводка [H] (используется в расчёте и измерять её не нужно)
    double L_sp = L_s + L_p; // суммарная длина поводка и образца [м]

    // Параметры интегрирования
    double dz = L_sp/pow(10.0,ui->spinBox_integStep->value()); // шаг интегрирования (по умолчанию L/1.0e+6)
    double W_fi = 0.0; // промежуточный результат интегрирования
    double W_tau = 0.0; // максимальное отклонение [м]
    double Szmax = 0.0; // максимальное мех. напряжение на поверхности рабочей части образца при приложении силы P [Па]
    double I2_z = 0.0; // момент инерции сечения поводка L_s<=z<=L_sp (поводок прямоугольного сечения)
    if(!leash_section) {
        if(!direction_force) I2_z = b_p*b_p*b_p*a_p/12.0; // сила направлена по оси х
        else I2_z = a_p*a_p*a_p*b_p/12.0; // сила направлена по оси y
    }
    else I2_z = M_PI*D_p*D_p*D_p*D_p/64.0; // момент инерции сечения поводка L_s<=z<=L_sp (поводок круглого сечения)

    double I1_z_const = 0.0; // момент инерции сечения образца
    if (!direction_force) I1_z_const = b_s*b_s*b_s/6.0; // смещение перпендикулярно ширине образца (сила по оси x)
    else I1_z_const = 2.0*b_s/3.0; // смещение перпендикулярно радиусу образца (сила по оси y)
    double R_square = R_s*R_s;
    double P_E2_I2z_const = P_force/(E_2*I2_z);
    double z = 0.0;
    double z_max = 0.0;
    while (z < L_sp+dz) {
        if (z < L_s) {
            double y = y_0-sqrt(R_square-(z-z_0)*(z-z_0));
            double I1_z = 0.0; // момент инерции сечения образца
            double Sz = 0.0; // мех. напряжение в рабочей области образца
            if (!direction_force) {
                I1_z = I1_z_const*y; // сила направлена по оси х
                Sz = b_s*P_force*(L_sp-z)/(I1_z*2.0); // сила направлена по оси х
            }
            else {
                I1_z = I1_z_const*y*y*y; // сила направлена по оси y
                Sz = y*P_force*(L_sp-z)/I1_z; // сила направлена по оси y
            }
            if (Sz > Szmax) {
                Szmax = Sz; // нахождение максимального мех. напряжения
                z_max = z; // расстояние до Szmax по оси z
            }

            W_fi += dz*P_force*(L_sp-z)/(E_1*I1_z);
            W_tau += W_fi*dz;
        } else {
            W_fi += dz*P_E2_I2z_const*(L_sp-z);
            W_tau += W_fi*dz;
        }
        z += dz;
    }
    // Результаты калибровки и параметры испытания
    double sens = ui->doubleSpinBox_coeff_k->value(); // чувствительность или коэффициент k при аппроксимации калибровочной зависимости прямой [ед.ацп/мкм]
    double Sigma = ui->doubleSpinBox_stress->value()*1e+6; // напряжение на поверхности образца в эксперименте [Па]
    double deflection = W_tau*Sigma/Szmax; // прогиб образца [м]

    // Размеры и модуль Юнга образца
    ui->textBrowser->append("ОБРАЗЕЦ (РАСЧЁТ №"+QString::number(calcResultId)+")");
    ui->textBrowser->append("Длина рабочей части L_s = "+QString::number(L_s*1000.0)+" мм," );
    ui->textBrowser->append("Ширина образца H_s = "+QString::number(H_s*1000.0)+" мм," );
    ui->textBrowser->append("Толщина образца b_s = "+QString::number(b_s*1000.0)+" мм," );
    ui->textBrowser->append("Ширина рабочей части h_s = "+QString::number(hs*1000.0)+" мм," );
    ui->textBrowser->append("Модуль Юнга образца E1 = "+QString::number(E_1/1.0e+9)+" ГПа\n" );

    // Размеры и модуль Юнга поводка
    ui->textBrowser->append("ПОВОДОК (РАСЧЁТ №"+QString::number(calcResultId)+")");
    ui->textBrowser->append("Длина поводка L_p = "+QString::number(L_p*1000.0)+" мм," );
    if (leash_section == 0) {
        ui->textBrowser->append("Толщина поводка a_p = "+QString::number(a_p*1000.0)+" мм," );
        ui->textBrowser->append("Высота поводка b_p = "+QString::number(b_p*1000.0)+" мм," );
    }
    if (leash_section == 1) {
        ui->textBrowser->append("Диаметр поводка D_p = "+QString::number(D_p*1000.0)+" мм," );
    }
    ui->textBrowser->append("Модуль Юнга поводка E2 = "+QString::number(E_2/1.0e+9)+" ГПа\n" );

    // Рассчитанные значения
    ui->textBrowser->append("РЕЗУЛЬТАТЫ РАСЧЁТА №"+QString::number(calcResultId));
    if(ui->checkBox_calibCoeff->isChecked()) {
        ui->textBrowser->append("Чувствительность = "+QString::number(sens)+" ед.ацп/мкм," );
    }
    if(ui->checkBox_force1N->isChecked()){
        ui->textBrowser->append("v tau = "+QString::number(W_tau*1000.0)+" мм при приложении силы P = "+QString::number(P_force)+" Н,");
        ui->textBrowser->append("σ max = "+QString::number(Szmax/1e+6)+" МПа при приложении силы P = "+QString::number(P_force)+" Н,");
        ui->textBrowser->append("z max = "+QString::number(z_max*1e+3)+" мм при приложении силы P = "+QString::number(P_force)+" Н,");
    }
    ui->textBrowser->append("Прогиб v при "+QString::number(Sigma/1e+6)+" МПа равен "+QString::number(deflection*1000)+" мм,");
    ui->textBrowser->append("Сила на конце поводка при прогибе v равна "+QString::number(P_force*Sigma/Szmax)+ " Н");

    // Время окончания отсчёта
    auto finish_time_us = std::chrono::high_resolution_clock::now()-start_time_us;
    if(ui->checkBox_calcTime->isChecked()) {
        ui->textBrowser->append("======================================");
        ui->textBrowser->append("Время расчёта: "+QString::number(
            std::chrono::duration_cast<std::chrono::microseconds>(finish_time_us).count()/1.0e+3,'f',3)+" мс");
    }
    ui->textBrowser->append("======================================\n");
}

// Очистить окно вывода с результатами расчёта
void MainWindow::on_pushButton_clear_clicked(bool checked) {
    ui->textBrowser->clear();
    calcResultId = 0;
}

// Вызов pdf файла с алгоритмом расчёта прогиба
void MainWindow::slotHelp() {
    QString fileName = QString(qApp->applicationDirPath()+"/doc/CalcDeflection_doc.pdf");
    if(!QFile::exists(fileName)){
        QMessageBox::warning(this, "Предупреждение",
                                 "Файл CalcDeflection_doc.pdf не найден!",
                                 QMessageBox::Ok);
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

// Вызов окна с информацией о программе
void MainWindow::slotProgInfo() {
    Dialog dialog;
    dialog.exec();
}

// Сохранение настроек при закрытии программы
void MainWindow::closeEvent(QCloseEvent *close) {
    saveSettings();
}

MainWindow::~MainWindow() {
    delete ui;
}
