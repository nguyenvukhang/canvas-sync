#include "mainwindow.h"
#include "filetree.h"
#include "tree.h"
#include "tree_model.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFuture>
#include <QMessageBox>
#include <QPromise>
#include <QtConcurrent>
#include <csrv.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      server("", "https://canvas.nus.edu.sg")
{
  ui->setupUi(this);
  this->token = settings.value("token").toString();
  this->try_auth(this->token);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_pushButton_pull_clicked()
{
  auto a = settings.allKeys();
  qDebug() << "STARTING PULL";
  // only folder ids are numbers, and only numbers are folder ids
  for (auto key : a) {
    bool ok = false;
    int id = key.toInt(&ok);
    if (!ok)
      continue;
    vector<File> files = this->server.folder_files(&id);
    for (auto f : files) {
      qDebug() << f.filename.c_str();
    }
  }
  qDebug() << "DONE WITH PULL";
}

void MainWindow::try_auth(const QString &token)
{
  this->token = token;
  if (token.size() > 20) {
    string token_s = token.toStdString();
    this->server.set_token(&token_s);
    ui->label_authenticationStatus->setText("authenticating...");
    ui->label_authenticationStatus->repaint();
    qApp->processEvents();
  } else {
    ui->label_authenticationStatus->setText("unauthenticated");
    return;
  }
  QFuture<bool> fut =
      QtConcurrent::run([this, token] { return this->server.valid_token(); });
  if (fut.result()) {
    ui->label_authenticationStatus->setText("authenticated!");
    settings.setValue("token", token);
    settings.sync();
    ui->lineEdit_accessToken->setReadOnly(true);
    ui->lineEdit_accessToken->setDisabled(true);
    this->server.load_tree();
    auto tree = this->server.get_tree();
    TreeModel *model = newTreeModel();
    insert(model->item(0), tree, &settings);
    ui->treeView->setModel(model);
    fix_tree(ui);
  } else {
    ui->label_authenticationStatus->setText("unauthenticated");
  }
}

void MainWindow::on_lineEdit_accessToken_textChanged(const QString &input)
{
  this->try_auth(input);
}

void to_dir_dialog(QFileDialog *dialog)
{
  dialog->setFileMode(QFileDialog::Directory);
  // because for some reason it crashes on macOS
  dialog->setOption(QFileDialog::DontUseNativeDialog);
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
  // don't do anything to root bois.
  if (!index.parent().isValid()) {
    return;
  }
  // go home for non-localdir bois
  if (index.column() != 1)
    return;

  QFileDialog dialog(this);
  to_dir_dialog(&dialog);
  auto home = QDir::homePath();
  dialog.setDirectory(this->start_dir != home ? this->start_dir : home);
  int result = dialog.exec();

  auto item = ui->treeView->model()->itemFromIndex(index);

  auto local_dir = dialog.selectedFiles()[0];

  // only set the value if a value was actually chosen.
  if (result == 1) {
    // clears children and parent tracked folders because this might cause
    // conflicts in downloads.

    // clear children maps
    on_all_children(item, [&](TreeItem *child) {
      child->setData(1, "");
      settings.remove(get_id(*child));
    });

    // clear parent maps
    on_all_parents(item, [&](TreeItem *parent) {
      parent->setData(1, "");
      settings.remove(get_id(*parent));
    });

    // update itself
    item->setData(1, local_dir);
    settings.setValue(get_id(index), local_dir);
    settings.sync();
  }

  // updated last selected dir
  QDir selected_dir = QDir::fromNativeSeparators(local_dir);
  selected_dir.cdUp();
  this->start_dir = selected_dir.path();
}

void MainWindow::on_treeView_expanded(const QModelIndex &index)
{
  fix_tree(ui);
}

void MainWindow::on_treeView_collapsed(const QModelIndex &index)
{
  fix_tree(ui);
}

void MainWindow::on_treeView_cleared(const QModelIndex &index)
{
  settings.remove(get_id(index));
  settings.sync();
}

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
  qDebug() << "[ DEBUG ]\n";
  qDebug() << "id: " << get_id(index);
  qDebug() << "dir: " << get_local_dir(index);
  auto model = ui->treeView->model();
  int count = index.model()->children().size();
  qDebug() << "children: " << get_id(model->index(0, 0, index));
  qDebug() << "count:    " << count;
}

void MainWindow::on_pushButton_clicked()
{
  settings.clear();
  settings.sync();
}
