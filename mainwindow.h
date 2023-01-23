#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <csrv.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_pushButton_pull_clicked();
  void on_lineEdit_accessToken_textChanged(const QString &arg1);

private:
  QString token;
  Ui::MainWindow *ui;
  canvas::Server server;
};
#endif // MAINWINDOW_H
