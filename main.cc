#include <canvas_sync.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  std::cout << "STARTING PRODCTION APP" << std::endl;
  MainWindow w(new Canvas("https://canvas.nus.edu.sg"));
  w.show();
  return a.exec();
}
