#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include <QDebug>
#include <QSignalSpy>
#include <QTest>
#include <QtWidgets>

#include "csync/canvas_sync.h"
#include "fake_canvas.h"

#include <algorithm>
#include <filesystem>
#include <mutex>
#include <vector>

class TestGui : public QObject
{
  Q_OBJECT
  int id = 1;
  std::mutex id_mtx;
  std::vector<QString> tmp_settings;

public:
  TestGui() : app(new FakeCanvas(), "canvas-sync-test.ini"){};
  MainWindow app;

  // convenience routines
  void authenticate();

private slots: // tests
  void access_token_entry_test();
  void fetch_courses_test();
  void fetch_courses_ui_test();
  void cleanupTestCase()
  {
    this->id_mtx.lock();
    for (auto i : this->tmp_settings)
      std::filesystem::remove(i.toStdString());
    this->id_mtx.unlock();
  };
};

#endif
