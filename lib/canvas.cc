#include "canvas.h"

void ICanvas::reset_counts()
{
  for (size_t i = 0; i < 4; i++)
    count[i] = 0;
}

size_t ICanvas::increment_total_downloads(size_t n)
{
  size_t x;
  count_mtx.lock();
  count[DOWNLOAD_TOTAL] += n;
  x = count[DOWNLOAD_TOTAL];
  count_mtx.unlock();
  return x;
}

size_t ICanvas::increment_done_downloads()
{
  size_t x;
  count_mtx.lock();
  count[DOWNLOAD_DONE]++;
  x = count[DOWNLOAD_DONE];
  count_mtx.unlock();
  return x;
}

bool ICanvas::is_done_downloading()
{
  bool x;
  count_mtx.lock();
  x = count[DOWNLOAD_DONE] == count[DOWNLOAD_TOTAL];
  count_mtx.unlock();
  return x;
}

void write_file(const std::filesystem::path &path, const QByteArray &data)
{
  QString file = QString::fromStdString(path.string());
  if (std::filesystem::exists(path)) {
    QFile::remove(file);
  }
  QSaveFile f(file);
  f.open(QIODevice::WriteOnly);
  f.write(data);
  f.commit();
  f.deleteLater();
}

void Canvas::terminate(QNetworkReply *r)
{
  disconnect(r);
  r->deleteLater();
}

QNetworkReply *Canvas::get_full(const QString &url)
{
  QNetworkRequest r((QUrl(url)));
  if (!this->token_inner.isEmpty()) {
    r.setRawHeader("Authorization", ("Bearer " + this->token_inner).toUtf8());
  }
  return this->nw->get(r);
}

QNetworkReply *Canvas::get(const QString &url)
{
  return this->get_full(this->base_url + url);
}

QNetworkReply *Canvas::get(const QString &fmt, const int &param)
{
  return this->get_full(this->base_url + QString(fmt).arg(param));
}

bool Canvas::has_network_err(QNetworkReply *r)
{
  if (r->error() != QNetworkReply::NoError) {
    qDebug() << "Network Error: " << r->errorString() << '\n'
             << "Error Type: " << r->error() << '\n'
             << "from url:" << r->url();
    return true;
  }
  return false;
}

void Canvas::authenticate()
{
  std::cout << "First out to canvas servers!" << std::endl;
  QNetworkReply *r = this->get("/api/v1/users/self/profile");
  connect(r, &QNetworkReply::finished, this, [=]() {
    terminate(r);
    bool ok = false;

    if (r->error() == QNetworkReply::AuthenticationRequiredError) {
      qDebug() << "Failed auth with token:" << this->token();
      emit authenticate_done(false);
    } else if (r->error() != QNetworkReply::NoError) {
      emit authenticate_done(false);
    } else if (is_valid_profile(to_json(r).object())) {
      emit authenticate_done(true);
    } else {
      emit authenticate_done(false);
    }
  });
}

void Canvas::fetch_courses()
{
  auto *r = this->get("/api/v1/courses?per_page=1180");
  connect(r, &QNetworkReply::finished, this, [=]() {
    terminate(r);
    std::vector<Course> c =
        has_network_err(r) ? std::vector<Course>() : to_courses(to_json(r));
    emit fetch_courses_done(c);
  });
}

void Canvas::fetch_folders(const Course &c)
{
  auto r = this->get("/api/v1/courses/%1/folders?per_page=1180", c.id);
  connect(r, &QNetworkReply::finished, this, [=]() {
    terminate(r);
    std::vector<Folder> f =
        has_network_err(r) ? std::vector<Folder>() : to_folders(to_json(r));
    emit fetch_folders_done(c, f);
  });
}

void Canvas::fetch_files(const Folder &fo)
{
  auto r = this->get("/api/v1/folders/%1/files?per_page=1180", fo.id);
  connect(r, &QNetworkReply::finished, this, [=]() {
    terminate(r);
    std::vector<File> fi =
        has_network_err(r) ? std::vector<File>() : to_files(to_json(r));
    emit fetch_files_done(fo, fi);

    bool done = false;
    count_mtx.lock();
    done = ++count[FETCH_DONE] == count[FETCH_TOTAL];
    count_mtx.unlock();
    if (done) emit all_fetch_done();
  });
}

void Canvas::download(const File &file, const Folder &folder)
{
  auto r = this->get_full(file.url.c_str());
  connect(r, &QNetworkReply::finished, this, [=]() {
    terminate(r);
    // TODO: handle failed downloads
    if (has_network_err(r)) return;

    write_file(folder.local_dir / file.filename, r->readAll());

    bool done = false;
    count_mtx.lock();
    emit download_done(++count[DOWNLOAD_DONE]);
    done = count[DOWNLOAD_DONE] == count[DOWNLOAD_TOTAL];
    count_mtx.unlock();

    if (done) emit all_download_done();
  });
}
