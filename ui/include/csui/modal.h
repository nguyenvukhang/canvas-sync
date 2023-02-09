#ifndef CANVAS_SYNC_MODAL_H
#define CANVAS_SYNC_MODAL_H

#include <QWidget>

namespace Ui
{
class Modal;
}

class Modal : public QWidget
{
  Q_OBJECT

public:
  explicit Modal(QWidget *parent = nullptr);
  ~Modal();

  void add_line(const QString &);
  void setText(const QString &);

private:
  Ui::Modal *ui;
};

#endif
