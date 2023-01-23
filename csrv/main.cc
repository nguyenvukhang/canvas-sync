#include "main.h"
#include "BS_thread_pool.hpp"
#include "canvas_api.h"
#include "debug.h"
#include "errors.h"
#include "filetree.h"
#include "httpjson.h"
#include "types.h"
#include <algorithm>
#include <future>
#include <thread>

using json = nlohmann::json;

void version(char *bin_name) {
  std::cout << bin_name << " Version " << CanvasServer_VERSION_MAJOR << "."
            << CanvasServer_VERSION_MINOR << std::endl;
}

class Server {
  CanvasApi *api;
  FileTree tree;

public:
  Server(const char *token, const char *base_url) {
    string token_s = token == NULL ? CanvasApi::get_token_from_env() : token;
    string base_url_s = base_url;
    this->api = new CanvasApi(new HttpJson(&token_s, &base_url_s));
  }

  void run() {
    Profile profile = api->profile();
    debug(&profile);
    api->courses_file_tree(&this->tree);
  }

  void run_debug() { debug(&this->tree); }
};

int main(int argc, char **argv) {
  Server server(NULL, "https://canvas.nus.edu.sg");
  server.run();
  server.run_debug();
  return 0;
}
