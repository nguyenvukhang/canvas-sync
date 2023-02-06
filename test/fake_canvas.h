#ifndef TEST_FAKE_CANVAS_H
#define TEST_FAKE_CANVAS_H

#include <QDebug>
#include <QSignalSpy>
#include <QTest>

#include "csync/canvas_sync.h"

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

#endif
