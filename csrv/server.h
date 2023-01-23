#ifndef SERVER_H
#define SERVER_H

#include "canvas_api.h"
#include "filetree.h"
#include "httpjson.h"

namespace canvas
{
class Server
{
private:
  CanvasApi *api;
  FileTree tree;
  string base_url;

public:
  Server(const char *token, const char *base_url);
  Server(string *token, string *base_url);
  void run();
  void run_debug();
  void load_tree();
  string dump_tree();
  Server with_token(string *token);

  string get_base_url();
};
}; // namespace canvas

#endif
