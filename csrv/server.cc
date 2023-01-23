#include "server.h"
#include "debug.h"

using Server = canvas::Server;
using namespace std;

Server::Server(const char *token, const char *base_url)
{
  string token_s = token;
  string base_url_s = base_url;
  this->base_url.assign(base_url_s);
  this->api = new CanvasApi(new HttpJson(&token_s, &base_url_s));
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

string Server::dump_tree()
{
  return this->tree.to_string();
}

Server Server::with_token(string *token)
{
  string base_url = this->get_base_url();
  cout << "BASE -> " << base_url << std::endl;
  return *new Server(std::move(token), std::move(&base_url));
}

string Server::get_base_url()
{
  return this->base_url;
}

void Server::run_debug()
{
  debug(&this->tree);
}
