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

    // QJsonValue value = object.value("dxcc");
    // array = value.toArray();
}

void MainWindow::MessageReceived(const QString& message)
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

     QString country2 = "Digi";
    QString country = FindCountry2(call);
    int entity = FindEntity(call);;

    //qDebug() << country << country2;

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

    params = "insert into qso (Call, Entity, Country, Mode, Band, Message) values('%1', '%2', '%3', '%4', '80M', '%5')";
    params = params.arg(call).arg(entity).arg(country).arg(mode).arg(message);
    //qDebug() << params;
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

QString MainWindow::FindCountry2(QString& call)
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
    foreach (const QJsonValue & value, array)
    {
        QRegularExpression rx(value.toObject().value("prefixRegex").toString());
        QRegularExpressionMatch match = rx.match(call);
        if (match.hasMatch())
        {
            return value.toObject().value("entityCode").toInt();
        }
    }
    return -1;
}
