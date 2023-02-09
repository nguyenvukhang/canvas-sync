#include "csui/modal.h"
#include "ui_modal.h"

Modal::Modal(QWidget *parent) : QWidget(parent), ui(new Ui::Modal)
{
  ui->setupUi(this);
  ui->text->clear();
}

Modal::~Modal()
{
  delete ui;
}

void Modal::add_line(const QString &line)
{
  ui->text->setText(ui->text->text() + '\n' + '\n' + line);
}

void Modal::setText(const QString &t)
{
  ui->text->setText(t);
}
