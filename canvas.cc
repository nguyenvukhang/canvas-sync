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
  QNetworkReply *r = this->get("/api/v1/courses?per_page=1180");
  connect(r, &QNetworkReply::finished, this, [=]() {
    std::vector<Course> c =
        has_network_err(r) ? std::vector<Course>() : to_courses(to_json(r));
    emit this->fetch_courses_done(c);
    terminate(r);
  });
}
