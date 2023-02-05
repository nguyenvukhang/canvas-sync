#include "main.h"

QTEST_MAIN(TestGui)

void TestGui::testGui()
{
  QSignalSpy spy(app.canvas, &ICanvas::authenticate_done);
  // click on "Change Token"
  QTest::mouseClick(app.ui->pushButton_changeToken,
                    Qt::MouseButton::LeftButton);
  // enter a random token
  QTest::keyClicks(app.ui->lineEdit_accessToken, "valid");
  QCOMPARE(spy.count(), 5);
  // ensure that text changed
  QCOMPARE(app.ui->label_authenticationStatus->text(), "authenticated!");
}
