#include "app.h"
#include <csui/modal.h>
#include <csync/csync.h>

const QString help_access_token =
    "<span style=\"color: #555555\">"
    "To get your Canvas Access Token, head over to your "
    "<a href=\"https://canvas.nus.edu.sg/profile/settings\">"
    "Canvas profile settings"
    "</a>. "
    "Generate a new token by clicking on \"New Access Token\", "
    "and leave the expiry date blank to have it last forever."
    "</span>";

void App::start()
{
  ui->setupUi(this);
  connect_ui();
  connect_tree();
  connect_canvas();
  setup_ui();
  check_auth(settings.get("access-token"));
}

void App::setup_ui()
{
  ui->message->hide();
  ui->change_token_button->hide();
  ui->clear_button->hide();
  ui->change_token_button->setEnabled(false);
  ui->download_progress->hide();
  ui->help_text->hide();
  ui->tree_view->setModel(newTreeModel());
  ui->fetch_button->set_states("Fetch", "Fetching...");
  ui->pull_button->set_states("Pull", "Pulling...");
}

void App::connect_ui()
{
  connect(ui->access_token_input, &QLineEdit::textChanged, this,
          &App::check_auth);
  connect(ui->pull_button, &QPushButton::clicked, this, &App::pull_clicked);
  connect(ui->fetch_button, &QPushButton::clicked, this, &App::fetch_clicked);
  connect(ui->clear_button, &QPushButton::clicked, this, &App::clear_clicked);
  connect(ui->change_token_button, &QPushButton::clicked, this,
          &App::change_token_clicked);
}

void App::connect_tree()
{
  using T = TreeView;
  T *t = ui->tree_view;

  connect(t, &T::expanded, ui->tree_view, &T::prettify);
  connect(t, &T::collapsed, ui->tree_view, &T::prettify);
  connect(t, &T::doubleClicked, this, &App::tree_view_track_folder);
  connect(t, &T::pressed, this, &App::tree_view_pressed);
}

void App::connect_canvas()
{
  connect(canvas, &ICanvas::authenticate_done, this, [=](bool authenticated) {
    this->set_auth_state(authenticated);
    if (authenticated) canvas->fetch_courses();
  });

  // this chained series manages these two production lines:
  // 1. fetch courses -> fetch folders -> load course/folder name cache
  // 2. fetch files -> download files -> collate and show updates

  connect(canvas, &ICanvas::fetch_courses_done, this,
          [=](std::vector<Course> c) {
            this->user_courses = std::move(c);
            for (auto c : this->user_courses)
              canvas->fetch_folders(c);
          });

  connect(canvas, &ICanvas::fetch_folders_done, this,
          [=](const Course &c, std::vector<Folder> f) {
            FileTree t(&c, f);
            tree_mtx.lock();
            for (auto f : f) {
              this->folder_names.insert(std::pair(f.id, f.full_name));
            }
            this->course_trees.push_back(t);
            TreeModel *model = newTreeModel();
            FileTree gt;
            gt.insert_course_trees(this->course_trees);
            model->item(0)->insert(gt, settings);
            ui->tree_view->setModel(model);
            ui->tree_view->prettify();
            tree_mtx.unlock();
          });

  connect(canvas, &ICanvas::fetch_files_done, this,
          [=](const Folder &_fo, std::vector<File> files) {
            Folder folder(std::move(_fo));
            remove_existing_files(&files, folder.local_dir);
            folder.files = std::move(files);

            if (this->action == Action::PULL) {
              ui->download_progress->setMaximum(
                  canvas->increment_total_downloads(folder.files.size()));
              if (ui->download_progress->maximum() > 0)
                ui->download_progress->show();

              std::filesystem::create_directories(folder.local_dir);

              for (auto file : folder.files)
                canvas->download(file, folder);
            }

            tracked_folders_mtx.lock();
            this->tracked_folders.push_back(std::move(folder));
            tracked_folders_mtx.unlock();
          });

  connect(canvas, &ICanvas::download_done, ui->download_progress,
          &QProgressBar::setValue);

  connect(canvas, &ICanvas::all_fetch_done, this, [=]() {
    if (action == FETCH || (action == PULL && !canvas->has_downloads()))
      show_updates();
  });

  connect(canvas, &ICanvas::all_download_done, this, &App::show_updates);
}

