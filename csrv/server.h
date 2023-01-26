#ifndef SERVER_H
#define SERVER_H

#include "canvas_api.h"
#include "filetree.h"
#include <map>

namespace canvas
{
class Server : public CanvasApi
{
private:
  const std::string base_url;
  std::string token;
  FileTree tree;
  std::vector<Course> courses;
  std::map<int, std::string> folder_names;

  void merge_data(std::vector<Update> *updates,
                  std::vector<std::vector<File>> *files);

public:
  Server(HttpClient *client, const std::string base_url);

  std::string folder_name(const int folder_id);
  std::string course_name(const int course_id);
  void load();

  // oldies
  void fetch_updates(std::vector<Update> *);
  void download_updates(const std::vector<Update> *);

  std::string dump_tree();
  FileTree *get_tree();

  bool valid_token();

  std::vector<File> folder_files(const int *folder_id);

  void set_token(const std::string &token);
  void run_debug(const std::string &);
};
}; // namespace canvas

#endif
