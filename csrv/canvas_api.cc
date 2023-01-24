#include "canvas_api.h"
#include "BS_thread_pool.hpp"
#include "debug.h"
#include "filetree.h"
#include <filesystem>
#include <fstream>

using json = nlohmann::json;
namespace fs = std::filesystem;

Profile CanvasApi::profile()
{
  try {
    return this->get("/api/v1/users/self/profile").get<Profile>();
  } catch (std::runtime_error) {
    return *new Profile();
  } catch (json::exception) {
    return *new Profile();
  }
}

vector<Course> CanvasApi::courses()
{
  json j = this->get("/api/v1/users/self/courses?per_page=118");
  return to_vec<Course>(j);
}

vector<File> CanvasApi::course_files(const int *course_id)
{
  string url =
      "/api/v1/courses/" + to_string(*course_id) + "/files?per_page=10000";
  json j = this->get(url.c_str());
  return to_vec<File>(j);
}

vector<File> CanvasApi::folder_files(const int *folder_id)
{
  string url =
      "/api/v1/folders/" + to_string(*folder_id) + "/files?per_page=10000";
  json j = this->get(url.c_str());
  auto v = to_vec<File>(j);
  int n = v.size();
  while (n-- > 0)
    v[n].filename = normalize_filename(&v[n].filename);
  return v;
}

vector<vector<File>> CanvasApi::folder_files(const vector<int> *folder_ids)
{
  vector<future<vector<File>>> futures;
  BS::thread_pool pool(5);
  int n = folder_ids->size();
  for (int i = 0; i < n; i++) {
    int folder_id = folder_ids->at(i);
    future<vector<File>> files = pool.submit(
        [this, folder_id] { return this->folder_files(&folder_id); });
    futures.push_back(std::move(files));
  }

  vector<vector<File>> all;
  for (auto fut = futures.begin(); fut < futures.end(); fut++) {
    vector<File> files = fut->get();
    all.push_back(std::move(files));
  }
  return all;
}

vector<Folder> CanvasApi::course_folders(const int *course_id)
{
  string url =
      "/api/v1/courses/" + to_string(*course_id) + "/folders?per_page=10000";
  json j = this->get(url.c_str());
  return to_vec<Folder>(j);
}

FileTree CanvasApi::courses_file_tree()
{
  vector<Course> courses = this->courses();
  vector<future<FileTree>> futures;
  BS::thread_pool pool(5);

  for (auto course = courses.begin(); course < courses.end(); course++) {
    FileTree *tree = new FileTree(&*course);
    future<FileTree> course_tree = pool.submit([tree, this] {
      tree->insert_folders(this->course_folders(&tree->id));
      return *tree;
    });
    futures.push_back(std::move(course_tree));
  }

  FileTree *root = new FileTree(0, "root");
  for (auto fut = futures.begin(); fut < futures.end(); fut++) {
    FileTree tree = fut->get();
    root->insert_tree(&tree);
  }

  return *root;
};

void CanvasApi::courses_file_tree(FileTree *root, const vector<Course> *courses)
{
  vector<future<FileTree>> futures;
  BS::thread_pool pool(5);

  for (auto it = courses->begin(); it < courses->end(); it++) {
    Course course = *it;
    FileTree *tree = new FileTree(&course);
    future<FileTree> course_tree = pool.submit([tree, this] {
      tree->insert_folders(this->course_folders(&tree->id));
      return *tree;
    });
    futures.push_back(std::move(course_tree));
  }

  for (auto fut = futures.begin(); fut < futures.end(); fut++) {
    FileTree tree = fut->get();
    root->insert_tree(&tree);
  }
};

void CanvasApi::download(vector<File> *files)
{
  BS::thread_pool pool(5);
  vector<future<void>> futures;

  for (auto file : *files) {
    future<void> dl = pool.submit([file, this] {
      File f = file;
      return this->download(&f);
    });
    futures.push_back(std::move(dl));
  }
  for (auto fut = futures.begin(); fut < futures.end(); fut++)
    fut->get();
}

void CanvasApi::download(File *file)
{
  // create local path
  fs::path local_path = file->local_dir;
  bool ok = fs::create_directories(file->local_dir);
  cout << "MKDIR " << file->local_dir << " -> " << ok << endl;
  local_path.append(file->filename);
  cout << "DOWNLOADING" << file->url << "->" << local_path << endl;
  // clear existing
  fs::remove(local_path);
  std::ofstream of(local_path, std::ios::binary);
  this->cli->get(file->url, [&](const char *data, size_t data_length) {
    of.write(data, data_length);
    return true;
  });
  of.close();
}
