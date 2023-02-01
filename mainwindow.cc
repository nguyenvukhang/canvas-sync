#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      nw("https://canvas.nus.edu.sg"),
      settings(
          QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
              "/canvas-sync-settings.ini",
          QSettings::IniFormat)
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
  std::vector<Update> all = gather_tracked();
  size_t c = all.size();
  for (Update u : all) {
    this->fetch_folder_files(u, c, true);
  }
}

void MainWindow::fetch_clicked()
{
  this->disable_fetch();
  this->updates.clear();
  std::vector<Update> all = gather_tracked();
  size_t c = all.size();
  for (Update u : all) {
    this->fetch_folder_files(u, c, false);
  }
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
  if (r->error() != QNetworkReply::NoError) {
    qDebug() << "Network Error: " << r->errorString();
    return;
  }
  auto j = to_json(r);
  this->set_auth_state(is_valid_profile(j.object()));
  this->fetch_courses();
}

void MainWindow::courses_fetched()
{
  QNetworkReply *r = (QNetworkReply *)this->sender();
  if (r->error() != QNetworkReply::NoError) {
    qDebug() << "Network Error: " << r->errorString();
    return;
  }
  auto j = to_json(r);
  this->user_courses = to_courses(j);
  for (auto c : this->user_courses)
    this->fetch_course_folders(c);
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
  ui->guideText->setHidden(!this->gather_tracked().empty());
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
    auto a = local_dir / f[j].filename;
    if (!std::filesystem::exists(local_dir / f[j].filename)) {
      f[j].local_dir = u.local_dir;
      u.files.push_back(f[j]);
    }
  }

  int ed = 0;

  if (download) {
    dl_e_mtx.lock();
    if (this->expected_downloads == 0 && u.files.size() > 0) {
      ui->progressBar->show();
    }
    this->expected_downloads += u.files.size();
    ed = this->expected_downloads;
    ui->progressBar->setMaximum(expected_downloads);
    qDebug() << "Total expected downloads is now" << expected_downloads;
    dl_e_mtx.unlock();
    for (auto f : u.files) {
      this->download_file(f, u.files.size());
    }
  }
  bool updates_done = false;

  update_mtx.lock();
  this->updates.push_back(std::move(u));
  updates_done = updates.size() == c;
  this->updates_done = updates_done;
  update_mtx.unlock();

  if (updates_done && !download) {
    this->enable_fetch();
    show_updates(this->updates);
  } else if (updates_done && ed == 0) {
    ui->progressBar->setHidden(true);
    this->enable_pull();
    show_updates(this->updates);
  }
  r->deleteLater();
}

void MainWindow::file_downloaded(File f, size_t c)
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
    show_updates(this->updates);
  }

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

    ui->guideText->hide();

    QDir selected_dir = QDir::fromNativeSeparators(local_dir);
    selected_dir.cdUp();
    this->start_dir = selected_dir.path();
  }
}

void MainWindow::treeView_cleared(const QModelIndex &index)
{
  settings.remove(get_id(index));
  settings.sync();
  ui->guideText->setHidden(!this->gather_tracked().empty());
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
  ui->treeView->resizeColumnToContents(0);
  expand_tracked(ui->treeView);
  ui->treeView->setColumnHidden(FOLDER_ID, true);
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

void MainWindow::show_updates(const std::vector<Update> &u)
{
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

void MainWindow::fetch_folder_files(Update u, size_t c, bool download)
{
  QNetworkReply *a =
      this->get("/api/v1/folders/" + QString::number(u.folder_id) +
                "/files?per_page=1180");
  connect(a, &QNetworkReply::finished, this, [=]() {
    folder_files_fetched(std::move(u), c, download);
    terminate(a);
  });
}

void MainWindow::download_file(File f, size_t c)
{
  if (!std::filesystem::exists(f.local_dir)) {
    std::filesystem::create_directories(f.local_dir);
  }
  QNetworkReply *a = this->get_full(QString::fromStdString(f.url));
  connect(a, &QNetworkReply::finished, this, [=]() {
    file_downloaded(std::move(f), c);
    terminate(a);
  });
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
