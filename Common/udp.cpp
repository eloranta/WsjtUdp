#include "udp.h"
#include <QDataStream>
#include <QTime>
#include <iostream>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include "QRegularExpression"

udp::udp(QObject *parent) : QObject(parent)
{
    ReadDxccJson();
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::LocalHost, 2237);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void udp::ReadDxccJson()
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

void udp::readyRead()
{
    QByteArray buffer;
    buffer.resize(static_cast<int>(socket->pendingDatagramSize()));

    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    //qDebug() << "Message from: " << sender.toString();
    //qDebug() << "Message port: " << senderPort;
    //qDebug() << "Message: " << buffer;

    ParseMessage(buffer);
}

void udp::ParseMessage(QByteArray& buffer)
{
    //qDebug() << "Message: " << buffer;

    QDataStream stream(&buffer, QIODevice::ReadOnly);
    quint32 magic;
    stream >> magic;
    if (magic != 0xadbccbda)
    {
        qDebug() << "Magic number not correct: " << magic;
        return;
    }

    quint32 schema;
    stream >> schema;
    if (schema != 2)
    {
        qDebug() << "Schema number not 2: " << schema;
        return;
    }
    quint32 messageNumber;
    stream >> messageNumber;

    //qDebug() << messageNumber;

    switch(messageNumber)
    {
    case Heartbeat:
        //qDebug() << "Heartbeat";
        break;
    case Status:
        //qDebug() << "Status";
        break;
    case Decode:
        //qDebug() << "Decode";
        decode(stream);
        break;
    case Clear:
        qDebug() << "Clear";
        break;
    case QSOLogged:
        qDebug() << "QSOLogged";
        break;
    case Close:
        qDebug() << "Close";
        break;
    case WSPRDecode:
        qDebug() << "WSPRDecode";
        break;
    case LoggedADIF:
        qDebug() << "LoggedADIF";
        break;
    default:
        qDebug() << "Unknown message number received: " << messageNumber;
        break;
    }
}

void udp::decode(QDataStream &stream)
{
    uint len;
    char *raw;
    stream.readBytes(raw, len);
    QString id = QString::fromUtf8(raw, static_cast<int>(len));
    bool New = false;
    QTime Time;
    qint32 snr;
    double deltaTime;
    quint32 deltaFrequency;
    stream >> New >> Time >> snr >> deltaTime >> deltaFrequency;
    //qDebug() << "Id =" << id << "New =" << New << "Time =" << Time << "SNR =" << snr
    //         << "DT =" << deltaTime << "DF =" << deltaFrequency;

    stream.readBytes(raw, len);
    QString mode = QString::fromUtf8(raw, static_cast<int>(len));
    //qDebug() << "Mode:" << mode;

    stream.readBytes(raw, len);
    QString message = QString::fromUtf8(raw, static_cast<int>(len));
    //qDebug() << "Message:" << message;

    QStringList list = message.split(" ");

    QString caller = list[1];
    if (caller.isEmpty())
        return;
    if (caller.length() < 3)
        return;
    if (caller == "RR73"|| caller == "RR73;" || caller == "...")
        return;
    if (caller == "USA" || caller == "VOTA")
        caller = list[2];

    if (!caller.isEmpty() && caller.front() == '<' && caller.back() == '>')
    {
        caller = caller.remove(0, 1);
        if (!caller.isEmpty()) caller.chop(1);
    }

    QString country = FindCountry(caller);

    std::cout << caller.toStdString() << "\t\t" << country.toStdString() << std::endl;
}

QString udp::FindCountry(QString& call)
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





