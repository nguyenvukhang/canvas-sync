#ifndef CANVAS_SYNC_CANVAS_H
#define CANVAS_SYNC_CANVAS_H

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSaveFile>
#include <QString>

#include "convert.h"
#include "types.h"

#include <mutex>

class ICanvas : public QObject
{
  Q_OBJECT

protected:
  QString base_url, token_inner;
  size_t count[4];
  enum Count { FETCH_DONE, FETCH_TOTAL, DOWNLOAD_DONE, DOWNLOAD_TOTAL };
  std::mutex count_mtx;
  QNetworkAccessManager *nw;

public:
  ICanvas(const QString &u) : base_url(u){};
  ICanvas(const QString &u, QNetworkAccessManager *nw) : base_url(u), nw(nw){};
  ICanvas() = delete;

  virtual void authenticate() = 0;
  virtual void fetch_courses() = 0;
  virtual void fetch_folders(const Course &) = 0;
  virtual void fetch_files(const Folder &) = 0;
  virtual void download(const File &, const Folder &) = 0;

  const QString &token() const { return this->token_inner; };
  void set_token(const QString &t) { this->token_inner = t; };
  void set_total_fetches(size_t n) { count[FETCH_TOTAL] = n; };
  bool has_downloads() { return count[DOWNLOAD_TOTAL] > 0; };
  void reset_counts();
  size_t increment_total_downloads(size_t);
  size_t increment_done_downloads();
  bool is_done_downloading();

signals:
  void authenticate_done(bool success);
  void fetch_courses_done(std::vector<Course>);
  void fetch_folders_done(const Course &, std::vector<Folder>);
  void fetch_files_done(const Folder &, std::vector<File>);
  void download_done(const size_t progress);
  void all_fetch_done();
  void all_download_done();
};

class Canvas : public ICanvas
{
  Q_OBJECT

  void terminate(QNetworkReply *r);

  QNetworkReply *get_full(const QString &url);
  QNetworkReply *get(const QString &url);
  QNetworkReply *get(const QString &fmt, const int &param);

public:
  Canvas(const QString &u, QNetworkAccessManager *nw) : ICanvas(u, nw){};
  Canvas() = delete;

  bool has_network_err(QNetworkReply *r);
  void authenticate() override;
  void fetch_courses() override;
  void fetch_folders(const Course &) override;
  void fetch_files(const Folder &) override;
  void download(const File &, const Folder &) override;
};
#endif // CANVAS_H
