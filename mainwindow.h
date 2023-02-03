#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <algorithm>
#include <filesystem>
#include <map>
#include <mutex>

#include "canvas.h"
#include "convert.h"
#include "filetree.h"
#include "tree.h"
#include "tree_model.h"
#include "types.h"
#include "ui_mainwindow.h"
#include "updates.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
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

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void pull_clicked();
  void fetch_clicked();
  void changeToken_clicked();
  // network stuff
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
  void enable_pull(const QString & = "Pull");
  void disable_pull(const QString & = "Pulling...");
  void enable_fetch(const QString & = "Fetch");
  void disable_fetch(const QString & = "Fetching...");
  void refresh_tree_data();
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
  bool updates_done;
  std::mutex tree_mtx, update_mtx, dl_e_mtx, dl_r_mtx;
  QSettings settings;
  std::vector<Update> updates;
  int expected_downloads, received_downloads;
  std::vector<FileTree> course_trees;
  std::vector<Course> user_courses;
  std::map<int, std::string> folder_names;
  Ui::MainWindow *ui;
  QString start_dir = QDir::homePath();

private:
  Canvas canvas;
};
#endif // MAINWINDOW_H
