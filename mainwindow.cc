#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      settings(
          QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
              "/canvas-sync-settings.ini",
          QSettings::IniFormat)
{
  ui->setupUi(this);

  // text inputs
  connect(ui->lineEdit_accessToken, &QLineEdit::textChanged, this,
          &MainWindow::accessToken_textChanged);

  // buttons
  connect(ui->pushButton_pull, &QPushButton::clicked, this,
          &MainWindow::pull_clicked);
  connect(ui->pushButton_fetch, &QPushButton::clicked, this,
          &MainWindow::fetch_clicked);
  connect(ui->pushButton_changeToken, &QPushButton::clicked, this,
          &MainWindow::changeToken_clicked);

  connect(ui->treeView, &ClickableTreeView::expanded, this,
          &MainWindow::treeView_expanded);
  connect(ui->treeView, &ClickableTreeView::collapsed, this,
          &MainWindow::treeView_collapsed);

  connect(ui->treeView, &ClickableTreeView::clicked, this,
          &MainWindow::treeView_clicked);

  connect(ui->treeView, &ClickableTreeView::cleared, this,
          &MainWindow::treeView_cleared);
  connect(ui->treeView, &ClickableTreeView::track_folder, this,
          &MainWindow::track_folder_requested);

  // scripted views
  ui->pushButton_changeToken->setHidden(true);
  ui->treeView->setColumnHidden(FOLDER_ID, true);
  ui->progressBar->setHidden(true);
  ui->treeView->setModel(newTreeModel());
  ui->guideText->hide();

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
  this->disable_pull();
  this->received_downloads = 0;
  this->expected_downloads = 0;
  this->updates_done = false;
  this->updates.clear();
  ui->progressBar->setMaximum(0);
  ui->progressBar->setValue(0);
  std::vector<Update> tracked_folders = gather_tracked();
  this->fetch_folder_files(tracked_folders, true);
}

void MainWindow::fetch_clicked()
{
  this->disable_fetch();
  this->updates.clear();
  std::vector<Update> tracked_folders = gather_tracked();
  this->fetch_folder_files(tracked_folders, false);
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
  if (has_network_err(r))
    return;
  auto j = to_json(r);
  this->set_auth_state(is_valid_profile(j.object()));
  this->fetch_courses();
}

void MainWindow::courses_fetched()
{
  QNetworkReply *r = (QNetworkReply *)this->sender();
  if (has_network_err(r))
    return;
  auto j = to_json(r);
  this->user_courses = to_courses(j);
  for (auto c : this->user_courses)
    this->fetch_course_folders(c);
}

void MainWindow::course_folders_fetched(const Course &c)
{
  QNetworkReply *r = (QNetworkReply *)this->sender();
  if (has_network_err(r))
    return;
  std::vector<Folder> f = to_folders(to_json(r));
  FileTree t(&c, f);
  tree_mtx.lock();
  for (auto f : f) {
    this->folder_names.insert(std::pair(f.id, f.full_name));
  }
  this->course_trees.push_back(t);
  this->refresh_tree();
  tree_mtx.unlock();
  ui->guideText->setHidden(!this->gather_tracked().empty());
}

void MainWindow::folder_files_fetched(Update u, size_t total_expected_updates,
                                      bool download)
{
  QNetworkReply *r = (QNetworkReply *)this->sender();
  if (has_network_err(r))
    return;
  std::vector<File> f = to_files(to_json(r));
  remove_existing_files(&f, u.local_dir);

  size_t n = f.size();
  while (n-- > 0)
    f[n].local_dir = u.local_dir;

  u.files = std::move(f);
  // u.files now contains a list of files that the user does not have

  bool updates_done = false, has_downloads = false;

  if (download) {
    dl_e_mtx.lock();
    this->expected_downloads += u.files.size();
    has_downloads = this->expected_downloads > 0;
    ui->progressBar->setMaximum(this->expected_downloads);
    dl_e_mtx.unlock();
    this->download_files(u.files);
    ui->progressBar->setHidden(!has_downloads);
  }

  update_mtx.lock();
  this->updates.push_back(std::move(u));
  this->updates_done = this->updates.size() == total_expected_updates;
  updates_done = this->updates_done;
  update_mtx.unlock();

  if (updates_done && (!download || !has_downloads)) {
    this->show_updates();
  }
}

void MainWindow::file_downloaded(File f)
{
  QNetworkReply *r = (QNetworkReply *)this->sender();
  if (has_network_err(r))
    return;
  std::filesystem::path local_path = f.local_dir;
  local_path.append(f.filename);

  QString filepath = QString::fromStdString(local_path.string());

  QFile::remove(filepath);
  QSaveFile file(filepath);
  file.open(QIODevice::WriteOnly);
  file.write(r->readAll());
  file.commit();

  bool show_downloads = false;
  dl_r_mtx.lock();
  received_downloads += 1;
  ui->progressBar->setValue(received_downloads);
  show_downloads = updates_done && received_downloads == expected_downloads;
  dl_r_mtx.unlock();

  if (show_downloads) {
    ui->progressBar->setHidden(true);
    this->enable_pull();
    show_updates();
  }
}

/// TREEVIEW SLOTS ---

