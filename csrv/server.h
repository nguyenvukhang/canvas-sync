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
  std::vector<Course> courses;
  std::map<int, std::string> folder_names;
  std::string base_url;
  void merge_data(std::vector<Update> *updates,
                  std::vector<std::vector<File>> *files);

public:
  std::string folder_name(int folder_id);
  std::string course_name(int course_id);
  Server(const char *token, const char *base_url);
  Server(std::string *token, std::string *base_url);
  void run();
  void run_debug();

  void load();

  void fetch_updates(std::vector<Update> *);
  void download_updates(const std::vector<Update> *);

  void load_tree();
  std::string dump_tree();
  FileTree *get_tree();

  bool valid_token();

  std::vector<File> folder_files(const int *folder_id);

  void set_token(std::string *token);
};
}; // namespace canvas

#endif
