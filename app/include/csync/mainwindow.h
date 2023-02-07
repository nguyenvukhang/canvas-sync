#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <mutex>

#include "canvas.h"
#include "convert.h"
#include "filetree.h"
#include "tree_model.h"
#include "types.h"

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
  // Construct with base url, defaulting to live implementation of Canvas
  MainWindow(const QString &url, const QString &settings_file,
             QWidget *parent = nullptr)
      : QMainWindow(parent), ui(new Ui::MainWindow),
        settings(settings_path + '/' + settings_file, QSettings::IniFormat),
        canvas(new Canvas(url, &this->nw)){};

public:
  // Construct with ICanvas*
  // For use in integration tests to mock a fake Canvas server.
  MainWindow(ICanvas *canvas, const QString &settings_file,
             QWidget *parent = nullptr)
      : QMainWindow(parent), ui(new Ui::MainWindow),
        settings(settings_path + '/' + settings_file, QSettings::IniFormat),
        canvas(canvas)
  {
    start();
  };

  // Construct with just a base url
  // For use in executable entry point
  MainWindow(const QString &base_url, QWidget *parent = nullptr)
      : MainWindow(base_url, "canvas-sync-settings.ini", parent)
  {
    start();
  };

  ~MainWindow() { delete ui; };

  void start();
  void setup_ui();
  void connect_ui();
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
  const static QString settings_path;
  Ui::MainWindow *ui;

  // core business
  std::vector<FileTree> course_trees;
  std::vector<Course> user_courses;
  std::vector<Folder> tracked_folders;
  std::map<int, std::string> folder_names;

  ICanvas *canvas;
  QNetworkAccessManager nw;
};
#endif // MAINWINDOW_H
