#ifndef SERVER_H
#define SERVER_H

#include "canvas_api.h"
#include "filetree.h"
#include "httpjson.h"
#include <map>

namespace canvas
{
class Server
{
private:
  CanvasApi *api;
  FileTree tree;
  vector<Course> courses;
  map<int, string> folder_names;
  string base_url;
  void merge_data(vector<Update> *updates, vector<vector<File>> *files);

public:
  string folder_name(int folder_id);
  string course_name(int course_id);
  Server(const char *token, const char *base_url);
  Server(string *token, string *base_url);
  void run();
  void run_debug();

  void load();

  void fetch_updates(vector<Update> *);
  void download_updates(const vector<Update> *);

  void load_tree();
  string dump_tree();
  FileTree *get_tree();

  bool valid_token();

  vector<File> folder_files(const int *folder_id);

  void set_token(string *token);
};
}; // namespace canvas

#endif
