#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include <QDebug>
#include <QSignalSpy>
#include <QTest>
#include <QtWidgets>

#include "csync/canvas_sync.h"
#include "fake_canvas.h"

#include <algorithm>

class TestGui : public QObject
{
  Q_OBJECT

public:
  TestGui() : app("", "canvas-sync-test.ini"){};

  MainWindow app;

  // convenience routines
  void authenticate();

private slots: // tests
  void access_token_entry_test();
  void fetch_courses_test();
  void fetch_courses_ui_test();
};

#endif
