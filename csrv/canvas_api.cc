#include "canvas_api.h"
#include "BS_thread_pool.hpp"
#include "debug.h"
#include "filetree.h"
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

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
  return to_vec<File>(j);
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

void CanvasApi::download(vector<File> files)
{
  BS::thread_pool pool(5);
  vector<future<void>> futures;

  namespace fs = std::filesystem;

  for (auto file : files) {
    // create local path
    fs::path local_path = file.local_dir;
    local_path.append(file.filename);

    __fs::filesystem::remove(file.local_dir);
    future<void> dl = pool.submit([file, this, local_path] {
      std::ofstream of(local_path, std::ios::binary);
      this->cli->get(file.url, [&](const char *data, size_t data_length) {
        of.write(data, data_length);
        return true;
      });
      of.close();
    });
    futures.push_back(std::move(dl));
  }
  for (auto fut = futures.begin(); fut < futures.end(); fut++) {
    fut->get();
  }
}
