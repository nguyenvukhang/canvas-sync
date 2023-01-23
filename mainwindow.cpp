#include "mainwindow.h"
#include "filetree.h"
#include "tree_model.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <csrv.h>

TreeItem *to_node(FileTree *content)
{
  QString id = QString::fromStdString(to_string(content->id));
  QString name = QString::fromStdString(content->name);
  TreeItem *t = new TreeItem(QStringList() << id << name);
  return t;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      server("", "https://canvas.nus.edu.sg")
{
  ui->setupUi(this);
  TreeModel *model = new TreeModel(QStringList() << "id"
                                                 << "name"
                                                 << "local dir");
  model->appendRow(QStringList() << "12345"
                                 << "Clownery"
                                 << "/dev/null");

  model->item(0)->appendChild(new TreeItem(QStringList() << "09876"
                                                         << "Nested"));
  FileTree f;
  model->item(0)->appendChild(to_node(&f));

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
  qDebug() << "CLICKED -> PULL" << this->token;
  this->server.load_tree();
  auto tree = this->server.get_tree();
  auto item = to_node(tree);
  auto model = ui->treeView->model();
  // model->item(0)->appendChild(&item);
  // ui->treeView->setModel(model);
  QString tree_dump = QString::fromStdString(this->server.dump_tree());
  qDebug() << "TREE -> " << tree_dump;
}

void MainWindow::on_lineEdit_accessToken_textChanged(const QString &arg1)
{
  this->token = arg1;
  string token = arg1.toStdString();
  this->server.set_token(&token);
  qDebug() << "Token is now -> " << arg1;
}
