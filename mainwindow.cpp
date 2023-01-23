#include "mainwindow.h"
#include "samplewidget.h"
#include "tree_model.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  // tree model stuff
  TreeModel *model = new TreeModel(QStringList() << "Col A"
                                                 << "Col B"
                                                 << "Col C");
  model->appendRow(QStringList() << "Elements"
                                 << "dank");
  model->appendRow(QStringList() << "Items");
  ui->treeView->setModel(model);
}

MainWindow::~MainWindow() { delete ui; }
