#include "mainwindow.h"
#include "convert.h"
#include "filetree.h"
#include "tree.h"
#include "tree_model.h"
#include "ui_mainwindow.h"
#include "updates.h"
#include <algorithm>
#include <csrv.h>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFuture>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QPromise>
#include <QtConcurrent>

/**
 * This class will support a https GET request
 */
class RealClient : public HttpClient
{
private:
  QNetworkAccessManager *nm;
  QNetworkRequest req;
  const std::string base_url;
  std::string token;

public:
  RealClient(QNetworkAccessManager *nm, const std::string base_url,
             const std::string token)
      : base_url(base_url)
  {
    this->token = token;
    this->nm = nm;
  };
  std::string get(const std::string &url) override
  {

    std::string full_url = this->base_url + url;
    qDebug() << "[->]" << full_url.c_str();
    req.setUrl(QUrl(full_url.c_str()));
    std::string htoken = "Bearer " + this->token;
    req.setRawHeader("Authorization", QByteArray::fromStdString(htoken));
    QNetworkReply *reply = nm->get(this->req);
    qDebug() << "REPLY" << reply->readAll();
    std::string body = reply->readAll().toStdString();
    qDebug() << "REPLY:parsed" << body.c_str();
    return body;
  };
};

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

  this->token = settings.value("token").toString().toStdString();
  ui->pushButton_changeToken->setHidden(true);
  ui->treeView->setModel(newTreeModel());
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
}

void MainWindow::fetch_clicked()
{
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

void MainWindow::check_auth_fetched(QNetworkReply *r)
{
  disconnect(&this->nw, SIGNAL(finished(QNetworkReply *)), this,
             SLOT(check_auth_fetched(QNetworkReply *)));
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

void MainWindow::courses_fetched(QNetworkReply *r)
{
  disconnect(&this->nw, SIGNAL(finished(QNetworkReply *)), this,
             SLOT(courses_fetched(QNetworkReply *)));
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
  auto j = to_json(r);
  std::vector<Folder> f = to_folders(j);
  qDebug() << c.name.c_str() << "has" << f.size() << "folders";
  FileTree t(&c, f);
  tree_mtx.lock();
  this->course_trees.push_back(t);
  this->refresh_tree();
  tree_mtx.unlock();
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
  // ui->treeView->setColumnHidden(FOLDER_ID, true);
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
    return;
  }
  ui->label_authenticationStatus->setText("unauthenticated");
  this->ui->pushButton_changeToken->setHidden(true);
}

void MainWindow::show_updates(const std::vector<Update> &u)
{
}

void MainWindow::check_auth(const QString &token)
{
  this->token = token.toStdString();
  QNetworkRequest r = req("/api/v1/users/self/profile");
  connect(&this->nw, SIGNAL(finished(QNetworkReply *)), this,
          SLOT(check_auth_fetched(QNetworkReply *)));
  this->nw.get(r);
}

void MainWindow::fetch_courses()
{
  QNetworkRequest r = req("/api/v1/courses");
  connect(&this->nw, SIGNAL(finished(QNetworkReply *)), this,
          SLOT(courses_fetched(QNetworkReply *)));
  this->nw.get(r);
}

void MainWindow::fetch_course_folders(const Course &c)
{
  std::string url = "/api/v1/courses/" + std::to_string(c.id) + "/folders";
  qDebug() << "[" << url.c_str() << "]";
  QNetworkRequest r = req(url);
  QNetworkReply *a = this->nw.get(r);
  connect(a, &QNetworkReply::finished, this,
          [=]() { this->course_folders_fetched(c); });
}
