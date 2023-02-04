#include "main.h"

QTEST_MAIN(TestGui)

void TestGui::testGui()
{
  QLineEdit lineEdit;

  QTest::keyClicks(&lineEdit, "hello world");

  QCOMPARE(lineEdit.text(), QString("hello world"));
}
