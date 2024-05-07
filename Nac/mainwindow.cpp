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
    if (list.length() <= 1)
        return;

    QStandardItem *item;
    QString call;
    if (list[0] == "CQ" && list.length() > 2)
    {
        call = list[2];
    }
    else if (list.length() > 2)
    {
        call = list[1];
    }
    if (call[0] == '<' && call[call.length()-1] == '>')
    {
        call = call.remove(0, 1);
        call = call.remove(-1, 1);
    }
    item = new QStandardItem(call);
    model.setItem(y, 0, item);

    item = new QStandardItem(message);
    model.setItem(y, 1, item);

    y++;
    ui->tableView->scrollToBottom();
}