void App::prefetch()
{
  this->tracked_folders.clear();
  ui->download_progress->setMaximum(0);
  ui->download_progress->setValue(0);
}

void App::fetch(const std::vector<Folder> &f)
{
  canvas->reset_counts();
  canvas->set_total_fetches(f.size());
  for (auto f : f) {
    canvas->fetch_files(f);
  }
}

//////////////////////////////////////////////////////////////////////
/// SLOTS
//////////////////////////////////////////////////////////////////////

void App::pull_clicked()
{
  ui->tree_view->setEnabled(false);
  action = Action::PULL;
  gather_tracked();
}

void App::fetch_clicked()
{
  ui->tree_view->setEnabled(false);
  action = Action::FETCH;
  gather_tracked();
}

void App::change_token_clicked()
{
  // reset access token entry
  ui->access_token_input->setText("");
  ui->access_token_input->setReadOnly(false);
  ui->access_token_input->setDisabled(false);

  canvas->set_token("");
  set_auth_state(false);
  ui->access_token_input->setFocus();

  settings.remove("access-token");
  settings.sync();
}

void App::clear_clicked()
{
  TreeView *t = ui->tree_view;
  const QModelIndex &index = t->currentIndex();
  t->model()->itemFromIndex(index)->setData(TreeIndex::LOCAL_DIR, "");
  QString folder_id = TreeIndex(index).id();
  emit t->dataChanged(index, index);
  ui->clear_button->hide();

  settings.remove(folder_id, Settings::LOCAL_DIR);
  settings.sync();
}

/// TREEVIEW SLOTS ---

void App::tree_view_track_folder(const QModelIndex &index)
{
  if (index.column() != TreeIndex::LOCAL_DIR) return;
  if (!index.parent().isValid()) {
    QMessageBox::information(
        this, "Info",
        "Can't track a course. Choose a folder within the course.");
    return;
  };
  TreeIndex ti = index;

  QFileDialog dialog(this, "Target for " + ti.get_ancestry("/"), start_dir);
  dialog.setFileMode(QFileDialog::Directory);
  int result = dialog.exec();

  // exit early if no file was chosen
  if (result == QFileDialog::Rejected) return;

  TreeItem *item = ui->tree_view->model()->itemFromIndex(index);
  QDir selected_dir = dialog.selectedFiles()[0];
  item->track_folder(index, selected_dir.path(), settings);

  selected_dir.cdUp();
  this->start_dir = selected_dir.path();
}

void App::tree_view_pressed(const QModelIndex &index)
{
  TreeIndex ti = index;
  if (index.column() == TreeIndex::LOCAL_DIR && ti.has_local_dir()) {
    qDebug() << ti.local_dir();
    ui->message->hide();
    ui->clear_button->show();
    return;
  }
  ui->clear_button->hide();
}

//////////////////////////////////////////////////////////////////////
/// HELPER FUNCTIONS
//////////////////////////////////////////////////////////////////////

std::string App::folder_name(const int folder_id)
{
  return this->folder_names.at(folder_id);
}

std::string App::course_name(const int course_id)
{
  size_t n = this->user_courses.size();
  for (size_t i = 0; i < n; i++) {
    if (this->user_courses[i].id == course_id)
      return this->user_courses[i].name;
  }
  return "[course not found]";
}

void App::terminate(QNetworkReply *r)
{
  disconnect(r);
  r->deleteLater();
}

