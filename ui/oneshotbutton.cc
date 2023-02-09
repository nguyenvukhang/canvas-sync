#include "csui/oneshotbutton.h"

OneShotButton::OneShotButton(QWidget *parent) : QPushButton(parent)
{
  connect(this, &OneShotButton::clicked, this, &OneShotButton::set_running);
};

void OneShotButton::set_states(const QString &ready, const QString &running)
{
  this->ready = ready;
  this->running = running;
}

void OneShotButton::set_running()
{
  disable();
  setText(this->running);
}

void OneShotButton::set_ready()
{
  enable();
  setText(this->ready);
}
