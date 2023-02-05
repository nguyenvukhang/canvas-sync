#include "main.h"

QTEST_MAIN(TestGui)

std::string strip_whitespace(std::string str)
{
  str.erase(remove_if(str.begin(), str.end(), isspace), str.end());
  return str;
}

void TestGui::authenticate()
{
  QPushButton *change_token = app.ui->pushButton_changeToken;
  QTest::mouseClick(change_token, Qt::MouseButton::LeftButton);
  QTest::keyClicks(app.ui->lineEdit_accessToken, "valid");
}

void FakeCanvas::authenticate()
{
  emit authenticate_done(token() == "valid");
};

void TestGui::access_token_entry_test()
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

void FakeCanvas::fetch_courses()
{
  std::vector<Course> c;
  c.push_back(Course(0, "Linear Algebra"));
  c.push_back(Course(1, "Calculus"));
  c.push_back(Course(2, "Geometry"));
  emit fetch_courses_done(c);
};

void FakeCanvas::fetch_folders(const Course &course)
{
  std::vector<Folder> folders;
  std::function<void(int, std::string)> add_folder =
      [&](int id, std::string n) { folders.push_back(Folder::of(id, n)); };

  switch (course.id) {
  case (0):
    add_folder(20010, "course files");
    add_folder(20011, "course files/Lectures");
    add_folder(20012, "course files/Tutorials");
    emit fetch_folders_done(course, folders);
    break;
  case (1):
    add_folder(20021, "course files/Admin");
    add_folder(20022, "course files/Tutorials");
    add_folder(20020, "course files");
    emit fetch_folders_done(course, folders);
    break;
  case (2):
    add_folder(22191, "course files/Lectures");
    add_folder(22192, "course files/Course notes");
    add_folder(22193, "course files/Tutorials");
    add_folder(22190, "course files");
    emit fetch_folders_done(course, folders);
    break;
  default:
    break;
  }
}

void TestGui::fetch_courses_test()
{
  authenticate();
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
  for (auto t : app.course_trees)
    QCOMPARE(QString(t.to_string().c_str()), QString(expected[t.id].c_str()));
}

void TestGui::fetch_courses_ui_test()
{
  authenticate();

  TreeModel *model = app.ui->treeView->model();
  QCOMPARE(model->childrenCount(), 3);

  TreeIndex ptr = model->index(0, 0);

  QCOMPARE(ptr.course(), "Calculus");
  ptr = ptr.child(0);
  QCOMPARE(ptr.remote_dir(), "course files");
  QCOMPARE(ptr.child(0).remote_dir(), "Admin");
  QCOMPARE(ptr.child(1).remote_dir(), "Tutorials");

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
