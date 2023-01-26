#include "types.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

QJsonDocument to_json(QNetworkReply *r);

bool is_valid_profile(const QJsonObject &j);
Course to_course(const QJsonObject &j);
Folder to_folder(const QJsonObject &j);
File to_file(const QJsonObject &j);

std::vector<Course> to_courses(const QJsonDocument &j);
std::vector<Folder> to_folders(const QJsonDocument &j);
std::vector<File> to_files(const QJsonDocument &j);
