#include "mainwindow.h"
#include "samplewidget.h"
#include "tree_model.h"
#include "ui_mainwindow.h"
#include <csrv.h>

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
  model->item(0)->appendChild(new TreeItem(QStringList() << "Items"));
  // model->item(0)->childrenItems;
  // model[0].appendChild();
  ui->treeView->setModel(model);
  ClickableTreeView *ctv = ui->treeView;
  ctv->expandAll();
}

MainWindow::~MainWindow() { delete ui; }
