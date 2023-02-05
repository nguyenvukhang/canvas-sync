#include "main.h"

QTEST_MAIN(TestGui)

void TestGui::testGui()
{
  QSignalSpy auth_tries(app.canvas, &ICanvas::authenticate_done);

  QPushButton *change_token = app.ui->pushButton_changeToken;

  // click on "Change Token"
  QTest::mouseClick(change_token, Qt::MouseButton::LeftButton);

  // "Change Token" should be disabled and hidden
  QVERIFY(!change_token->isEnabled());
  QVERIFY(change_token->isHidden());

  // enter a valid token
  QTest::keyClicks(app.ui->lineEdit_accessToken, "valid");

  QCOMPARE(auth_tries.count(), 5);

  QCOMPARE(app.ui->label_authenticationStatus->text(), "authenticated!");
  QVERIFY(change_token->isEnabled());
  QVERIFY(!change_token->isHidden());
}
