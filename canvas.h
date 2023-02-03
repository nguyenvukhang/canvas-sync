#ifndef CANVAS_H
#define CANVAS_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>

class Canvas : public QObject
{
  Q_OBJECT
private:
  QNetworkAccessManager nw;
  QString base_url, token_inner;

public:
  Canvas(const QString &u) : base_url(u){};

  const QString &token() const;
  void set_token(const QString &);
  QNetworkReply *get_full(const QString &url);
  QNetworkReply *get(const QString &url);
  bool has_network_err(QNetworkReply *r);
};
#endif // CANVAS_H
