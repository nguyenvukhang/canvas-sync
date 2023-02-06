#include "csync/canvas_sync.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w("https://canvas.nus.edu.sg");
  w.show();
  return a.exec();
}
