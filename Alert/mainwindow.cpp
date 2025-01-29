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

const std::vector<QString> wwa_calls = {
    "3B8WWA",
    "3Z6I",
    "3Z7WWA",
    "4M5A",
    "4U1A",
    "5K4WWA",
    "6K0FM",
    "8E3R",
    "9M2D",
    "A43WWA",
    "A71AE",
    "A91WWA",
    "BA3RA",
    "BA7CK",
    "BG9MM",
    "BY0AB",
    "BY1RX",
    "BY2AA",
    "BY4DX",
    "BY5HB",
    "BY6SX",
    "BY8GA",
    "CQ3WWA",
    "CQ7WWA",
    "CQ8WWA",
    "CR5WWA",
    "CR6WWA",
    "DA0WWA",
    "E7W",
    "EG1WWA",
    "EG2WWA",
    "EG3WWA",
    "EG4WWA",
    "EG5WWA",
    "EG6WWA",
    "EG7WWA",
    "EG8WWA",
    "EG9WWA",
    "EN0U",
    "GB1WWA",
    "GB2WWA",
    "GB4WWA",
    "GB6WWA",
    "GB8WWA",
    "HB9WWA",
    "HZ1WWA",
    "II0WWA",
    "II1WWA",
    "II2WWA",
    "II3WWA",
    "II4WWA",
    "II5WWA",
    "II6WWA",
    "II7WWA",
    "II8WWA",
    "II9WWA",
    "IR0WWA",
    "IR1WWA",
    "N0V",
    "N1W",
    "N5W",
    "N9W",
    "OL5WWA",
    "P49X",
    "PA25WWA",
    "RW1F",
    "S53WWA",
    "SN3WWA",
    "SN4WWA",
    "SN6WWA",
    "SN7WWA",
    "SX0W",
    "T77LA",
    "TI1I",
    "TK4TH",
    "UP7WWA",
    "W4I",
    "YI1RN",
    "YU5EA",
    "ZW5B"};

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
    params = "select * from dxcc where Dxcc = %1";
    params = params.arg(entity);
    //qDebug() << params;
    query.exec(params);

    while (query.next())
    {
        qDebug() << call << query.value(4).toString() << query.value(5).toString() << query.value(6).toString() << query.value(7).toString() << query.value(8).toString();
    }

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

