#include "mainwindow.h"
#include "convert.h"
#include "filetree.h"
#include "tree.h"
#include "tree_model.h"
#include "ui_mainwindow.h"
#include "updates.h"
#include <algorithm>
#include <csrv.h>
#include <filesystem>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFuture>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QPromise>
#include <QtConcurrent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      nw("https://canvas.nus.edu.sg")
{
  ui->setupUi(this);

  // text inputs
  connect(ui->lineEdit_accessToken, SIGNAL(textChanged(const QString &)), this,
          SLOT(accessToken_textChanged(const QString &)));

  // buttons
  connect(ui->pushButton_pull, SIGNAL(clicked()), this, SLOT(pull_clicked()));
  connect(ui->pushButton_fetch, SIGNAL(clicked()), this, SLOT(fetch_clicked()));
  connect(ui->pushButton_changeToken, SIGNAL(clicked()), this,
          SLOT(changeToken_clicked()));

  connect(ui->treeView, SIGNAL(expanded(const QModelIndex &)), this,
          SLOT(treeView_expanded(const QModelIndex &)));
  connect(ui->treeView, SIGNAL(collapsed(const QModelIndex &)), this,
          SLOT(treeView_collapsed(const QModelIndex &)));
  connect(ui->treeView, SIGNAL(clicked(const QModelIndex &)), this,
          SLOT(treeView_clicked(const QModelIndex &)));
  connect(ui->treeView, SIGNAL(doubleClicked(const QModelIndex &)), this,
          SLOT(treeView_doubleClicked(const QModelIndex &)));
  connect(ui->treeView, SIGNAL(cleared(const QModelIndex &)), this,
          SLOT(treeView_cleared(const QModelIndex &)));

  ui->pushButton_changeToken->setHidden(true);
  ui->treeView->setModel(newTreeModel());

  if (settings.contains("access-token")) {
    this->check_auth(settings.value("access-token").toString());
  }
}

MainWindow::~MainWindow()
{
  delete ui;
}

//////////////////////////////////////////////////////////////////////
/// SLOTS
//////////////////////////////////////////////////////////////////////

void MainWindow::pull_clicked()
{
  this->updates.clear();
  std::vector<Update> all = gather_tracked();
  size_t c = all.size();
  for (Update u : all) {
    this->fetch_folder_files(u, c, true);
  }
}

void MainWindow::fetch_clicked()
{
  ui->pushButton_fetch->setDisabled(true);
  ui->pushButton_fetch->setText("Fetching...");
  this->updates.clear();
  std::vector<Update> all = gather_tracked();
  size_t c = all.size();
  for (Update u : all) {
    this->fetch_folder_files(u, c, false);
  }
  ui->pushButton_fetch->setText("Fetch");
  ui->pushButton_fetch->setDisabled(false);
  qDebug() << "Fetch click done!";
}

void MainWindow::changeToken_clicked()
{
  this->token = "";
  this->ui->lineEdit_accessToken->setText("");
  this->ui->lineEdit_accessToken->setReadOnly(false);
  this->ui->lineEdit_accessToken->setDisabled(false);
  this->ui->pushButton_changeToken->setHidden(true);
  this->ui->label_authenticationStatus->setText("unauthenticated");
}

void MainWindow::accessToken_textChanged(const QString &input)
{
  this->check_auth(input);
}

/// NETWORK SLOTS ---

void MainWindow::check_auth_fetched()
{
  QNetworkReply *r = (QNetworkReply *)this->sender();
  disconnect(r);
  if (r->error() != QNetworkReply::NoError) {
    r->deleteLater();
    qDebug() << "Network Error: " << r->errorString();
    return;
  }
  auto j = to_json(r);
  this->set_auth_state(is_valid_profile(j.object()));
  this->fetch_courses();
  r->deleteLater();
}

void MainWindow::courses_fetched()
{
  QNetworkReply *r = (QNetworkReply *)this->sender();
  disconnect(r);
  if (r->error() != QNetworkReply::NoError) {
    r->deleteLater();
    qDebug() << "Network Error: " << r->errorString();
    return;
  }
  auto j = to_json(r);
  this->user_courses = to_courses(j);
  for (auto c : this->user_courses)
    this->fetch_course_folders(c);
  r->deleteLater();
}

void MainWindow::course_folders_fetched(const Course &c)
{
  QNetworkReply *r = (QNetworkReply *)this->sender();
  disconnect(r);
  if (r->error() != QNetworkReply::NoError) {
    r->deleteLater();
    qDebug() << "Network Error: " << r->errorString();
    return;
  }
  std::vector<Folder> f = to_folders(to_json(r));
  FileTree t(&c, f);
  tree_mtx.lock();
  for (auto f : f) {
    this->folder_names.insert(std::pair(f.id, f.full_name));
  }
  this->course_trees.push_back(t);
  this->refresh_tree();
  tree_mtx.unlock();
  r->deleteLater();
}

