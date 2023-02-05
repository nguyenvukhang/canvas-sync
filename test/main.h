#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include <QDebug>
#include <QNetworkAccessManager>
#include <QSignalSpy>
#include <QTest>
#include <QtWidgets>

#include <canvas_sync.h>

class FakeCanvas : public ICanvas
{
  Q_OBJECT

public:
  FakeCanvas() : ICanvas(""){};

  void authenticate() { emit authenticate_done(token() == "valid"); };
  void fetch_courses(){};
  void fetch_folders(const Course &){};
  void fetch_files(const Folder &){};
  void download(const File &, const Folder &){};
  void reset_counts(){};
  void set_total_fetches(size_t){};
  size_t increment_total_downloads(size_t) { return 0; };
  size_t increment_done_downloads() { return 0; };
  bool is_done_downloading() { return true; };
  bool has_downloads() { return true; };
};

class TestGui : public QObject
{
  Q_OBJECT

public:
  TestGui() : app(new FakeCanvas(), "canvas-sync-test.ini"){};

  QNetworkAccessManager nw;
  MainWindow app;

private slots:
  void testGui();
};

#endif
