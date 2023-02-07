#ifndef CANVAS_SYNC_SETTINGS_H
#define CANVAS_SYNC_SETTINGS_H

#include <algorithm>
#include <filesystem>
#include <iostream> // for std::cout and std::cerr in the .cc file
#include <vector>

class Profile
{
public:
  int id;
  std::string name;
  std::string primary_email;
  std::string login_id;
  std::string integration_id;
  Profile() : id(-1){};
};

class Course
{
public:
  int id;
  std::string name;
  Course(const int id, const std::string &name) : id(id), name(name){};
};

class File
{
public:
  int id;
  int folder_id;
  std::string filename;
  std::string url;
  std::filesystem::path local_dir;
};

class Folder
{
public:
  int id, course_id;
  std::string name;
  std::string full_name;
  std::filesystem::path local_dir;
  std::vector<File> files;
  Folder(const int id) : id(id){};
  Folder(const int id, std::filesystem::path local_dir)
      : id(id), local_dir(local_dir){};
  Folder(const Folder &f)
      : id(f.id), course_id(f.course_id), name(f.name), local_dir(f.local_dir),
        files(f.files), full_name(f.full_name){};
  static Folder of(const int id, const std::string &full_name)
  {
    Folder f(id);
    f.full_name = full_name;
    f.name = full_name.substr(full_name.rfind('/') + 1);
    return f;
  };
};

std::string normalize_filename(std::string *);
template <typename T> void swap_remove(std::vector<T> *, size_t);
void remove_existing_files(std::vector<File> *, const std::filesystem::path &);

void debug(Profile *);
void debug(Course *);
void debug(Folder *);
void debug(File *);

#endif
