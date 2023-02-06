#include "fake_canvas.h"

void FakeCanvas::authenticate()
{
  emit authenticate_done(token() == "valid");
};

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
