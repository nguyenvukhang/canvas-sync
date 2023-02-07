#include "main.h"

QTEST_MAIN(TestGui)

void wait(int ms)
{
  QTest::qWait(ms);
}

void sleep(int ms)
{
  QTest::qSleep(ms);
}

void log(std::string log_text)
{
  std::cout << "--> " << log_text << std::endl;
}

void log(std::string log_text, int x, int y)
{
  std::cout << "--> " << log_text << '(' << x << ", " << y << ')' << std::endl;
}

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
  TreeModel *model = app->ui->treeView->model();
  TreeIndex ptr(model->index(0, 0));

  QCOMPARE(ptr.course(), "Calculus");
  ptr = ptr.child(0);
  QCOMPARE(ptr.remote_dir(), "course files");
  QCOMPARE(ptr.child(0).remote_dir(), "Admin");
  QCOMPARE(ptr.child(1).remote_dir(), "Tutorials");

  ptr = model->index(1, 0);
  QCOMPARE(ptr.course(), "Geometry");
  ptr = ptr.child(0);
  QCOMPARE(ptr.remote_dir(), "course files");
  QCOMPARE(ptr.child(0).remote_dir(), "Course notes");
  QCOMPARE(ptr.child(1).remote_dir(), "Lectures");
  QCOMPARE(ptr.child(2).remote_dir(), "Tutorials");

  ptr = model->index(2, 0);
  QCOMPARE(ptr.course(), "Linear Algebra");
  ptr = ptr.child(0);
  QCOMPARE(ptr.remote_dir(), "course files");
  QCOMPARE(ptr.child(0).remote_dir(), "Lectures");
  QCOMPARE(ptr.child(1).remote_dir(), "Tutorials");
}

void TestGui::track_folder_ui_test()
{
  MainWindow *app = create_app();
  ClickableTreeView *t = app->ui->treeView;

  app->show();
  t->menu.setEnabled(true);

  authenticate(app);

  TreeIndex ptr = t->model()->index(1, 0);

  // this probably sets focus on the target. Not sure why this is needed.
  QTest::mouseClick(t->viewport(), Qt::MouseButton::LeftButton, Qt::NoModifier,
                    t->visualRect(ptr).center());

  // expand "Geometry" course files
  QCOMPARE(ptr.course(), "Geometry");
  QVERIFY(!t->isExpanded(ptr));
  QTest::mouseDClick(t->viewport(), Qt::LeftButton, {},
                     t->visualRect(ptr).center());
  QVERIFY(t->isExpanded(ptr));

  // go to the first child folder of "Geometry"
  ptr = ptr.child(0);

  QInputEvent *rc = new QContextMenuEvent(QContextMenuEvent::Mouse,
                                          t->visualRect(ptr).center());
  qApp->postEvent(t->viewport(), rc);
  // QTest::mouseClick(t->viewport(), Qt::RightButton, {},
  //                   t->visualRect(ptr).center());

  for (int x = 0; x < 50; x += 5) {
    for (int y = 0; y < 50; y += 5) {
      // rc=
      // QTest::mouseClick(t->viewport(), Qt::RightButton, {}, QPoint(x, y));
      log("GO", x, y);
      wait(50);
    }
  }

  qDebug() << "Complete execution";
}
