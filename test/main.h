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

  void authenticate() override { emit authenticate_done(token() == "valid"); };
  void fetch_courses() override{};
  void fetch_folders(const Course &) override{};
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

private slots:
  void testGui();
};

#endif
