#include "server.h"
#include "debug.h"
#include <map>

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
}

void Server::load_tree()
{
  this->courses = api->courses();
  api->courses_file_tree(&this->tree, &this->courses);
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

std::map<int, vector<File>> to_map(vector<File> *files)
{
  std::map<int, vector<File>> m;
  for (auto f : *files) {
    auto key = m.find(f.folder_id);
    // already exists
    if (key != m.end()) {
      key->second.push_back(f);
    } else {
      vector<File> files;
      files.push_back(f);
      m.insert(std::pair(f.folder_id, files));
    }
  }
  return m;
}

void insert_files(FileTree *tree, std::map<int, vector<File>> *files)
{
  auto key = files->extract(tree->id);
  if (!key.empty()) {
    cout << "hit! " << tree->id << " @ " << tree->name << endl;
    vector<File> v = key.mapped();
    tree->files.assign(v.begin(), v.end());
  }
  int n = tree->folders.size();
  for (int i = 0; i < n; i++) {
    insert_files(&tree->folders[i], files);
  }
}

vector<File> Server::folder_files(const int *folder_id)
{
  return this->api->folder_files(folder_id);
}

void Server::run_debug()
{
  this->load_tree();
  const int course_id = 38518; // CS2040
  int n = this->courses.size();

  // for (int i = 0; i < n; i++) { Course c = this->courses[i];
  //   vector<File> files = this->api->course_files(&c.id);
  // }
  vector<File> files = this->api->course_files(&course_id);

  auto m = to_map(&files);
  // vector<File> downloads;
  // for (int i = 0; i < 5; i++) {
  //   downloads.push_back(files[i]);
  // }
  // api->download(downloads);

  // debug(&this->tree);
  for (std::map<int, vector<File>>::iterator iter = m.begin(); iter != m.end();
       ++iter) {
    int k = iter->first;
    cout << k << endl;
    // ignore kvalue
    // Value v = iter->second;
  }
  insert_files(&this->tree, &m);
  debug(&tree);
}
