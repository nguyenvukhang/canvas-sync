#ifndef CANVAS_SYNC_TYPES_H
#define CANVAS_SYNC_TYPES_H

#include "json.hpp"
#include <iostream> // for std::cout and std::cerr in the .cc file

using namespace std;
using json = nlohmann::json;

// extract all valid sub-json to form a vec. ignore invalids.
template <typename T> vector<T> to_vec(json j)
{
  vector<T> v;
  for (json::iterator it = j.begin(); it != j.end(); ++it)
    try {
      v.push_back(it->get<T>());
    } catch (json::exception) {
    }
  return v;
}

class Profile
{
public:
  int id;
  string name;
  string primary_email;
  string login_id;
  string integration_id;
  Profile()
  {
    this->id = -1;
  }
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Profile, id, name, login_id, integration_id,
                                   primary_email);

class Course
{
public:
  int id;
  string name;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Course, id, name);

class Folder
{
public:
  int id;
  string name;
  string full_name;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Folder, id, name, full_name);

class File
{
public:
  int id;
  int folder_id;
  string filename;
  string url;
  std::filesystem::path local_dir;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(File, id, folder_id, filename, url);

// enough information to execute a download + display updates
class Update
{
public:
  // needed for download
  // using `folder_id`, a list of files can be fetched from the API
  // these files will then be checked against files inside of `local_dir`
  int folder_id;
  std::filesystem::path local_dir;

  // needed for update report
  int course_id;
  string remote_dir; // can just read from an already-fetched vector<Folder>
  vector<File> files; // update after running the download's API call

  Update(const int folder_id, const string local_dir)
  {
    this->folder_id = folder_id;
    this->local_dir = std::move(local_dir);
  }
  Update(const int folder_id)
  {
    this->folder_id = folder_id;
  }
};

string normalize_filename(string *);

void debug(Profile *);
void debug(Course *);
void debug(Folder *);
void debug(File *);
void debug(Update *);

#endif