void MainWindow::folder_files_fetched(Update u, size_t c, bool download)
{
  QNetworkReply *r = (QNetworkReply *)this->sender();
  disconnect(r);
  if (r->error() != QNetworkReply::NoError) {
    r->deleteLater();
    qDebug() << "Network Error: " << r->errorString();
    return;
  }
  std::vector<File> f = to_files(to_json(r));
  // merge data into master updates
  size_t fc = f.size();
  std::filesystem::path local_dir = u.local_dir;
  u.remote_dir = this->folder_name(u.folder_id);
  for (int j = 0; j < fc; j++) {
    if (!std::filesystem::exists(local_dir / f[j].filename)) {
      f[j].local_dir = u.local_dir;
      u.files.push_back(f[j]);
    }
  }
  qDebug() << "download?" << download;
  if (download) {
    for (auto f : u.files) {
      qDebug() << "sending download:" << f.filename.c_str();
      this->download_file(f);
    }
  }
  bool done = false;
  update_mtx.lock();
  this->updates.push_back(std::move(u));
  done = updates.size() == c;
  update_mtx.unlock();
  if (done)
    show_updates(this->updates);
  r->deleteLater();
}

void MainWindow::file_downloaded(File f)
{
  QNetworkReply *r = (QNetworkReply *)this->sender();
  disconnect(r);
  if (r->error() != QNetworkReply::NoError) {
    r->deleteLater();
    qDebug() << "Network Error: " << r->errorString();
    return;
  }
  std::filesystem::path local_path = f.local_dir;
  local_path.append(f.filename);
  QFile::remove(local_path.c_str());
  QSaveFile file(local_path.c_str());
  file.open(QIODevice::WriteOnly);
  file.write(r->readAll());
  file.commit();
  r->deleteLater();
}

/// TREEVIEW SLOTS ---

void MainWindow::treeView_clicked(const QModelIndex &index)
{
  TreeModel *model = ui->treeView->model();
  qDebug() << "[ DEBUG ]\n";
  qDebug() << "id:     " << get_id(index);
  qDebug() << "remote: " << get_remote_dir(index);
  qDebug() << "local : " << get_local_dir(index);
  int count = index.model()->children().size();
  qDebug() << "children: " << get_id(model->index(0, 0, index));
  qDebug() << "count:    " << count;
  for (auto a : this->updates) {
    qDebug() << "update: " << folder_name(a.folder_id).c_str();
    for (auto a : a.files) {
      qDebug() << "file: " << a.filename.c_str();
    }
  }
}

void MainWindow::treeView_doubleClicked(const QModelIndex &index)
{
  // don't do anything to root bois.
  if (!index.parent().isValid()) {
    return;
  }
  // go home for non-localdir bois
  if (index.column() != 1)
    return;

  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::Directory);
  dialog.setOption(QFileDialog::DontUseNativeDialog);
  QString home = QDir::homePath();
  dialog.setDirectory(this->start_dir != home ? this->start_dir : home);
  dialog.setWindowTitle("Select target for " + get_ancestry(index, " / "));
  int result = dialog.exec();

  TreeItem *item = ui->treeView->model()->itemFromIndex(index);

  QString local_dir = dialog.selectedFiles()[0];

  // only set the value if a value was actually chosen.
  if (result == 1) {
    // clears children and parent tracked folders because this might cause
    // conflicts in downloads.

    // clear children maps
    on_all_children(item, [&](TreeItem *child) {
      child->setData(TreeCol::LOCAL_DIR, "");
      settings.remove(get_id(*child));
    });

    // clear parent maps
    on_all_parents(item, [&](TreeItem *parent) {
      parent->setData(TreeCol::LOCAL_DIR, "");
      settings.remove(get_id(*parent));
    });

    // update itself
    item->setData(1, local_dir);
    settings.setValue(get_id(index), local_dir);
    settings.sync();

    QDir selected_dir = QDir::fromNativeSeparators(local_dir);
    selected_dir.cdUp();
    this->start_dir = selected_dir.path();
  }
}

void MainWindow::treeView_cleared(const QModelIndex &index)
{
  settings.remove(get_id(index));
  settings.sync();
}

void MainWindow::treeView_expanded(const QModelIndex &index)
{
  fix_tree(ui);
}

void MainWindow::treeView_collapsed(const QModelIndex &index)
{
  fix_tree(ui);
}

//////////////////////////////////////////////////////////////////////
/// HELPER FUNCTIONS
//////////////////////////////////////////////////////////////////////

std::string MainWindow::folder_name(const int folder_id)
{
  qDebug() << "requested" << folder_id;
  return this->folder_names.at(folder_id);
}

