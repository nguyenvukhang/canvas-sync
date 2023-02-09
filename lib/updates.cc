#include "updates.h"

Updates::Updates(QWidget *parent) : QDialog(parent), ui(new Ui::Updates)
{
  ui->setupUi(this);
}

void Updates::setText(const QString &text)
{
  this->ui->label->setText(text);
}

Updates::~Updates()
{
  delete ui;
}
