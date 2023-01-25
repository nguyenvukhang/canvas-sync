#include "types.h"
#include <cstdarg>

void eprintln(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  std::cerr << std::endl;
}

std::string normalize_filename(const char *a, size_t size)
{
  std::string result = "";
  while (size-- > 0) {
    if (*a == '+' || *a == '-' || *a == ' ') {
      result.push_back('_');
      a++;
      continue;
    }
    if (*a == '_' && *(a + 1) == '_' && size > 0)
      size--, a++;
    result.push_back(*a++);
  }
  return result;
}

std::string normalize_filename(std::string *v)
{
  size_t n = v->size();
  std::string s = normalize_filename(v->c_str(), n);
  while (s.size() < n)
    s = normalize_filename(s.c_str(), (n = s.size()));
  return s;
}

void debug(Profile *p)
{
  eprintln("Profile");
  eprintln("* id:         %d", p->id);
  eprintln("* name:       %s", p->name.c_str());
  eprintln("* email:      %s", p->primary_email.c_str());
  eprintln("* login_id:   %s", p->login_id.c_str());
  eprintln("* student_id: %s", p->integration_id.c_str());
}
void debug(Course *c)
{
  eprintln("Course");
  eprintln("* id:   %d", c->id);
  eprintln("* name: %s", c->name.c_str());
}
void debug(File *c)
{
  eprintln("File { %d, %s }", c->id, c->filename.c_str());
}
void debug(Update *c)
{
  eprintln("Update { folder_id: %d, local_dir: %s }", c->folder_id,
           c->local_dir.c_str());
}
void debug(Folder *c)
{
  eprintln("Folder { %d, %s, (%s) }", c->id, c->full_name.c_str(),
           c->name.c_str());
}
