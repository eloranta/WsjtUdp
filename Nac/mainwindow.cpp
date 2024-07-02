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

    QString call;
    QString locator;

    if (list[0] == "CQ" && list.length() > 2)
    {
        call = list[2];
        if (list.length() > 3)
            locator = list[3];
    }
    else if (list.length() == 3)
    {
        call = list[1];
        locator = list[2];
    }
    else if (list.length() == 4)
    {
        call = list[1];
        locator = list[3];
        locator.chop(2);
    }
    else if (list.length() == 5)
    {
        call = list[1];
        locator = list[4];
        locator.chop(2);
    }
    if (call[0] == '<' && call[call.length()-1] == '>')
    {
        call = call.remove(0, 1);
        call = call.remove(-1, 1);
    }
    if (locator == "73" || locator == "RR73" || locator == "RRR")
        locator = "";

    QStandardItem *item = new QStandardItem(call);
    model.setItem(y, 0, item);

    item = new QStandardItem(locator);
    model.setItem(y, 1, item);

    item = new QStandardItem(message);
    model.setItem(y, 2, item);

    y++;
    ui->tableView->scrollToBottom();
}


