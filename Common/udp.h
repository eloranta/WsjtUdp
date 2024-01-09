#ifndef udp_h
#define udp_h

#include <QObject>
#include <QUdpSocket>
#include <QJsonArray>

class udp : public QObject
{
    Q_OBJECT
private:
    enum Type
    {
        Heartbeat,
        Status,
        Decode,
        Clear,
        Reply,
        QSOLogged,
        Close,
        Replay,
        HaltTx,
        FreeText,
        WSPRDecode,
        Location,
        LoggedADIF,
        HighlightCallsign,
        SwitchConfiguration,
        Configure,
        maximum_message_type_     // ONLY add new message types immediately before here
    };

    QUdpSocket *socket;
    QJsonArray array;
    void ParseMessage(QByteArray& buffer);
    void decode(QDataStream &stream);
    QString FindCountry(QString& call);
    void ReadDxccJson();
public:
    explicit udp(QObject *parent = nullptr);
signals:

public slots:
    void readyRead();
};

#endif
