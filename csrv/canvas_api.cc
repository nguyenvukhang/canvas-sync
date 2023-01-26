#include "canvas_api.h"
#include "filetree.h"

using json = nlohmann::json;

Profile CanvasApi::get_profile() const
{
  json j = this->get_json("/api/v1/users/self/profile");
  if (!j["id"].is_number() || j["name"].is_null() ||
      j["primary_email"].is_null() || j["login_id"].is_null() ||
      j["integration_id"].is_null()) {
    return *new Profile();
  }
  return j.get<Profile>();
}

Vec<Course> CanvasApi::get_courses() const
{
  json j = this->get_json("/api/v1/users/self/courses?per_page=118");
  return to_vec<Course>(j);
}

Vec<File> CanvasApi::get_course_files(const int &course_id) const
{
  std::string url =
      "/api/v1/courses/" + std::to_string(course_id) + "/files?per_page=10000";
  json j = this->get_json(url.c_str());
  return to_vec<File>(j);
}

Vec<File> CanvasApi::get_folder_files(const int &folder_id) const
{
  std::string url =
      "/api/v1/folders/" + std::to_string(folder_id) + "/files?per_page=10000";
  json j = this->get_json(url.c_str());
  auto v = to_vec<File>(j);
  size_t n = v.size();
  while (n-- > 0)
    v[n].filename = normalize_filename(&v[n].filename);
  return v;
}

Vec<Folder> CanvasApi::get_course_folders(const int &course_id) const
{
  std::string url = "/api/v1/courses/" + std::to_string(course_id) +
                    "/folders?per_page=10000";
  json j = this->get_json(url.c_str());
  return to_vec<Folder>(j);
}

void CanvasApi::load_tree(FileTree *t, const Vec<Course> &c,
                          const Vec<Vec<Folder>> &f) const
{
  for (size_t i = 0; i < c.size(); i++) {
    FileTree *tree = new FileTree(&c[i]);
    tree->insert_folders(f[i]);
    t->insert_tree(tree);
  }
  // TODO: use me to hash folders
  //  for (auto f : folders)
  //    for (auto f : f)
  //      m->insert(std::pair(f.id, f.full_name));
}

// TODO: re-implement this elsewhere, and delete this
// Canvas-api has nothing to do with this because all it does is stream the
// void CanvasApi::download(Vec<File> *files)
// {
//   BS::thread_pool pool(5);
//   Vec<std::future<void>> futures;
//
//   for (auto file : *files) {
//     std::future<void> dl = pool.submit([file, this] {
//       File f = file;
//       return this->download(&f);
//     });
//     futures.push_back(std::move(dl));
//   }
//   for (auto fut = futures.begin(); fut < futures.end(); fut++)
//     fut->get();
// }

// TODO: re-implement this elsewhere, and delete this
// Canvas-api has nothing to do with this because all it does is stream the
// bytes of a url to a file. void CanvasApi::download(File *file)
// {
//   if (!fs::exists(file->local_dir)) {
//     std::cerr << "Download's target directory does not exist." << std::endl;
//     std::cerr << file->local_dir << std::endl;
//     std::exit(1);
//   }
//   fs::path local_path = file->local_dir;
//   local_path.append(file->filename);
//   // clear existing
//   fs::remove(local_path);
//   std::ofstream of(local_path, std::ios::binary);
//   this->cli->get(file->url, [&](const char *data, size_t len) {
//     of.write(data, len);
//     return true;
//   });
//   of.close();
// }
