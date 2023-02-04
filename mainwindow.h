#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <algorithm>
#include <filesystem>
#include <map>
#include <mutex>

#include "canvas.h"
#include "convert.h"
#include "filetree.h"
#include "tree_model.h"
#include "types.h"
#include "updates.h"

#include "ui_mainwindow.h"
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

  void connect_buttons();
  void connect_tree();
  void connect_canvas();
  void prefetch();
  void fetch(const std::vector<Folder> &f);

private slots:
  void pull_clicked();
  void fetch_clicked();
  void changeToken_clicked();
  // tree stuff
  void treeView_cleared(const QModelIndex &);
  void treeView_trackFolder(const QModelIndex &);

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
  std::vector<Folder> gather_tracked();
  TreeModel *newTreeModel();

public:
  enum Action { FETCH, PULL };
  Action action;
  std::mutex tree_mtx, tracked_folders_mtx;
  QString start_dir = QDir::homePath();
  QSettings settings;
  Ui::MainWindow *ui;

  // core business
  std::vector<FileTree> course_trees;
  std::vector<Course> user_courses;
  std::vector<Folder> tracked_folders;
  std::map<int, std::string> folder_names;

private:
  Canvas canvas;
};
#endif // MAINWINDOW_H
