#include "infoDialog.h"
#include "mainwindow.h"

Dialog::Dialog() {
  ui.setupUi(this);

  // Название окна
  this->setWindowTitle("О программе");

  // Зафиксировать размер окна
  this->setFixedSize(530, 370);

  // Иконка окна
  // OS Windows
  #if defined(Q_OS_WIN)
    this->setWindowIcon(QIcon(":images/icon.ico"));
  #endif
  // OS Linux
  #if defined(Q_OS_LINUX)
    this->setWindowIcon(QIcon(":images/icon.svg"));
  #endif

  // Логотип программы
  MainWindow pixconv;
  ui.label_image->setPixmap(pixconv.SvgConvPixmap(":images/iconLogo.svg", 256, 256));

  // Информация о версии программы
  ui.label_version->setText(QString("<strong>%1</strong>").arg("CalcDeflection 0.2"));
  ui.label_version->setAlignment(Qt::AlignCenter);

  // Информация об авторах программы
  ui.label_autors->setText(QString("<strong>%1</strong>").arg("Авторы программы"));
  ui.label_autors->setAlignment(Qt::AlignCenter);
  ui.textBrowser_autors->append(QString("<strong>%1</strong>").arg("Программирование"));
  ui.textBrowser_autors->append("Ветров Д.Н.\n");
  ui.textBrowser_autors->append(QString("<strong>%1</strong>").arg("Разработка алгоритма расчёта"));
  ui.textBrowser_autors->append("Cысоев А.Н.");
  ui.textBrowser_autors->append("Берендеев Н.Н.\n");
  ui.textBrowser_autors->append(QString("<strong>%1</strong>").arg("Изготовление прототипа"));
  ui.textBrowser_autors->append("Котков Д.Н.");

  // Информация об организации
  ui.label_data_company->setText(QString("<strong>%1</strong>").arg("НИФТИ ННГУ 2025"));
  ui.label_data_company->setAlignment(Qt::AlignCenter);

  // Информация о версии Qt
  ui.label_qtversion->setText(QString("<strong>%1</strong>").arg("Qt "+QVariant(qVersion()).toString()));
  ui.label_qtversion->setAlignment(Qt::AlignCenter);
}

void Dialog::on_pushButton_close_clicked(bool checked) {
    QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
}
