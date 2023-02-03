#ifndef CANVAS_H
#define CANVAS_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>

#include "convert.h"
#include "types.h"

class Canvas : public QObject
{
  Q_OBJECT
private:
  QNetworkAccessManager nw;
  QString base_url, token_inner;

private:
  void terminate(QNetworkReply *r);

public:
  Canvas(const QString &u) : base_url(u){};

  const QString &token() const;
  void set_token(const QString &);
  QNetworkReply *get_full(const QString &url);
  QNetworkReply *get(const QString &url);
  bool has_network_err(QNetworkReply *r);
  void fetch_courses();

signals:
  void fetch_courses_done(std::vector<Course>);
};
#endif // CANVAS_H
