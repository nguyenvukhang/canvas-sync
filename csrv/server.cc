#include "server.h"
#include "debug.h"
#include <filesystem>
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
  // 142793 /course files
  // 235231 /course files/Lecture
  // 270993 /course files/Recitation
  //
  // 142803 /course files
  // 224009 /course files/Labs
  // 224010 /course files/Lectures
  // 224013 /course files/Lectures/Java Intro and ADT
  // 234811 /course files/Lectures/Week1
  // 260627 /course files/Lectures/Week2
  // 282247 /course files/Lectures/Week3
  // 224011 /course files/Tutorials
  // 282251 /course files/Tutorials/Tutorial1

  namespace fs = std::filesystem;
  fs::path base = "/Users/khang/uni/CS2040-ta";
  vector<Update> updates;
  vector<int> folder_ids;
  std::function<void(int, string)> f = [&](int i, string s) {
    updates.push_back(*new Update(i, s));
    folder_ids.push_back(i);
  };
  f(224013, base / "course files/Lectures/Java Intro and ADT");
  f(234811, base / "course files/Lectures/Week1");
  f(224011, base / "course files/Tutorials");

  vector<vector<File>> all_files = this->api->folder_files(&folder_ids);

  int n = updates.size();
  for (int i = 0; i < n; i++) {
    auto u = updates[i];
    auto f = all_files[i];
    fs::path local_dir = u.local_dir;
    cout << u.folder_id << " -> " << u.local_dir << endl;
    for (auto i : f) {
      auto target = local_dir / i.filename;
      cout << target << " ? " << fs::exists(target) << endl;
      if (!fs::exists(target)) {
        i.local_dir = local_dir;
        api->download(&i);
      }
    }
  }
}
