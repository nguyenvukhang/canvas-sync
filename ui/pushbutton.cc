#include "csui/pushbutton.h"
#include "ui_pushbutton.h"

PushButton::PushButton(QWidget *parent) : QPushButton(parent)
{
  ui->setupUi(this);
  connect(this, &PushButton::clicked, this, []() { qDebug() << "PUSHED!"; });
};
