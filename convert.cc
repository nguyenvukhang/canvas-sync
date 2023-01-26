#include "convert.h"

const int BAD_ID = -1;

inline bool has(const QJsonObject &j, const char *prop)
{
  return !j[prop].isNull();
}

// public methods

QJsonDocument to_json(QNetworkReply *r)
{
  return QJsonDocument::fromJson(r->readAll());
}

template <typename T>
std::vector<T> to_vecq(const QJsonDocument &d,
                       std::function<T(const QJsonObject &)> f,
                       std::vector<std::string> keys)
{
  const QJsonArray a = d.array();
  std::vector<T> cs;
  size_t k = keys.size(), n = a.size();
  while (n-- > 0) {
    bool valid = true;
    QJsonObject t = a[n].toObject();
    for (size_t i = 0; i < k; i++)
      valid &= !t[keys[i].c_str()].isNull();
    if (valid)
      cs.push_back(f(t));
  }
  return cs;
}

bool is_valid_profile(const QJsonObject &j)
{
  return has(j, "id") && has(j, "name") && has(j, "primary_email") &&
         has(j, "login_id") && has(j, "integration_id");
}

Course to_course(const QJsonObject &j)
{
  Course c;
  c.id = j["id"].toInt(BAD_ID);
  c.name = j["name"].toString().toStdString();
  return c;
}

Folder to_folder(const QJsonObject &j)
{
  Folder f;
  f.id = j["id"].toInt(BAD_ID);
  f.name = j["name"].toString().toStdString();
  f.full_name = j["full_name"].toString().toStdString();
  return f;
}

File to_file(const QJsonObject &j)
{
  File f;
  f.id = j["id"].toInt(BAD_ID);
  f.folder_id = j["folder_id"].toInt();
  f.filename = j["filename"].toString().toStdString();
  f.url = j["url"].toString().toStdString();
  return f;
}

std::vector<Course> to_courses(const QJsonDocument &d)
{
  Course (*f)(const QJsonObject &j) = to_course;
  std::vector<std::string> keys = {"id", "name"};
  return to_vecq(d, (std::function<Course(const QJsonObject &)>)f, keys);
}

std::vector<Folder> to_folders(const QJsonDocument &d)
{
  Folder (*f)(const QJsonObject &j) = to_folder;
  std::vector<std::string> keys = {"id", "name", "full_name"};
  return to_vecq(d, (std::function<Folder(const QJsonObject &)>)f, keys);
}

std::vector<File> to_files(const QJsonDocument &d)
{
  File (*f)(const QJsonObject &j) = to_file;
  std::vector<std::string> keys = {"id", "folder_id", "filename", "url"};
  return to_vecq(d, (std::function<File(const QJsonObject &)>)f, keys);
}
