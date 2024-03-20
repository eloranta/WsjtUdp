#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model.initialize();
    ui->tableView->setModel(&model);

    ReadDxccJson();

    QSqlQuery query;
    query.exec("drop table qso");


    query.exec(QString("create table if not exists qso ("
               "Id integer primary key autoincrement,"
               "Call text,"
               "Entity integer,"
               "Country text,"
               "Mode text,"
               "Band text,"
               "Message text)"));

    connect(&udp, SIGNAL(MessageReceived(const QString&)), this, SLOT(MessageReceived(const QString&)));

    ui->tableView->hideColumn(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ReadDxccJson()
{
    QString jsonFile = qApp->applicationDirPath() + "/dxcc.json";
    QFile file;
    file.setFileName(jsonFile);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString text = file.readAll();
    file.close();
    //qDebug() << text;

    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    if (doc.isNull())
    {
        qDebug() << "Error in parsing worked.json";
        return;
    }
    //qDebug() << doc;

    QJsonObject object = doc.object();
    QJsonValue value = object.value("dxcc");
    array = value.toArray();
}

void MainWindow::MessageReceived(const QString& message)
{
    qDebug() << message;
    if (message.isEmpty())
        return;

    const QStringList list = message.split(" ");
    if (list.length() == 1)
        return;

    QString call = list[1];
    if (list.length() >= 2 && list[1].length() == 2)
        call = list[2];

     if (call.front() == '<' && call.back() == '>')
     {
         call = call.remove(0, 1);
         if (!call.isEmpty()) call.chop(1);
     }

    QString country = FindCountry(call);
    int entity = 666;

    QString params;
    params = "insert into qso (Call, Entity, Country, Mode, Band, Message) values('%1', '%2', '%3', 'DATA', '80M', '%4')";
    params = params.arg(call).arg(entity).arg(country).arg(message);
    qDebug() << params;

    QSqlQuery query;
    query.exec(params);
    model.select();

    ui->tableView->scrollToBottom();
}

QString MainWindow::FindCountry(QString& call)
{
    foreach (const QJsonValue & value, array)
    {
        QRegularExpression rx(value.toObject().value("prefixRegex").toString());
        QRegularExpressionMatch match = rx.match(call);
        if (match.hasMatch())
        {
            //dxcc = QString::number(value.toObject().value("entityCode").toInt());
            return value.toObject().value("name").toString();
            //continent = value.toObject().value("continent").toArray()[0].toString();
        }
    }
    return "not found";
}
