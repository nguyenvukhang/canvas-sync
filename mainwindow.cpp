#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "samplewidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  SampleWidget* sw = new SampleWidget();
  ui->gridLayout->addWidget(sw, 0, 1);
}

MainWindow::~MainWindow() { delete ui; }
