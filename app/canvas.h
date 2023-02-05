#ifndef CANVAS_H
#define CANVAS_H

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

class Canvas : public QObject
{
  Q_OBJECT

  QNetworkAccessManager nw;
  QString base_url, token_inner;

  void terminate(QNetworkReply *r);
  // fetch done, fetch expected, donwload done, download expected
  size_t count[4];
  enum Count { FETCH_DONE, FETCH_TOTAL, DOWNLOAD_DONE, DOWNLOAD_TOTAL };
  std::mutex count_mtx;

  QNetworkReply *get_full(const QString &url);
  QNetworkReply *get(const QString &url);
  QNetworkReply *get(const QString &fmt, const int &param);

public:
  Canvas(const QString &u) : base_url(u){};
  Canvas() = delete;

  const QString &token() const;
  void set_token(const QString &);
  bool has_network_err(QNetworkReply *r);
  void authenticate();
  void fetch_courses();
  void fetch_folders(const Course &);
  void fetch_files(const Folder &);
  void download(const File &, const Folder &);
  void reset_counts();
  void set_total_fetches(size_t);
  size_t increment_total_downloads(size_t);
  size_t increment_done_downloads();
  bool is_done_downloading();
  bool has_downloads();

signals:
  void authenticate_done(bool success);
  void fetch_courses_done(std::vector<Course>);
  void fetch_folders_done(const Course &, std::vector<Folder>);
  void fetch_files_done(const Folder &, std::vector<File>);
  void download_done(const size_t progress);
  void all_fetch_done();
  void all_download_done();
};
#endif // CANVAS_H
