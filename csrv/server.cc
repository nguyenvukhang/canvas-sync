#include "server.h"
#include "debug.h"

using Server = canvas::Server;
using namespace std;

Server::Server(const char *token, const char *base_url)
{
  string token_s = token;
  this->base_url.assign(base_url);
  this->api = new CanvasApi(new HttpJson(&token_s, &this->base_url));
}

Server::Server(string *token, string *base_url)
{
  this->base_url.assign(*base_url);
  this->api = new CanvasApi(new HttpJson(token, base_url));
}

void Server::run()
{
  Profile profile = api->profile();
  debug(&profile);
  api->courses_file_tree(&this->tree);
}

void Server::load_tree()
{
  api->courses_file_tree(&this->tree);
}

FileTree *Server::get_tree()
{
  return &this->tree;
}

string Server::dump_tree()
{
  return this->tree.to_string();
}

void Server::set_token(string *token)
{
  this->api = new CanvasApi(new HttpJson(token, &this->base_url));
}

bool Server::valid_token()
{
  return this->api->profile().id > 0;
}

void Server::run_debug()
{
  debug(&this->tree);
}
