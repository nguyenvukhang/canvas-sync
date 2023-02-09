#ifndef CANVAS_SYNC_PUSHBUTTON_H
#define CANVAS_SYNC_PUSHBUTTON_H

#include <QPushButton>

namespace Ui
{
class PushButton;
}

class PushButton : public QPushButton
{
  Q_OBJECT

public:
  explicit PushButton(QWidget *parent = nullptr);

private:
  Ui::PushButton *ui;
};

#endif
