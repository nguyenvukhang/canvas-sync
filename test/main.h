#include <QNetworkAccessManager>
#include <QTest>
#include <QtWidgets>

#include <canvas_sync.h>

class TestGui : public QObject
{
  Q_OBJECT

  QNetworkAccessManager nw;
  // MainWindow app = new MainWindow(&this->nw);

private slots:
  void testGui();
};
