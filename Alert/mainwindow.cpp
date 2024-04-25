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

    ReadDxccJson2();

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

    connect(&udp, SIGNAL(MessageReceived(QString)), this, SLOT(MessageReceived(QString)));
    connect(&udp, SIGNAL(FreqChange(int)), this, SLOT(FreqChange(int)));

    ui->tableView->hideColumn(0);

    band = "7";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ReadDxccJson2()
{
    QString jsonFile = qApp->applicationDirPath() + "/prefix.json";
    QFile file;
    file.setFileName(jsonFile);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString text = file.readAll();
    file.close();
    //qDebug() << text;

    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8());
    if (doc.isNull())
    {
        qDebug() << "Error in parsing prefix.json";
        return;
    }

    object = doc.object();
    //qDebug() << doc;
}

void MainWindow::MessageReceived(QString message)
{
    //qDebug() << message;
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
     if (call == "RR73;")
         return;

    QString country = FindCountry(call);
    int entity = FindEntity(call);

    QSqlQuery query;
    QString params;
    // params = "select * from dxcc where Dxcc = %1";
    // params = params.arg(entity);
    // qDebug() << params;
    // qDebug() << query.exec(params);

    // while (query.next())
    // {
    //     qDebug() << query.value(14);
    // }

    const QString mode = "Digi";

    params = "insert into qso (Call, Entity, Country, Mode, Band, Message) values('%1', '%2', '%3', '%4', '%5', '%6')";
    params = params.arg(call).arg(entity).arg(country).arg(mode).arg(band).arg(message);
    //qDebug() << params;
    query.exec(params);
    model.select();

    ui->tableView->scrollToBottom();
}

QString MainWindow::FindCountry(QString& call)
{
    QJsonValue item = object.value(call[0]);
    QJsonArray array = item.toArray();
    static QRegularExpression rx;
    static QRegularExpressionMatch match;
    foreach (const QJsonValue & value, array)
    {
        QString e = value.toObject().value("re").toString();
        rx.setPattern(e);
        match = rx.match(call);
        if (match.hasMatch())
        {
            return value.toObject().value("country").toString();
        }
    }
    return "not found";
}

int MainWindow::FindEntity(QString& call)
{
    QJsonValue item = object.value(call[0]);
    QJsonArray array = item.toArray();
    static QRegularExpression rx;
    static QRegularExpressionMatch match;
    foreach (const QJsonValue & value, array)
    {
        QString e = value.toObject().value("re").toString();
        rx.setPattern(e);
        match = rx.match(call);
        if (match.hasMatch())
        {
            return value.toObject().value("entity").toInt();
        }
    }
    return -1;
}

void MainWindow::FreqChange(int freq)
{
    band = QString::number(freq/1000000);
}

