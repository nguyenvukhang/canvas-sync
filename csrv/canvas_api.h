#ifndef CANVAS_SYNC_CANVAS_API_H
#define CANVAS_SYNC_CANVAS_API_H

#include "filetree.h"
#include "httpclient.h"
#include "json.hpp"
#include "types.h"
#include <map>

class CanvasApi
{

private:
  HttpClient *cli;
  nlohmann::json get(const char *url)
  {
    return json::parse(this->cli->get(url));
  };

public:
  CanvasApi() = delete;
  CanvasApi(HttpClient *cli)
  {
    this->cli = cli;
  };

  Profile profile();

  std::vector<Course> courses();
  std::vector<Folder> course_folders(const int *course_id);
  std::vector<std::vector<Folder>> course_folders(const std::vector<int> *course_ids);
  std::vector<File> course_files(const int *course_id);

  void download(std::vector<File> *);
  void download(File *);
  FileTree courses_file_tree();
  void courses_file_tree(FileTree *tree, const std::vector<Course> *);

  void load(FileTree *, std::vector<Course> *, std::map<int, std::string> *folders);

  std::vector<File> folder_files(const int *);
  std::vector<std::vector<File>> folder_files(const std::vector<int> *);

  static const char *get_token_from_env()
  {
    char *token = std::getenv("CANVAS_TOKEN");
    if (token == NULL) {
      std::cerr << "[error] $CANVAS_TOKEN environment variable not found" << std::endl;
      std::exit(1);
    }
    return token;
  };
};

#endif
