#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <algorithm>
#include <filesystem>
#include <map>
#include <mutex>

#include "convert.h"
#include "filetree.h"
#include "tree.h"
#include "tree_model.h"
#include "types.h"
#include "ui_mainwindow.h"
#include "updates.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>

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
  QNetworkReply *get(QString url)
  {
    return this->get_full(this->base_url + url);
  }

  QNetworkReply *get_full(QString url)
  {
    QNetworkRequest r(*new QUrl(url));
    r.setRawHeader("Authorization", ("Bearer " + this->token).toUtf8());
    return this->nw.get(r);
  }

  bool has_network_err(QNetworkReply *r)
  {
    if (r->error() != QNetworkReply::NoError) {
      qDebug() << "Network Error: " << r->errorString();
      return true;
    }
    return false;
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
  void check_auth_fetched();
  void courses_fetched();
  void course_folders_fetched(const Course &);
  void folder_files_fetched(Update u, size_t c, bool download);
  void file_downloaded(File f);
  // tree stuff
  void treeView_clicked(const QModelIndex &);
  void treeView_cleared(const QModelIndex &);
  void treeView_expanded(const QModelIndex &);
  void treeView_collapsed(const QModelIndex &);
  void track_folder_requested(const QModelIndex &);

public:
  std::string folder_name(const int folder_id);
  std::string course_name(const int course_id);
  void terminate(QNetworkReply *);
  void enable_pull();
  void disable_pull();
  void enable_fetch();
  void disable_fetch();
  void refresh_tree();
  void set_auth_state(bool);
  void show_updates();
  void check_auth(const QString &token);
  void fetch_courses();
  void fetch_course_folders(const Course &);
  void fetch_folder_files(Update u, size_t c, bool download);
  void fetch_folder_files(std::vector<Update>, bool download);
  void download_file(File);
  void download_files(std::vector<File>);
  std::vector<Update> gather_tracked();

public:
  bool authenticated = false, updates_done;
  std::mutex tree_mtx, update_mtx, dl_e_mtx, dl_r_mtx;
  QSettings settings;
  std::vector<Update> updates;
  int expected_downloads, received_downloads;
  std::vector<FileTree> course_trees;
  std::vector<Course> user_courses;
  std::map<int, std::string> folder_names;
  QString token, base_url = "https://canvas.nus.edu.sg";
  Ui::MainWindow *ui;
  QString start_dir = QDir::homePath();

private:
  QNetworkAccessManager nw;
};
#endif // MAINWINDOW_H
