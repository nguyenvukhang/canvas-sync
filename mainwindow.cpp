#include "mainwindow.h"
#include "filetree.h"
#include "tree_model.h"
#include "ui_mainwindow.h"
#include <QDebug>
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
    item->appendChild(new TreeItem(QStringList() << name << id));
    insert(item->child(i), &f);
  }
}

TreeModel *newTreeModel()
{
  auto headers = QStringList() << "id"
                               << "name"
                               << "local dir";
  TreeModel *model = new TreeModel(headers);
  return model;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      server("", "https://canvas.nus.edu.sg")
{
  ui->setupUi(this);
  auto headers = QStringList() << "id"
                               << "name"
                               << "local dir";
  TreeModel *model = new TreeModel(headers);
  ui->treeView->setModel(model);
  ClickableTreeView *ctv = ui->treeView;
  ctv->expandAll();
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
}

void MainWindow::on_lineEdit_accessToken_textChanged(const QString &input)
{
  this->token = input;
  string token = input.toStdString();
  this->server.set_token(&token);
}
