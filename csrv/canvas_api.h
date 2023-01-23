#ifndef CANVAS_SYNC_CANVAS_API_H
#define CANVAS_SYNC_CANVAS_API_H

#include "filetree.h"
#include "httpclient.h"
#include "json.hpp"
#include "types.h"

using namespace std;

class CanvasApi {

private:
  HttpClient *cli;
  nlohmann::json get(const char *url) {
    return json::parse(this->cli->get(url));
  };

public:
  CanvasApi() = delete;
  CanvasApi(HttpClient *cli) { this->cli = cli; };

  Profile profile();
  vector<Course> courses();
  vector<Folder> course_folders(const int *course_id);
  vector<File> course_files(const int *course_id);
  FileTree courses_file_tree();
  void courses_file_tree(FileTree *tree);

  static const char *get_token_from_env() {
    char *token = std::getenv("CANVAS_TOKEN");
    if (token == NULL) {
      cerr << "[error] $CANVAS_TOKEN environment variable not found" << endl;
      std::exit(1);
    }
    return token;
  };
};

#endif
