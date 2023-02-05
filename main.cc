#include "csync/canvas_sync.h"

int main(int argc, char *argv[])
{
  Profile p;
  p.id = 1000;
  std::cout << p.id << std::endl;
  return 0;
  // QApplication a(argc, argv);
  // std::cout << "STARTING PRODCTION APP" << std::endl;
  // MainWindow w(new Canvas("https://canvas.nus.edu.sg"));
  // w.show();
  // return a.exec();
}
