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

void insert(TreeItem *item, FileTree *tree)
{
  int child_count = tree->folders.size();
  if (child_count == 0)
    return;
  for (int i = 0; i < child_count; i++) {
    auto f = tree->folders[i];
    QString id = QString::fromStdString(to_string(f.id));
    QString name = QString::fromStdString(f.name);
    item->appendChild(new TreeItem(QStringList() << name << "" << id));
    insert(item->child(i), &f);
  }
}

void fix_tree(Ui::MainWindow *ui)
{
  auto tree_view = ui->treeView;
  tree_view->resizeColumnToContents(0);
  tree_view->setColumnHidden(2, true);
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
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_pushButton_pull_clicked()
{
  this->server.load_tree();
  auto tree = this->server.get_tree();
  TreeModel *model = newTreeModel();
  insert(model->item(0), tree);
  ui->treeView->setModel(model);
  ui->treeView->setColumnHidden(2, true);
}

void MainWindow::on_lineEdit_accessToken_textChanged(const QString &input)
{
  this->token = input;
  if (token.size() > 20) {
    string token = input.toStdString();
    this->server.set_token(&token);
    ui->label_authenticationStatus->setText("authenticating...");
    ui->label_authenticationStatus->repaint();
    qApp->processEvents();
  } else {
    ui->label_authenticationStatus->setText("unauthenticated");
    return;
  }
  QFuture<bool> fut =
      QtConcurrent::run([this, input] { return this->server.valid_token(); });
  if (fut.result()) {
    ui->label_authenticationStatus->setText("authenticated!");
    ui->lineEdit_accessToken->setReadOnly(true);
    ui->lineEdit_accessToken->setDisabled(true);
    this->server.load_tree();
    auto tree = this->server.get_tree();
    TreeModel *model = newTreeModel();
    insert(model->item(0), tree);
    ui->treeView->setModel(model);
    fix_tree(ui);
  } else {
    ui->label_authenticationStatus->setText("unauthenticated");
  }
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
  qDebug() << "[ DEBUG ] " << index.siblingAtColumn(2).data(2) << index.data(1)
           << index.data(2);
}
