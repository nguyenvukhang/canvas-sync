#include "canvas_api.h"
#include "BS_thread_pool.hpp"
#include "debug.h"
#include "filetree.h"
#include <filesystem>
#include <fstream>
#include <functional>

using json = nlohmann::json;
namespace fs = std::filesystem;

template <typename T, typename I>
std::vector<T> get_many(const std::vector<I> *input, std::function<T(I)> func,
                        int parallel = 5)
{
  std::vector<std::future<T>> futures;
  BS::thread_pool pool(parallel);
  size_t n = input->size();
  for (size_t i = 0; i < n; i++) {
    I input_value = input->at(i);
    std::future<T> fut =
        pool.submit([func, input_value] { return func(input_value); });
    futures.push_back(std::move(fut));
  }
  std::vector<T> all;
  for (auto fut = futures.begin(); fut < futures.end(); fut++) {
    T f = fut->get();
    all.push_back(std::move(f));
  }
  return all;
}

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

std::vector<Course> CanvasApi::courses()
{
  json j = this->get("/api/v1/users/self/courses?per_page=118");
  return to_vec<Course>(j);
}

std::vector<File> CanvasApi::course_files(const int *course_id)
{
  std::string url =
      "/api/v1/courses/" + std::to_string(*course_id) + "/files?per_page=10000";
  json j = this->get(url.c_str());
  return to_vec<File>(j);
}

std::vector<File> CanvasApi::folder_files(const int *folder_id)
{
  std::string url =
      "/api/v1/folders/" + std::to_string(*folder_id) + "/files?per_page=10000";
  json j = this->get(url.c_str());
  auto v = to_vec<File>(j);
  size_t n = v.size();
  while (n-- > 0)
    v[n].filename = normalize_filename(&v[n].filename);
  return v;
}

std::vector<std::vector<File>>
CanvasApi::folder_files(const std::vector<int> *folder_ids)
{
  std::function<std::vector<File>(int)> get = [this](int folder_id) {
    return this->folder_files(&folder_id);
  };
  return get_many(folder_ids, get);
}

std::vector<Folder> CanvasApi::course_folders(const int *course_id)
{
  std::string url = "/api/v1/courses/" + std::to_string(*course_id) +
                    "/folders?per_page=10000";
  json j = this->get(url.c_str());
  return to_vec<Folder>(j);
}

std::vector<std::vector<Folder>>
CanvasApi::course_folders(const std::vector<int> *course_ids)
{
  std::function<std::vector<Folder>(int)> b = [this](int course_id) {
    return this->course_folders(&course_id);
  };
  return get_many(course_ids, b);
}

FileTree CanvasApi::courses_file_tree()
{
  std::vector<Course> courses = this->courses();
  std::vector<int> course_ids;
  for (auto c : courses)
    course_ids.push_back(c.id);
  std::vector<std::vector<Folder>> folders = this->course_folders(&course_ids);

  FileTree *root = new FileTree(0, "root");

  size_t n = course_ids.size();
  for (size_t i = 0; i < n; i++) {
    FileTree *tree = new FileTree(&courses[i]);
    tree->insert_folders(folders[i]);
    root->insert_tree(tree);
  }

  return *root;
};

void CanvasApi::load(FileTree *t, std::vector<Course> *c,
                     std::map<int, std::string> *m)
{
  *c = this->courses();
  std::vector<int> c_ids;
  for (auto c : *c)
    c_ids.push_back(c.id);
  std::vector<std::vector<Folder>> folders = this->course_folders(&c_ids);

  size_t n = c_ids.size();
  for (size_t i = 0; i < n; i++) {
    FileTree *tree = new FileTree(&c->at(i));
    tree->insert_folders(folders[i]);
    t->insert_tree(tree);
  }

  for (auto f : folders)
    for (auto f : f)
      m->insert(std::pair(f.id, f.full_name));
}

void CanvasApi::courses_file_tree(FileTree *root,
                                  const std::vector<Course> *courses)
{
  std::vector<std::future<FileTree>> futures;
  BS::thread_pool pool(5);

  for (auto it = courses->begin(); it < courses->end(); it++) {
    Course course = *it;
    FileTree *tree = new FileTree(&course);
    std::future<FileTree> course_tree = pool.submit([tree, this] {
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

void CanvasApi::download(std::vector<File> *files)
{
  BS::thread_pool pool(5);
  std::vector<std::future<void>> futures;

  for (auto file : *files) {
    std::future<void> dl = pool.submit([file, this] {
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
  if (!fs::exists(file->local_dir)) {
    std::cerr << "Download's target directory does not exist." << std::endl;
    std::cerr << file->local_dir << std::endl;
    std::exit(1);
  }
  fs::path local_path = file->local_dir;
  local_path.append(file->filename);
  // clear existing
  fs::remove(local_path);
  std::ofstream of(local_path, std::ios::binary);
  this->cli->get(file->url, [&](const char *data, size_t len) {
    of.write(data, len);
    return true;
  });
  of.close();
}
