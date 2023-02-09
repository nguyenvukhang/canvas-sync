#include "app.h"
#include <csync/csync.h>

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  App w("https://canvas.nus.edu.sg");
  w.show();
  return a.exec();
}
