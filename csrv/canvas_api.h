#ifndef CANVAS_SYNC_CANVAS_API_H
#define CANVAS_SYNC_CANVAS_API_H

#include "BS_thread_pool.hpp"
#include "filetree.h"
#include "json.hpp"
#include "types.h"
#include <map>

class HttpClient
{
public:
  virtual std::string get(const std::string &url) = 0;
};

template <typename T, typename I>
inline Vec<T> get_many(const Vec<I> &input, std::function<T(I)> fn,
                       int parallel = 5)
{
  size_t n = input.size();
  Vec<std::future<T>> ft(n);
  Vec<T> r(n);
  BS::thread_pool pool(parallel);
  for (size_t i = 0; i < n; i++) {
    I v = input[i];
    ft[i] = std::move(pool.submit([fn, v] { return fn(v); }));
  }
  for (size_t i = 0; i < n; i++)
    r[i] = std::move(ft[i].get());
  return r;
}

class CanvasApi
{

private:
  HttpClient *cli;
  nlohmann::json get_json(const char *url) const
  {
    std::string raw = this->cli->get(url);
    std::cout << "PRE_JSON: (" << raw << ')';
    return json::parse(raw);
  };

public:
  // constructors
  CanvasApi() = delete;
  CanvasApi(HttpClient *cli) : cli(std::move(cli)){};

  // getters
  Profile get_profile() const;
  Vec<Course> get_courses() const;
  Vec<Folder> get_course_folders(const int &course_id) const;
  Vec<File> get_course_files(const int &course_id) const;
  Vec<File> get_folder_files(const int &folder_id) const;

  // parallel getters
  Vec<Vec<Folder>> get_course_folders(const Vec<int> &course_ids) const
  {
    std::function<Vec<Folder>(int)> get = [this](int course_id) {
      return this->get_course_folders(course_id);
    };
    return get_many(course_ids, get);
  };
  Vec<Vec<Folder>> get_course_folders(const Vec<Course> &courses) const
  {
    std::function<Vec<Folder>(Course)> get = [this](Course c) {
      return this->get_course_folders(c.id);
    };
    return get_many(courses, get);
  };
  Vec<Vec<File>> get_folder_files(const Vec<int> &folder_ids) const
  {
    std::function<Vec<File>(int)> get = [this](int folder_id) {
      return this->get_folder_files(folder_id);
    };
    return get_many(folder_ids, get);
  };

  // loaders
  void load_courses(Vec<Course> *c) const
  {
    *c = std::move(this->get_courses());
  };
  void load_tracked_folders(Vec<Vec<Folder>> *f, const Vec<Course> &c) const
  {
    *f = std::move(this->get_course_folders(c));
  };
  void load_tree(FileTree *, const Vec<Course> &,
                 const Vec<Vec<Folder>> &) const;
};

#endif
