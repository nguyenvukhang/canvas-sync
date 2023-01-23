#include "mainwindow.h"
#include "tree_model.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <csrv.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      server("", "https://canvas.nus.edu.sg")
{
  ui->setupUi(this);
  qDebug("hello");
  // tree model stuff
  TreeModel *model = new TreeModel(QStringList() << "Col A"
                                                 << "Col B"
                                                 << "Col C");
  model->appendRow(QStringList() << "Elements"
                                 << "dank");
  model->appendRow(QStringList() << "Items");
  model->item(0)->appendChild(new TreeItem(QStringList() << "Items"));
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
  QString tree_dump = QString::fromStdString(this->server.dump_tree());
  qDebug() << "TREE -> " << tree_dump;
}

void MainWindow::on_lineEdit_accessToken_textChanged(const QString &arg1)
{
  this->token = arg1;
  string token = arg1.toStdString();
  this->server = this->server.with_token(&token);
  qDebug() << "Token is now -> " << arg1;
}
