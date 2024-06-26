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
        //heartbeat(stream);
        break;
    case Status:
        //qDebug() << "Status";
        status(stream);
        break;
    case Decode:
        //qDebug() << "Decode";
        decode(stream);
        break;
    case Clear:
        //qDebug() << "Clear";
        //clear(stream);
        break;
    case QSOLogged:
        //qDebug() << "QSOLogged";
        //qsoLogged(stream);
        break;
    case Close:
        //qDebug() << "Close";
        close(stream);
        break;
    case WSPRDecode:
        qDebug() << "WSPRDecode";
        break;
    case LoggedADIF:
        //qDebug() << "LoggedADIF";
        loggedADIF(stream);
        break;
    default:
        qDebug() << "Unknown message number received: " << messageNumber;
        break;
    }
}

void Udp::heartbeat(QDataStream &stream)
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
    QString Id = QString::fromUtf8(raw, static_cast<int>(len));
    quint64 Dial_frequency;
    stream >> Dial_frequency;
    stream.readBytes(raw, len);
    QString Mode = QString::fromUtf8(raw, static_cast<int>(len));
    stream.readBytes(raw, len);
    QString Dx_call = QString::fromUtf8(raw, static_cast<int>(len));
    stream.readBytes(raw, len);
    QString Report = QString::fromUtf8(raw, static_cast<int>(len));
    stream.readBytes(raw, len);
    QString Tx_mode = QString::fromUtf8(raw, static_cast<int>(len));
    bool Tx_Enabled;
    bool Transmitting;
    bool Decoding;
    stream >> Tx_Enabled >> Transmitting >> Decoding;

    // qDebug() << "Status: " << "Id =" << Id << "freq =" << Dial_frequency << "mode =" << Mode
    //          << "dx call =" << Dx_call << "report =" << Report << "tx mode =" << Tx_mode << "tx enabled =" << Tx_Enabled
    //          << "transmitting =" << Transmitting << "decoding =" << Decoding;
    quint32 Tx_df;
    quint32 Rx_df;
    stream >> Tx_df >> Rx_df;
    stream.readBytes(raw, len);
    QString Call = QString::fromUtf8(raw, static_cast<int>(len));
    stream.readBytes(raw, len);
    QString Grid = QString::fromUtf8(raw, static_cast<int>(len));
    stream.readBytes(raw, len);
    QString DxGrid = QString::fromUtf8(raw, static_cast<int>(len));

    // qDebug() << "        " << "txdf =" << Tx_df << "rxdf =" << Rx_df << "mycall =" << Call << "mygrid =" << Grid << "dxgrid =" << DxGrid;

    bool TxWatchdog;
    stream >> TxWatchdog;

    QString SubMode;
    quint32 count;
    stream >> count;
    if (count < 0xffffffff)
    {
        raw = new char[count];
        stream.readRawData(raw, static_cast<int>(count));
        SubMode = QString::fromUtf8(raw, static_cast<int>(count));
        delete [] raw;
    }

    bool FastMode;
    stream >> FastMode;

    // qDebug() << "        " << "TxWatchdog =" << TxWatchdog <<  "SubMode =" << SubMode << "FastMode =" << FastMode;

    quint8 SpecialOperationMode;
    quint32 FrequencyTolerance;
    quint32 TRPeriod;
    stream >> SpecialOperationMode >> FrequencyTolerance >> TRPeriod;
    // qDebug() << "        " << "SpecialOperationMode =" << SpecialOperationMode;
    // if (FrequencyTolerance == 0xffffffff)
    //     qDebug() << "        " << "FrequencyTolerance = 0xffffffff";
    // else
    //     qDebug() << "        " << "FrequencyTolerance =" << FrequencyTolerance;

    // if (TRPeriod == 0xffffffff)
    //     qDebug() << "        " << "TRPeriod = 0xffffffff";
    // else
    //     qDebug() << "        " << "TRPeriod =" << TRPeriod;

    stream.readBytes(raw, len);
    QString ConfigurationName = QString::fromUtf8(raw, static_cast<int>(len));

    stream.readBytes(raw, len);
    QString TxMessage = QString::fromUtf8(raw, static_cast<int>(len));
    // qDebug() << "        " << "ConfigurationName =" << ConfigurationName << "TxMessage =" << TxMessage;

    emit FreqChange(Dial_frequency);
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

    if (message.length() != 0)
        emit MessageReceived(message);
}

void Udp::clear(QDataStream &stream)
{
    uint len;
    char *raw;
    stream.readBytes(raw, len);
    QString Id = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "Clear: Id =" << Id;
}

void Udp::qsoLogged(QDataStream &stream)
{
    uint len;
    char *raw;
    stream.readBytes(raw, len);
    QString Id = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "qsoLogged: Id =" << Id;

    QDateTime DateTimeOff;
    stream >> DateTimeOff;
    qDebug() << "DateTimeOff =" << DateTimeOff;

    stream.readBytes(raw, len);
    QString DXcall = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "DXcall =" << DXcall;

    stream.readBytes(raw, len);
    QString DXgrid = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "DXgrid =" << DXgrid;

    quint64 TxFrequency;
    stream >> TxFrequency;
    qDebug() << "TxFrequency =" << TxFrequency;

    stream.readBytes(raw, len);
    QString Mode = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "Mode =" << Mode;

    stream.readBytes(raw, len);
    QString ReportSent = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "ReportSent =" << ReportSent;

    stream.readBytes(raw, len);
    QString ReportReceived = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "ReportReceived =" << ReportReceived;

    stream.readBytes(raw, len);
    QString TxPower = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "TxPower =" << TxPower;

    stream.readBytes(raw, len);
    QString Comments = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "Comments =" << Comments;

    stream.readBytes(raw, len);
    QString Name = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "Name =" << Name;

    QDateTime DateTimeOn;
    stream >> DateTimeOn;
    qDebug() << "DateTimeOn =" << DateTimeOn;

    stream.readBytes(raw, len);
    QString OperatorCall = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "OperatorCall =" << OperatorCall;

    stream.readBytes(raw, len);
    QString MyCall = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "MyCall =" << MyCall;

    stream.readBytes(raw, len);
    QString MyGrid = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "MyGrid =" << MyGrid;

    stream.readBytes(raw, len);
    QString ExchangeSent = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "ExchangeSent =" << ExchangeSent;

    stream.readBytes(raw, len);
    QString ExchangeReceived = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "ExchangeReceived =" << ExchangeReceived;

    stream.readBytes(raw, len);
    QString ADIFPropagationMode = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "ADIFPropagationMode =" << ADIFPropagationMode;
}

void Udp::close(QDataStream &stream)
{
    uint len;
    char *raw;
    stream.readBytes(raw, len);
    QString Id = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "Close: Id =" << Id;
}

void Udp::loggedADIF(QDataStream &stream)
{
    uint len;
    char *raw;
    stream.readBytes(raw, len);
    QString Id = QString::fromUtf8(raw, static_cast<int>(len));
    stream.readBytes(raw, len);
    QString text = QString::fromUtf8(raw, static_cast<int>(len));
    qDebug() << "loggedADIF: Id =" << Id << "text:" << text;
}





