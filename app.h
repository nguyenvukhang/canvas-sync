#ifndef CANVAS_SYNC_APP_H
#define CANVAS_SYNC_APP_H

#include <csync/csync.h>

#include "./ui_app.h"

#include <algorithm>
#include <filesystem>
#include <map>
#include <mutex>

#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QTimer>
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
class App;
}
QT_END_NAMESPACE

class App : public QMainWindow
{
  Q_OBJECT

  const QString GREEN = "#22c55e";
  const QString AMBER = "#fbbf24";
  const QString GRAY = "#e5e7eb";

  // Construct with base url, defaulting to live implementation of Canvas
  App(const QString &url, const QString &settings_file,
      QWidget *parent = nullptr)
      : QMainWindow(parent), ui(new Ui::App), settings(settings_file),
        canvas(new Canvas(url, &this->nw)){};

public:
  ~App() { delete ui; };
  // Construct with ICanvas*
  // For use in integration tests to mock a fake Canvas server.
  App(ICanvas *canvas, const QString &settings_file, QWidget *parent = nullptr)
      : QMainWindow(parent), ui(new Ui::App), canvas(canvas),
        settings(settings_file)
  {
    start();
  };

  // Construct with just a base url
  // For use in executable entry point
  App(const QString &base_url, QWidget *parent = nullptr)
      : App(base_url, "canvas-sync-settings.ini", parent)
  {
    start();
  };

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
  void change_token_clicked();
  void clear_clicked();
  // tree stuff
  void tree_view_track_folder(const QModelIndex &);
  void tree_view_pressed(const QModelIndex &);

public:
  std::string folder_name(const int folder_id);
  std::string course_name(const int course_id);
  void terminate(QNetworkReply *);
  void set_auth_state(bool);
  void show_updates();
  void check_auth(const QString &token);
  void gather_tracked();
  void ready_all();
  TreeModel *newTreeModel();

public:
  enum Action { FETCH, PULL };
  Action action;
  std::mutex tree_mtx, tracked_folders_mtx;
  QString start_dir = QDir::homePath();
  Settings settings;
  CSync csync;
  Ui::App *ui;

  // core business
  std::vector<FileTree> course_trees;
  std::vector<Course> user_courses;
  std::vector<Folder> tracked_folders;
  std::map<int, std::string> folder_names;

  ICanvas *canvas;
  QNetworkAccessManager nw;
};

#endif
