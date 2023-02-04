#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      settings(
          QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
              "/canvas-sync-settings.ini",
          QSettings::IniFormat),
      canvas("https://canvas.nus.edu.sg")
{
  ui->setupUi(this);
  connect_buttons();
  connect_tree();
  connect_canvas();

  // text inputs
  connect(ui->lineEdit_accessToken, &QLineEdit::textChanged, this,
          &MainWindow::check_auth);

  // scripted views
  ui->pushButton_changeToken->hide();
  ui->progressBar->hide();
  ui->treeView->setModel(newTreeModel());
  ui->guideText->hide();
  ui->label_accessTokenHelp->hide();

  this->check_auth(settings.value("access-token").toString());
}

void MainWindow::connect_buttons()
{
  connect(ui->pushButton_pull, &QPushButton::clicked, this,
          &MainWindow::pull_clicked);
  connect(ui->pushButton_fetch, &QPushButton::clicked, this,
          &MainWindow::fetch_clicked);
  connect(ui->pushButton_changeToken, &QPushButton::clicked, this,
          &MainWindow::changeToken_clicked);
}

void MainWindow::connect_tree()
{
  connect(ui->treeView, &ClickableTreeView::expanded, ui->treeView,
          &ClickableTreeView::prettify);
  connect(ui->treeView, &ClickableTreeView::collapsed, ui->treeView,
          &ClickableTreeView::prettify);

  connect(ui->treeView, &ClickableTreeView::cleared, this,
          &MainWindow::treeView_cleared);
  connect(ui->treeView, &ClickableTreeView::track_folder, this,
          &MainWindow::treeView_trackFolder);
}

void MainWindow::connect_canvas()
{
  connect(&canvas, &Canvas::authenticate_done, this, [=](bool authenticated) {
    this->set_auth_state(authenticated);
    if (authenticated) canvas.fetch_courses();
  });

  // this chained series manages these two production lines:
  // 1. fetch courses -> fetch folders -> load course/folder name cache
  // 2. fetch files -> download files -> collate and show updates

  connect(&canvas, &Canvas::fetch_courses_done, this,
          [=](std::vector<Course> c) {
            this->user_courses = std::move(c);
            for (auto c : this->user_courses)
              canvas.fetch_folders(c);
          });

  connect(&canvas, &Canvas::fetch_folders_done, this,
          [=](const Course &c, std::vector<Folder> f) {
            FileTree t(&c, f);
            tree_mtx.lock();
            for (auto f : f) {
              this->folder_names.insert(std::pair(f.id, f.full_name));
            }
            this->course_trees.push_back(t);
            refresh_tree_data();
            ui->treeView->prettify();
            tree_mtx.unlock();
            ui->guideText->setHidden(!gather_tracked().empty());
          });

  connect(&canvas, &Canvas::fetch_files_done, this,
          [=](const Folder &_fo, std::vector<File> files) {
            Folder folder(std::move(_fo));
            remove_existing_files(&files, folder.local_dir);
            folder.files = std::move(files);

            if (this->action == Action::PULL) {
              ui->progressBar->setMaximum(
                  canvas.increment_total_downloads(folder.files.size()));
              if (ui->progressBar->maximum() > 0) ui->progressBar->show();

              std::filesystem::create_directories(folder.local_dir);

              for (auto file : folder.files)
                canvas.download(file, folder);
            }

            tracked_folders_mtx.lock();
            this->tracked_folders.push_back(std::move(folder));
            tracked_folders_mtx.unlock();
          });

  connect(&canvas, &Canvas::download_done, ui->progressBar,
          &QProgressBar::setValue);

  connect(&canvas, &Canvas::all_fetch_done, this, [=]() {
    if (action == FETCH || (action == PULL && !canvas.has_downloads()))
      show_updates();
  });

  connect(&canvas, &Canvas::all_download_done, this, &MainWindow::show_updates);
}

void MainWindow::prefetch()
{
  this->tracked_folders.clear();
  ui->progressBar->setMaximum(0);
  ui->progressBar->setValue(0);
}

void MainWindow::fetch(const std::vector<Folder> &f)
{
  canvas.reset_counts();
  canvas.set_total_fetches(f.size());
  for (auto f : f) {
    canvas.fetch_files(f);
  }
}

//////////////////////////////////////////////////////////////////////
/// SLOTS
//////////////////////////////////////////////////////////////////////

void MainWindow::pull_clicked()
{
  this->action = Action::PULL;
  this->prefetch();
  this->disable_pull();

  std::vector<Folder> tracked = gather_tracked();
  if (tracked.empty()) {
    QMessageBox::information(this, "Pull", "No folders selected to track.");
    this->enable_pull();
    return;
  }
  fetch(tracked);
}

void MainWindow::fetch_clicked()
{
  this->action = Action::FETCH;
  this->prefetch();
  this->disable_fetch();

  std::vector<Folder> tracked = gather_tracked();
  if (tracked.empty()) {
    QMessageBox::information(this, "Fetch", "No folders selected to track.");
    this->enable_fetch();
    return;
  }
  fetch(tracked);
}

void MainWindow::changeToken_clicked()
{
  // reset access token entry
  ui->lineEdit_accessToken->setText("");
  ui->lineEdit_accessToken->setReadOnly(false);
  ui->lineEdit_accessToken->setDisabled(false);

  canvas.set_token("");
  settings.setValue("access-token", "");
  this->set_auth_state(false);
}

/// TREEVIEW SLOTS ---

void MainWindow::treeView_cleared(const QModelIndex &index)
{
  ui->treeView->model()->itemFromIndex(index)->setData(TreeItem::LOCAL_DIR, "");
  ui->guideText->setHidden(!this->gather_tracked().empty());
  QString folder_id = get_id(index);
  if (folder_id.isEmpty()) return;
  settings.remove(folder_id);
  settings.sync();
}