void MainWindow::treeView_clicked(const QModelIndex &index)
{
}

void MainWindow::treeView_cleared(const QModelIndex &index)
{
  settings.remove(get_id(index));
  settings.sync();
  ui->guideText->setHidden(!this->gather_tracked().empty());
}

void MainWindow::treeView_expanded(const QModelIndex &index)
{
  ui->treeView->resizeColumnToContents(0);
}

void MainWindow::treeView_collapsed(const QModelIndex &index)
{
  ui->treeView->resizeColumnToContents(0);
}

void MainWindow::track_folder_requested(const QModelIndex &index)
{
  if (!index.parent().isValid())
    return;

  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::Directory);
  QString home = QDir::homePath();
  dialog.setDirectory(this->start_dir != home ? this->start_dir : home);
  dialog.setWindowTitle("Target for " + get_ancestry(index, " / "));
  int result = dialog.exec();
  grabKeyboard();

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

    ui->guideText->hide();

    QDir selected_dir = QDir::fromNativeSeparators(local_dir);
    selected_dir.cdUp();
    this->start_dir = selected_dir.path();
  }
}

//////////////////////////////////////////////////////////////////////
/// HELPER FUNCTIONS
//////////////////////////////////////////////////////////////////////

std::string MainWindow::folder_name(const int folder_id)
{
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

void MainWindow::terminate(QNetworkReply *r)
{
  disconnect(r);
  r->deleteLater();
}

void MainWindow::enable_pull()
{
  ui->pushButton_pull->setText("Pull");
  ui->pushButton_pull->setEnabled(true);
}

void MainWindow::disable_pull()
{
  ui->pushButton_pull->setText("Pulling...");
  ui->pushButton_pull->setEnabled(false);
}

void MainWindow::enable_fetch()
{
  ui->pushButton_fetch->setText("Fetch");
  ui->pushButton_fetch->setEnabled(true);
}

void MainWindow::disable_fetch()
{
  ui->pushButton_fetch->setText("Fetching...");
  ui->pushButton_fetch->setEnabled(false);
}

void MainWindow::refresh_tree()
{
  TreeModel *model = newTreeModel();
  FileTree t;
  t.insert_trees(this->course_trees);
  insert(model->item(0), &t, &settings);
  ui->treeView->setModel(model);
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
    this->settings.setValue("access-token", this->token);
    settings.sync();
    return;
  }
  ui->label_authenticationStatus->setText("unauthenticated");
  this->ui->pushButton_changeToken->setHidden(true);
}

void MainWindow::show_updates()
{
  // update state before showing update window
  this->enable_fetch();
  this->enable_pull();
  ui->progressBar->setHidden(true);

  QString buffer = "", tmp = "";
  int prev_course = -1;
  for (auto u : this->updates) {
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
  this->token = token;
  ui->treeView->setModel(newTreeModel());
  this->course_trees.clear();
  QNetworkReply *a = this->get("/api/v1/users/self/profile");
  connect(a, &QNetworkReply::finished, this, [=]() {
    check_auth_fetched();
    terminate(a);
  });
}

void MainWindow::fetch_courses()
{
  QNetworkReply *a = this->get("/api/v1/courses?per_page=1180");
  connect(a, &QNetworkReply::finished, this, [=]() {
    courses_fetched();
    terminate(a);
  });
}

void MainWindow::fetch_course_folders(const Course &c)
{
  QNetworkReply *a = this->get("/api/v1/courses/" + QString::number(c.id) +
                               "/folders?per_page=1180");
  connect(a, &QNetworkReply::finished, this, [=]() {
    course_folders_fetched(c);
    terminate(a);
  });
}

void MainWindow::fetch_folder_files(Update u, size_t total_expected_updates,
                                    bool download)
{
  QNetworkReply *a =
      this->get("/api/v1/folders/" + QString::number(u.folder_id) +
                "/files?per_page=1180");
  connect(a, &QNetworkReply::finished, this, [=]() {
    folder_files_fetched(std::move(u), total_expected_updates, download);
    terminate(a);
  });
}

void MainWindow::fetch_folder_files(std::vector<Update> u, bool download)
{
  for (Update tracked_folder : u) {
    tracked_folder.remote_dir = this->folder_name(tracked_folder.folder_id);
    this->fetch_folder_files(tracked_folder, u.size(), download);
  }
}

void MainWindow::download_file(File f)
{
  if (!std::filesystem::exists(f.local_dir)) {
    std::filesystem::create_directories(f.local_dir);
  }
  QNetworkReply *a = this->get_full(QString::fromStdString(f.url));
  connect(a, &QNetworkReply::finished, this, [=]() {
    file_downloaded(std::move(f));
    terminate(a);
  });
}

void MainWindow::download_files(std::vector<File> f)
{
  for (File f : f)
    this->download_file(f);
}

std::vector<Update> MainWindow::gather_tracked()
{
  TreeModel *model = ui->treeView->model();
  size_t n = model->childrenCount();
  std::vector<Update> all;
  while (n-- > 0) {
    std::vector<Update> u = resolve_all_folders(model->item(n));
    all.reserve(all.size() + std::distance(u.begin(), u.end()));
    all.insert(all.end(), u.begin(), u.end());
  }
  return all;
}
