#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include <QDebug>
#include <QNetworkAccessManager>
#include <QSignalSpy>
#include <QTest>
#include <QtWidgets>

#include "csync/canvas_sync.h"

#include <algorithm>

class FakeCanvas : public ICanvas
{
  Q_OBJECT

public:
  FakeCanvas() : ICanvas(""){};

  void authenticate() override;
  void fetch_courses() override;
  void fetch_folders(const Course &course) override;
  void fetch_files(const Folder &) override{};
  void download(const File &, const Folder &) override{};
};

class TestGui : public QObject
{
  Q_OBJECT

public:
  TestGui() : app(new FakeCanvas(), "canvas-sync-test.ini"){};

  QNetworkAccessManager nw;
  MainWindow app;

  // convenience routines
  void authenticate();

private slots: // tests
  void access_token_entry_test();
  void fetch_courses_test();
  void fetch_courses_ui_test();
};

#endif
