#ifndef CANVAS_SYNC_ONESHOTBUTTON_H
#define CANVAS_SYNC_ONESHOTBUTTON_H

#include <QPushButton>

namespace Ui
{
class OneShotButton;
}

class OneShotButton : public QPushButton
{
  Q_OBJECT

  QString ready, running;

public:
  explicit OneShotButton(QWidget *parent = nullptr);

  void set_states(const QString &ready, const QString &running);
  void set_running();
  void set_ready();
  void enable() { setEnabled(true); };
  void disable() { setEnabled(false); };

private:
  Ui::OneShotButton *ui;
};

#endif
