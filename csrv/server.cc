#include "server.h"
#include <algorithm>
#include <filesystem>
#include <functional>
#include <map>

using namespace canvas;

Server::Server(HttpClient *client, const std::string base_url)
    : CanvasApi(std::move(client)), base_url(base_url)
{
}

std::string Server::folder_name(const int folder_id)
{
  return this->folder_names.at(folder_id);
}

std::string Server::course_name(const int course_id)
{
  std::vector<Course> c = this->courses;
  size_t i = this->courses.size();
  while (i-- > 0) {
    if (this->courses[i].id == course_id) {
      return this->courses[i].name;
    }
  }
  return "[course not found]";
}

void Server::load()
{
  this->load_courses(&this->courses);
  Vec<Vec<Folder>> f;
  this->load_tracked_folders(&f, this->courses);
  this->load_tree(&this->tree, this->courses, f);
}

void Server::fetch_updates(std::vector<Update> *u)
{
  std::vector<int> folder_ids;
  size_t n = u->size();
  for (size_t i = 0; i < n; i++) {
    folder_ids.push_back(u->at(i).folder_id);
  }
  std::vector<std::vector<File>> af = this->get_folder_files(folder_ids);
  this->merge_data(u, &af);
}

void Server::download_updates(const std::vector<Update> *u)
{
  size_t n = u->size();
  for (size_t i = 0; i < n; i++) {
    std::filesystem::create_directories(u->at(i).local_dir);
    std::vector<File> files = u->at(i).files;
    // FIXME
    // api->download(&files);
  }
}

FileTree *Server::get_tree()
{
  return &this->tree;
}

std::string Server::dump_tree()
{
  return this->tree.to_string();
}

void Server::set_token(const std::string &token)
{
  this->token = token;
}

// bool Server::valid_token()
// {
//   return this->api->profile().id > 0;
// }

std::map<int, std::vector<File>> to_map(std::vector<File> *files)
{
  std::map<int, std::vector<File>> m;
  for (auto f : *files) {
    auto key = m.find(f.folder_id);
    // already exists
    if (key != m.end()) {
      key->second.push_back(f);
    } else {
      std::vector<File> files;
      files.push_back(f);
      m.insert(std::pair(f.folder_id, files));
    }
  }
  return m;
}

void insert_files(FileTree *tree, std::map<int, std::vector<File>> *files)
{
  auto key = files->extract(tree->id);
  if (!key.empty()) {
    std::cout << "hit! " << tree->id << " @ " << tree->name << std::endl;
    std::vector<File> v = key.mapped();
    tree->files.assign(v.begin(), v.end());
  }
  size_t n = tree->folders.size();
  for (size_t i = 0; i < n; i++) {
    insert_files(&tree->folders[i], files);
  }
}

// std::vector<File> Server::folder_files(const int *folder_id)
// {
//   return this->api->folder_files(folder_id);
// }

void Server::merge_data(std::vector<Update> *updates,
                        std::vector<std::vector<File>> *files)
{
  namespace fs = std::filesystem;
  size_t n = updates->size();
  for (size_t i = 0; i < n; i++) {
    std::vector<File> f = files->at(i);
    size_t fc = f.size();
    fs::path local_dir = updates->at(i).local_dir;
    (*updates)[i].remote_dir = this->folder_name(updates->at(i).folder_id);
    for (size_t j = 0; j < fc; j++) {
      if (!fs::exists(local_dir / f[j].filename)) {
        f[j].local_dir = local_dir;
        (*updates)[i].files.push_back(f[j]);
      }
    }
  }
}

bool Server::valid_token()
{
  return this->get_profile().id > 0;
}

// void Server::run_debug(const std::string &token)
// {
//   this->set_token(token);
//   Profile p = api->profile();
//   std::cout << "ID: " << p.integration_id << std::endl;
// }
