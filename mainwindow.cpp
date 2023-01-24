#include "mainwindow.h"
#include "filetree.h"
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

void insert(TreeItem *item, FileTree *tree, QSettings *settings)
{
  int child_count = tree->folders.size();
  if (child_count == 0)
    return;
  for (int i = 0; i < child_count; i++) {
    auto f = tree->folders[i];
    QString id = QString::fromStdString(to_string(f.id));
    QString name = QString::fromStdString(f.name);
    QString dir = settings->value(id).toString();
    item->appendChild(new TreeItem(QStringList() << name << dir << id));
    insert(item->child(i), &f, settings);
  }
}

QString get_id(const QModelIndex &index)
{
  return index.siblingAtColumn(2).data(2).toString();
}

QString get_local_dir(const QModelIndex &index)
{
  return index.siblingAtColumn(1).data(2).toString();
}

bool expand_tracked(ClickableTreeView *tree, QModelIndex &index)
{
  TreeModel *model = tree->model();
  bool expand = false;
  int i = 0;
  QModelIndex child = model->index(i++, 0, index);
  while (child.isValid()) {
    expand |= expand_tracked(tree, child);
    child = model->index(i++, 0, index);
  }
  if ((i == 1 && !get_local_dir(index).isEmpty()) || expand) {
    tree->expand(index.parent());
    return true;
  }
  return expand;
}

void expand_tracked(ClickableTreeView *tree)
{
  TreeModel *model = tree->model();
  int i = 0;
  QModelIndex child = model->index(i++, 0);
  while (child.isValid()) {
    if (expand_tracked(tree, child)) {
      tree->expand(child);
    }
    child = model->index(i++, 0);
  }
}

void fix_tree(Ui::MainWindow *ui)
{
  auto tree_view = ui->treeView;
  tree_view->resizeColumnToContents(0);
  // FIXME: after debugging, hide ids from user
  // tree_view->setColumnHidden(2, true);
  expand_tracked(tree_view);
}

void clear_children(TreeItem *item, int index)
{
  item->setData(index, "");
  int child_count = item->childrenItems().size();
  for (int i = 0; i < child_count; i++) {
    clear_children(item->child(i), index);
  }
}

TreeModel *newTreeModel()
{
  auto headers = QStringList() << "canvas folder"
                               << "local folder"
                               << "id";
  TreeModel *model = new TreeModel(headers);
  return model;
}

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
    clear_children(item, 1);
    // update itself
    item->setData(1, local_dir);
    // update settings
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

void MainWindow::on_treeView_cleared(const QModelIndex &index)
{
  settings.remove(get_id(index));
  settings.sync();
}
