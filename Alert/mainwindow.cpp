#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableView->setModel(&model);
    y = 0;
    connect(&udp, SIGNAL(MessageReceived(const QString&)), this, SLOT(MessageReceived(const QString&)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::MessageReceived(const QString& message)
{
    qDebug() << message;
    QStandardItem *item1 = new QStandardItem(message);
    model.setItem(y++, 0, item1);

}


