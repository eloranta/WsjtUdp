#include "udp.h"
#include <QDataStream>
#include <QTime>

Udp::Udp(QObject *parent) : QObject(parent)
{
    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::LocalHost, 2237);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

void Udp::readyRead()
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

void Udp::ParseMessage(QByteArray& buffer)
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
        //Heartbeat1(stream);
        break;
    case Status:
        //qDebug() << "Status";
        status(stream);
        break;
    case Decode:
        //qDebug() << "Decode";
        //decode(stream);
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

void Udp::Heartbeat1(QDataStream &stream)
{
    uint len;
    char *raw;
    stream.readBytes(raw, len);
    QString Id = QString::fromUtf8(raw, int(len));
    quint32 Maximum_schema_number;
    stream >> Maximum_schema_number;
    stream.readBytes(raw, len);
    QString Version = QString::fromUtf8(raw, int(len));
    stream.readBytes(raw, len);
    QString Revision = QString::fromUtf8(raw, int(len));
    qDebug() << "Heartbeat: " << "Id =" << Id << "Maximum_schema_number =" << Maximum_schema_number << "Version =" << Version << "Revision =" << Revision;
}

void Udp::status(QDataStream &stream)
{
    uint len;
    char *raw;
    stream.readBytes(raw, len);
    QString Id = QString::fromUtf8(raw, len);
    quint64 Dial_frequency;
    stream >> Dial_frequency;
    stream.readBytes(raw, len);
    QString Mode = QString::fromUtf8(raw, len);
    stream.readBytes(raw, len);
    QString Dx_call = QString::fromUtf8(raw, len);
    stream.readBytes(raw, len);
    QString Report = QString::fromUtf8(raw, len);
    stream.readBytes(raw, len);
    QString Tx_mode = QString::fromUtf8(raw, len);
    bool Tx_Enabled;
    bool Transmitting;
    bool Decoding;
    stream >> Tx_Enabled >> Transmitting >> Decoding;

    qDebug() << "Status: " << "Id =" << Id << "freq =" << Dial_frequency << "mode =" << Mode
             << "dx call =" << Dx_call << "report =" << Report << "tx mode =" << Tx_mode << "tx enabled =" << Tx_Enabled
             << "transmitting =" << Transmitting << "decoding =" << Decoding;
    quint32 Tx_df;
    quint32 Rx_df;
    stream >> Tx_df >> Rx_df;
    stream.readBytes(raw, len);
    QString Call = QString::fromUtf8(raw, len);
    stream.readBytes(raw, len);
    QString Grid = QString::fromUtf8(raw, len);
    stream.readBytes(raw, len);
    QString DxGrid = QString::fromUtf8(raw, len);

    qDebug() << "        " << "txdf =" << Tx_df << "rxdf =" << Rx_df << "mycall =" << Call << "mygrid =" << Grid << "dxgrid =" << DxGrid;

    bool TxWatchdog;
    stream >> TxWatchdog;

    QString SubMode;
    quint32 count;
    stream >> count;
    if (count < 0xffffffff)
    {
        raw = new char[count];
        stream.readRawData(raw, count);
        SubMode = QString::fromUtf8(raw, count);
        delete [] raw;
    }

    bool FastMode;
    stream >> FastMode;

    qDebug() << "        " << "TxWatchdog =" << TxWatchdog <<  "SubMode =" << SubMode << "FastMode =" << FastMode;

    quint8 SpecialOperationMode;
    quint32 FrequencyTolerance;
    quint32 TRPeriod;
    stream >> SpecialOperationMode >> FrequencyTolerance >> TRPeriod;
    qDebug() << "        " << "SpecialOperationMode =" << SpecialOperationMode;
    if (FrequencyTolerance == 0xffffffff)
        qDebug() << "        " << "FrequencyTolerance = 0xffffffff";
    else
        qDebug() << "        " << "FrequencyTolerance =" << FrequencyTolerance;

    if (TRPeriod == 0xffffffff)
        qDebug() << "        " << "TRPeriod = 0xffffffff";
    else
        qDebug() << "        " << "TRPeriod =" << TRPeriod;

    stream.readBytes(raw, len);
    QString ConfigurationName = QString::fromUtf8(raw, len);

    stream.readBytes(raw, len);
    QString TxMessage = QString::fromUtf8(raw, len);
    qDebug() << "        " << "ConfigurationName =" << ConfigurationName << "TxMessage =" << TxMessage;
}

void Udp::decode(QDataStream &stream)
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

    qDebug() << caller;
}




