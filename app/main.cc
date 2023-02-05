#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w(new Canvas("https://canvas.nus.edu.sg"));
  w.show();
  return a.exec();
}
