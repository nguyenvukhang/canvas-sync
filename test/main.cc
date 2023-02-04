#include "main.h"

QTEST_MAIN(TestGui)

void TestGui::testGui()
{
  QLineEdit lineEdit;

  QTest::keyClicks(ui->lineEdit_accessToken, "hello world");

  QCOMPARE(ui->lineEdit_accessToken->text(), QString("hello world"));
}