std::string MainWindow::course_name(const int course_id)
{
  size_t i = this->user_courses.size();
  while (i-- > 0)
    if (this->user_courses[i].id == course_id)
      return this->user_courses[i].name;
  return "[course not found]";
}

void MainWindow::refresh_tree()
{
  TreeModel *model = newTreeModel();
  FileTree t;
  t.insert_trees(this->course_trees);
  insert(model->item(0), &t, &settings);
  ui->treeView->setModel(model);
  ui->treeView->resizeColumnToContents(0);
  expand_tracked(ui->treeView);
  // FIXME: after debugging, hide ids from user
  ui->treeView->setColumnHidden(FOLDER_ID, false);
}

void MainWindow::set_auth_state(bool authenticated)
{
  qDebug() << "MainWindow::set_auth_state -> " << authenticated;
  this->authenticated = authenticated;
  if (authenticated) {
    ui->label_authenticationStatus->setText("authenticated!");
    // disable token entry
    ui->lineEdit_accessToken->setReadOnly(true);
    ui->lineEdit_accessToken->setDisabled(true);
    // show edit token button, in case the user wants to change it
    this->ui->pushButton_changeToken->setHidden(false);
    this->settings.setValue("access-token",
                            QString::fromStdString(this->token));
    return;
  }
  ui->label_authenticationStatus->setText("unauthenticated");
  this->ui->pushButton_changeToken->setHidden(true);
}

void MainWindow::show_updates(const std::vector<Update> &u)
{
  qDebug() << "SHOWING UPDATES";
  QString buffer = "", tmp = "";
  int prev_course = -1;
  for (auto u : u) {
    if (u.course_id != prev_course) {
      if (!tmp.isEmpty()) {
        buffer.push_back("## ");
        buffer.push_back(course_name(prev_course).c_str());
        buffer.push_back('\n');
        buffer.push_back(tmp);
        tmp.clear();
      }
      prev_course = u.course_id;
    }
    if (!u.files.empty()) {
      tmp.push_back("#### ");
      tmp.push_back(u.remote_dir.c_str());
      tmp.push_back('\n');
    }
    for (auto f : u.files) {
      tmp.push_back(f.filename.c_str());
      tmp.push_back('\n');
      tmp.push_back('\n');
    }
  }
  buffer += tmp;
  if (buffer.isEmpty()) {
    QMessageBox::information(this, "Update", "All up to date!");
  } else {
    Updates w;
    w.setText(buffer);
    w.setModal(true);
    w.exec();
  }
}

void MainWindow::check_auth(const QString &token)
{
  this->token = token.toStdString();
  ui->treeView->setModel(newTreeModel());
  this->course_trees.clear();
  QNetworkRequest r = req("/api/v1/users/self/profile");
  QNetworkReply *a = this->nw.get(r);
  connect(a, &QNetworkReply::finished, this, &MainWindow::check_auth_fetched);
}

void MainWindow::fetch_courses()
{
  QNetworkRequest r = req("/api/v1/courses");
  QNetworkReply *a = this->nw.get(r);
  connect(a, &QNetworkReply::finished, this, &MainWindow::courses_fetched);
}

void MainWindow::fetch_course_folders(const Course &c)
{
  std::string url = "/api/v1/courses/" + std::to_string(c.id) + "/folders";
  QNetworkRequest r = req(url);
  QNetworkReply *a = this->nw.get(r);
  connect(a, &QNetworkReply::finished, this,
          [=]() { this->course_folders_fetched(c); });
}

void MainWindow::fetch_folder_files(Update u, size_t c, bool download)
{
  std::string url = "/api/v1/folders/" + std::to_string(u.folder_id) + "/files";
  QNetworkRequest r = req(url);
  QNetworkReply *a = this->nw.get(r);
  connect(a, &QNetworkReply::finished, this,
          [=]() { this->folder_files_fetched(std::move(u), c, download); });
}

void MainWindow::download_file(File f)
{
  if (!std::filesystem::exists(f.local_dir)) {
    std::filesystem::create_directories(f.local_dir);
    qDebug() << "Download's target directory does not exist."
             << f.local_dir.c_str();
    return;
  }
  QNetworkRequest r = download_req(f.url);
  QNetworkReply *a = this->nw.get(r);
  connect(a, &QNetworkReply::finished, this,
          [=]() { this->file_downloaded(std::move(f)); });
}

std::vector<Update> MainWindow::gather_tracked()
{
  TreeModel *model = ui->treeView->model();
  size_t n = model->childrenCount();
  std::vector<Update> all;
  while (n-- > 0) {
    std::vector<Update> u = resolve_all_folders(model->item(n));
    for (auto i : u)
      all.push_back(i);
  }
  return all;
}
