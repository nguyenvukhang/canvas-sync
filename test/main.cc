#include "main.h"

QTEST_MAIN(TestGui)

void TestGui::authenticate(MainWindow *app)
{
  QPushButton *change_token = app->ui->pushButton_changeToken;
  QTest::mouseClick(change_token, Qt::MouseButton::LeftButton);
  QTest::keyClicks(app->ui->lineEdit_accessToken, "valid");
}

void TestGui::access_token_entry_test()
{
  MainWindow *app = create_app();
  QSignalSpy auth_tries(app->canvas, &ICanvas::authenticate_done);

  QPushButton *change_token = app->ui->pushButton_changeToken;

  // click on "Change Token"
  QTest::mouseClick(change_token, Qt::MouseButton::LeftButton);

  // "Change Token" should be disabled and hidden
  QVERIFY(!change_token->isEnabled());
  QVERIFY(change_token->isHidden());

  // enter a valid token
  QTest::keyClicks(app->ui->lineEdit_accessToken, "valid");

  QCOMPARE(auth_tries.count(), 5);

  QCOMPARE(app->ui->label_authenticationStatus->text(), "authenticated!");
  QVERIFY(change_token->isEnabled());
  QVERIFY(!change_token->isHidden());
}

void TestGui::fetch_courses_test()
{
  MainWindow *app = create_app();
  authenticate(app);
  std::string expected[] = {
      "#0|Linear Algebra: {\n"
      "  #20010|course files: {\n"
      "    #20011|Lectures: {}\n"
      "    #20012|Tutorials: {}\n"
      "  }\n"
      "}",
      "#1|Calculus: {\n"
      "  #20020|course files: {\n"
      "    #20021|Admin: {}\n"
      "    #20022|Tutorials: {}\n"
      "  }\n"
      "}",
      "#2|Geometry: {\n"
      "  #22190|course files: {\n"
      "    #22192|Course notes: {}\n"
      "    #22191|Lectures: {}\n"
      "    #22193|Tutorials: {}\n"
      "  }\n"
      "}",
  };
  for (auto t : app->course_trees)
    QCOMPARE(QString(t.to_string().c_str()), QString(expected[t.id].c_str()));
}

void TestGui::fetch_courses_ui_test()
{
  MainWindow *app = create_app();
  authenticate(app);

  qDebug() << "GONNA TEST THE MODEL";
  TreeModel *model = app->ui->treeView->model();
  qDebug() << "AFTER GETTING THE MODEL";
  QCOMPARE(model->childrenCount(), 3);
  qDebug() << "~~~~~~~~~~" << 1;

  TreeIndex ptr = model->index(0, 0);
  qDebug() << "~~~~~~~~~~" << 2;

  QCOMPARE(ptr.course(), "Calculus");
  qDebug() << "~~~~~~~~~~" << 3;
  ptr = ptr.child(0);
  qDebug() << "~~~~~~~~~~" << 4;
  QCOMPARE(ptr.remote_dir(), "course files");
  qDebug() << "~~~~~~~~~~" << 5;
  QCOMPARE(ptr.child(0).remote_dir(), "Admin");
  qDebug() << "~~~~~~~~~~" << 6;
  QCOMPARE(ptr.child(1).remote_dir(), "Tutorials");
  qDebug() << "~~~~~~~~~~" << 7;

  ptr = model->index(1, 0);
  QCOMPARE(ptr.course(), "Geometry");
  ptr = ptr.child(0);
  QCOMPARE(ptr.remote_dir(), "course files");
  QCOMPARE(ptr.child(0).remote_dir(), "Course ntoes");
  QCOMPARE(ptr.child(1).remote_dir(), "Lectures");
  QCOMPARE(ptr.child(2).remote_dir(), "Tutorials");

  ptr = model->index(2, 0);
  QCOMPARE(ptr.course(), "Linear Algebra");
  ptr = ptr.child(0);
  QCOMPARE(ptr.remote_dir(), "course files");
  QCOMPARE(ptr.child(0).remote_dir(), "Lectures");
  QCOMPARE(ptr.child(1).remote_dir(), "Tutorials");
}
