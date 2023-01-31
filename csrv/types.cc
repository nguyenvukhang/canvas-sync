#include "types.h"
#include <cstdarg>

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
  std::cout << "Profile" << std::endl;
  printf("* id:         %d\n", p->id);
  printf("* name:       %s\n", p->name.c_str());
  printf("* email:      %s\n", p->primary_email.c_str());
  printf("* login_id:   %s\n", p->login_id.c_str());
  printf("* student_id: %s\n", p->integration_id.c_str());
}
void debug(Course *c)
{
  std::cout << "Course" << std::endl;
  printf("* id:   %d\n", c->id);
  printf("* name: %s\n", c->name.c_str());
}
void debug(File *c)
{
  printf("File { %d, %s }\n", c->id, c->filename.c_str());
}
void debug(Update *c)
{
  printf("Update { folder_id: %d, local_dir: %s }\n", c->folder_id,
           c->local_dir.string().c_str());
}
void debug(Folder *c)
{
  printf("Folder { %d, %s, (%s) }\n", c->id, c->full_name.c_str(),
           c->name.c_str());
}
