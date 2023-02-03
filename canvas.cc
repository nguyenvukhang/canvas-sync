#include "canvas.h"

void Canvas::terminate(QNetworkReply *r)
{
  disconnect(r);
  r->deleteLater();
}

const QString &Canvas::token() const
{
  return this->token_inner;
}

void Canvas::set_token(const QString &token)
{
  this->token_inner = token;
}

QNetworkReply *Canvas::get_full(const QString &url)
{
  QNetworkRequest r((QUrl(url)));
  if (!this->token_inner.isEmpty()) {
    r.setRawHeader("Authorization", ("Bearer " + this->token_inner).toUtf8());
  }
  return this->nw.get(r);
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
    if (done)
      emit all_fetch_done();
  });
}

void Canvas::download(const File &f,
                      const std::function<void(QNetworkReply *)> write)
{
  auto r = this->get_full(f.url.c_str());
  connect(r, &QNetworkReply::finished, this, [=]() {
    terminate(r);
    if (has_network_err(r))
      return;
    write(r);

    bool done = false;
    count_mtx.lock();
    emit download_done(++count[DOWNLOAD_DONE]);
    done = count[DOWNLOAD_DONE] == count[DOWNLOAD_TOTAL];
    count_mtx.unlock();
    if (done)
      emit all_download_done();
  });
}

void Canvas::reset_counts()
{
  for (size_t i = 0; i < 4; i++)
    count[i] = 0;
}

void Canvas::set_total_fetches(size_t n)
{
  count[FETCH_TOTAL] = n;
}

size_t Canvas::increment_total_downloads(size_t n)
{
  size_t x;
  count_mtx.lock();
  count[DOWNLOAD_TOTAL] += n;
  x = count[DOWNLOAD_TOTAL];
  count_mtx.unlock();
  return x;
}

size_t Canvas::increment_done_downloads()
{
  size_t x;
  count_mtx.lock();
  count[DOWNLOAD_DONE]++;
  x = count[DOWNLOAD_DONE];
  count_mtx.unlock();
  return x;
}

bool Canvas::is_done_downloading()
{
  bool x;
  count_mtx.lock();
  x = count[DOWNLOAD_DONE] == count[DOWNLOAD_TOTAL];
  count_mtx.unlock();
  return x;
}
