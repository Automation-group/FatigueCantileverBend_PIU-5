#include "infoDialog.h"
#include "mainwindow.h"

Dialog::Dialog() {
  ui.setupUi(this);

  // Название окна
  this->setWindowTitle("О программе");

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
  ui.label_version->setText("CalcDeflection 0.2");
  ui.label_version->setAlignment(Qt::AlignCenter);

  // Информация об авторах программы
  ui.label_autors->setText("Авторы программы:");
  ui.label_autors->setAlignment(Qt::AlignCenter);
  ui.label_autor_1->setText("Ветров Д.Н.");
  ui.label_autor_1->setAlignment(Qt::AlignCenter);
  ui.label_autor_2->setText("Сысоев А.Н.");
  ui.label_autor_2->setAlignment(Qt::AlignCenter);
  ui.label_autor_3->setText("Берендеев Н.Н.");
  ui.label_autor_3->setAlignment(Qt::AlignCenter);

  // Информация об организации
  ui.label_data_company->setText("НИФТИ ННГУ 2025");
  ui.label_data_company->setAlignment(Qt::AlignCenter);

  // Информация о версии Qt
  ui.label_qtversion->setText("Qt "+QVariant(qVersion()).toString());
  ui.label_qtversion->setAlignment(Qt::AlignCenter);
}

void Dialog::on_pushButton_close_clicked(bool checked) {
    QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
}
