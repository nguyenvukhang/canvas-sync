#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <csrv.h>
#include <mutex>

class Network : public QNetworkAccessManager
{
  Q_OBJECT

private:
  std::string base_url;

private slots:
  void replyFinished(QNetworkReply *r)
  {
    qDebug() << r->readAll();
  }

public:
  Network(std::string base_url) : base_url(base_url){};
  void get(std::string &url)
  {
  }
  QNetworkReply *get(QNetworkRequest q)
  {
    return QNetworkAccessManager::get(q);
  }
  void ping()
  {
    QUrl qrl("https://nguyenvukhang.com/api/nus");
    connect(this, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(replyFinished(QNetworkReply *)));
    QNetworkRequest r(qrl);
    r.setRawHeader("Authorization", "Bearer token");
    this->get(r);
  }
};

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

private:
  QNetworkRequest req(std::string url)
  {
    QUrl qrl((this->base_url + url).c_str());
    QNetworkRequest r(qrl);
    r.setRawHeader("Authorization",
                   QByteArray::fromStdString("Bearer " + token));
    return r;
  }

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void pull_clicked();
  void fetch_clicked();
  void changeToken_clicked();
  void accessToken_textChanged(const QString &);
  // network stuff
  void check_auth_fetched(QNetworkReply *);
  void courses_fetched(QNetworkReply *);
  void course_folders_fetched(const Course &);
  // tree stuff
  void treeView_clicked(const QModelIndex &);
  void treeView_doubleClicked(const QModelIndex &);
  void treeView_cleared(const QModelIndex &);
  void treeView_expanded(const QModelIndex &);
  void treeView_collapsed(const QModelIndex &);

public:
  bool has_fetched_course(int course_id);
  void refresh_tree();
  void set_auth_state(bool);
  void show_updates(const std::vector<Update> &);
  void check_auth(const QString &token);
  void fetch_courses();
  void fetch_course_folders(const Course &);

  // data
  bool authenticated = false;
  std::mutex tree_mtx;
  std::vector<int> fetched_course_ids;
  QSettings settings;
  FileTree tree;
  std::vector<Course> user_courses;
  std::string token, base_url = "https://canvas.nus.edu.sg";
  Ui::MainWindow *ui;
  Network nw;
  QString start_dir = QDir::homePath();
};
#endif // MAINWINDOW_H
