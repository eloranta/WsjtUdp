#ifndef udp_h
#define udp_h

#include <QUdpSocket>

class Udp : public QObject
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
    void ParseMessage(QByteArray& buffer);
    void heartbeat(QDataStream &stream);
    void status(QDataStream &stream);
    void decode(QDataStream &stream);
    void clear(QDataStream &stream);
    void qsoLogged(QDataStream &stream);
    void close(QDataStream &stream);
    void loggedADIF(QDataStream &stream);

public:
    explicit Udp(QObject *parent = nullptr);
signals:
    void MessageReceived(QString message);
    void FreqChange(int freq);
public slots:
    void readyRead();
};

#endif
