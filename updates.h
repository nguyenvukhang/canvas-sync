#ifndef UPDATES_H
#define UPDATES_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui
{
class Updates;
}
QT_END_NAMESPACE

class Updates : public QDialog
{
  Q_OBJECT

public:
  Updates(QWidget *parent = nullptr);
  ~Updates();
  void setText(const QString &);

private:
  Ui::Updates *ui;
};
#endif // UPDATES_H
