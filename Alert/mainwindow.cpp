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
    if (message.isEmpty())
        return;
    const QStringList list = message.split(" ");
    if (list.length() > 0)
    {
        QStandardItem *item = new QStandardItem(list[0]);
        model.setItem(y, 0, item);
    }
    if (list.length() > 1)
    {
        QStandardItem *item = new QStandardItem(list[1]);
        model.setItem(y, 1, item);
    }
    if (list.length() > 2)
    {
        QStandardItem *item = new QStandardItem(list[2]);
        model.setItem(y, 2, item);
    }
    y++;
}


