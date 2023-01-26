#include "convert.h"

inline bool has(QJsonObject &j, const char *prop)
{
  return !j[prop].isNull();
}

// public methods

QJsonObject to_json(QNetworkReply *r)
{
  return QJsonDocument::fromJson(r->readAll()).object();
}

bool is_valid_profile(QJsonObject &j)
{
  qDebug() << "VALID?" << j["integration_id"];
  return has(j, "id") && has(j, "name") && has(j, "primary_email") &&
         has(j, "login_id") && has(j, "integration_id");
}