void MainWindow::treeView_trackFolder(const QModelIndex &index)
{
  if (!index.parent().isValid()) return;

  QFileDialog dialog(this, "Target for " + get_ancestry(index, " / "),
                     this->start_dir);
  int result = dialog.exec();
  grabKeyboard();

  TreeItem *item = ui->treeView->model()->itemFromIndex(index);

  QString local_dir = dialog.selectedFiles()[0];

  // exit early if no file was chosen
  if (result != 1) return;

  // clears children and parent tracked folders because this might cause
  // conflicts in downloads.

  // clear children maps
  item->on_all_children([&](TreeItem &child) {
    child.setData(TreeItem::LOCAL_DIR, "");
    QString folder_id = child.get_id();
    if (!folder_id.isEmpty()) settings.remove(folder_id);
  });

  // clear parent maps
  item->on_all_parents([&](TreeItem &parent) {
    parent.setData(TreeItem::LOCAL_DIR, "");
    QString folder_id = parent.get_id();
    if (!folder_id.isEmpty()) settings.remove(folder_id);
  });

  // update itself
  item->setData(TreeItem::LOCAL_DIR, local_dir);
  settings.setValue(get_id(index), local_dir);
  settings.sync();

  ui->guideText->hide();

  QDir selected_dir = QDir::fromNativeSeparators(local_dir);
  selected_dir.cdUp();
  this->start_dir = selected_dir.path();
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
  size_t n = this->user_courses.size();
  for (size_t i = 0; i < n; i++) {
    if (this->user_courses[i].id == course_id)
      return this->user_courses[i].name;
  }
  return "[course not found]";
}

void MainWindow::terminate(QNetworkReply *r)
{
  disconnect(r);
  r->deleteLater();
}

void MainWindow::enable_pull(const QString &s)
{
  ui->pushButton_pull->setText(s);
  ui->pushButton_pull->setEnabled(true);
}

void MainWindow::disable_pull(const QString &s)
{
  ui->pushButton_pull->setText(s);
  ui->pushButton_pull->setEnabled(false);
}

void MainWindow::enable_fetch(const QString &s)
{
  ui->pushButton_fetch->setText(s);
  ui->pushButton_fetch->setEnabled(true);
}

void MainWindow::disable_fetch(const QString &s)
{
  ui->pushButton_fetch->setText(s);
  ui->pushButton_fetch->setEnabled(false);
}

void MainWindow::refresh_tree_data()
{
  TreeModel *model = newTreeModel();
  FileTree t;
  t.insert_course_trees(this->course_trees);
  model->item(0)->insert(t, settings);
  ui->treeView->setModel(model);
}

void MainWindow::set_auth_state(bool authenticated)
{
  if (authenticated) {
    this->enable_pull();
    this->enable_fetch();
    ui->label_accessTokenHelp->hide();
    ui->label_authenticationStatus->setText("authenticated!");
    // disable token entry
    ui->lineEdit_accessToken->setReadOnly(true);
    ui->lineEdit_accessToken->setDisabled(true);
    // show edit token button, in case the user wants to change it
    this->ui->pushButton_changeToken->show();
    this->settings.setValue("access-token", canvas.token());
    settings.sync();
    return;
  }
  this->disable_fetch("Fetch");
  this->disable_pull("Pull");
  ui->label_accessTokenHelp->show();
  ui->label_authenticationStatus->setText("unauthenticated");
  this->ui->pushButton_changeToken->hide();
}

void MainWindow::show_updates()
{
  // update state before showing update window
  this->enable_fetch();
  this->enable_pull();
  ui->progressBar->hide();

  QString buffer = "", tmp = "";
  int prev_course = -1;
  sort(tracked_folders.begin(), tracked_folders.end(),
       [=](Folder &a, Folder &b) {
         return course_name(a.course_id) < course_name(b.course_id);
       });
  for (auto f : this->tracked_folders) {
    if (f.course_id != prev_course) {
      if (!tmp.isEmpty()) {
        buffer.push_back("## ");
        buffer.push_back(course_name(prev_course).c_str());
        buffer.push_back('\n');
        buffer.push_back(tmp);
        tmp.clear();
      }
      prev_course = f.course_id;
    }
    if (!f.files.empty()) {
      tmp.push_back("#### ");
      tmp.push_back(this->folder_name(f.id).c_str());
      tmp.push_back('\n');
    }
    for (auto f : f.files) {
      tmp.push_back(f.filename.c_str());
      tmp.push_back('\n');
      tmp.push_back('\n');
    }
  }
  if (!tmp.isEmpty()) {
    buffer.push_back("## ");
    buffer.push_back(course_name(prev_course).c_str());
    buffer.push_back('\n');
    buffer.push_back(tmp);
    tmp.clear();
  }
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
  canvas.set_token(token);
  ui->treeView->setModel(newTreeModel());
  course_trees.clear();
  folder_names.clear();
  canvas.authenticate();
}

std::vector<Folder> MainWindow::gather_tracked()
{
  TreeModel *model = ui->treeView->model();
  size_t n = model->childrenCount();
  std::vector<Folder> all;
  for (size_t i = 0; i < n; i++) {
    std::vector<Folder> tf = model->item(i)->resolve_folders();
    all.reserve(all.size() + std::distance(tf.begin(), tf.end()));
    all.insert(all.end(), tf.begin(), tf.end());
  }
  return all;
}

TreeModel *MainWindow::newTreeModel()
{
  return new TreeModel({"canvas folder", "local folder"});
};
