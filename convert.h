#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

QJsonObject to_json(QNetworkReply *r);

bool is_valid_profile(QJsonObject &j);