void App::set_auth_state(bool authenticated)
{
  if (authenticated) {
    ui->pull_button->enable();
    ui->fetch_button->enable();
    ui->help_text->hide();
    ui->auth_indicator->setStyleSheet("color: " + GREEN);
    // disable token entry
    ui->access_token_input->setReadOnly(true);
    ui->access_token_input->setDisabled(true);
    // show edit token button, in case the user wants to change it
    ui->change_token_button->show();
    ui->change_token_button->setEnabled(true);

    settings.set("access-token", canvas->token());
    settings.sync();
    return;
  }
  ui->fetch_button->disable();
  ui->pull_button->disable();
  ui->help_text->setText(help_access_token);
  ui->help_text->show();
  ui->auth_indicator->setStyleSheet("color: " + AMBER);
  ui->change_token_button->hide();
  ui->change_token_button->setEnabled(false);
}

void App::show_updates()
{
  ready_all();

  QString buffer = "", tmp = "";
  int prev_course = -1;
  auto push = [&](const QString &t) { tmp.push_back(t); };
  auto bpush = [&](const QString &t) { buffer.push_back(t); };
  auto offload = [&]() {
    // bpush("<h2 style=\"padding-bottom: -100px\">");
    bpush("<h2>");
    bpush(course_name(prev_course).c_str());
    bpush("</h2>");
    bpush(std::move(tmp));
    tmp.clear();
  };
  sort(tracked_folders.begin(), tracked_folders.end(),
       [=](Folder &a, Folder &b) {
         return course_name(a.course_id) < course_name(b.course_id);
       });
  for (auto f : this->tracked_folders) {
    if (f.course_id != prev_course) {
      if (!tmp.isEmpty()) offload();
      prev_course = f.course_id;
    }
    if (!f.files.empty()) {
      push("<h3 style=\"color: #525252\">");
      push(folder_name(f.id).c_str());
      push("</h3>");
      for (auto f : f.files) {
        push(f.filename.c_str());
      }
    }
  }
  if (!tmp.isEmpty()) offload();
  if (buffer.isEmpty()) {
    QMessageBox::information(this, "Update", "All up to date!");
  } else {
    Modal *m = new Modal();
    m->setWindowTitle(action == Action::PULL ? "Downloaded" : "Updates");
    m->setText(buffer);
    m->show();
  }
}

void App::check_auth(const QString &token)
{
  canvas->set_token(token);
  ui->tree_view->setModel(newTreeModel());
  course_trees.clear();
  folder_names.clear();
  canvas->authenticate();
}

void App::gather_tracked()
{
  prefetch();
  TreeModel *model = ui->tree_view->model();
  size_t n = model->childrenCount();

  for (size_t i = 0; i < n; i++) {
    if (model->item(i)->has_untargeted()) {
      ui->message->setText("Some folders have no destination");
      ui->clear_button->hide();
      ui->message->show();
      ready_all();
      QTimer::singleShot(5000, [=]() { ui->message->hide(); });
      return;
    }
  }

  std::vector<Folder> all;

  // actually gather stuff
  for (size_t i = 0; i < n; i++) {
    std::vector<Folder> tf = model->item(i)->resolve_folders();
    all.reserve(all.size() + std::distance(tf.begin(), tf.end()));
    all.insert(all.end(), tf.begin(), tf.end());
  }
  qDebug() << "Gather tracked folders:";
  for (auto f : all)
    qDebug() << "*" << folder_name(f.id).c_str() << "->" << f.local_dir.c_str();

  if (all.empty()) {
    QMessageBox::information(this, action == PULL ? "Pull" : "Fetch",
                             "No folders selected to track.");
    ready_all();
  }
  fetch(all);
}

void App::ready_all()
{
  ui->pull_button->set_ready();
  ui->fetch_button->set_ready();
  ui->tree_view->setEnabled(true);
  ui->download_progress->hide();
}

TreeModel *App::newTreeModel()
{
  return new TreeModel({"canvas folder", "local folder", ""}, &this->settings);
};
