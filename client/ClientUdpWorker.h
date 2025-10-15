#pragma once
#include <QObject>
#include <QUdpSocket>
#include "nettypes.h"

class UdpClientWorker : public QObject
{
    Q_OBJECT
public:
    explicit UdpClientWorker(QObject* parent = nullptr);

public slots:
    void start(quint16 localBindPort);   // 0 = porta effimera
    void stop();
    void send(const UdpPacket& pkt);     // invia verso pkt.peer:pkt.port

signals:
    void dataReceived(QHostAddress from, quint16 port, QByteArray data);
    void error(QString message);

private slots:
    void onReadyRead();

private:
    QUdpSocket* m_sock = nullptr;        // creato in start()
    quint16     m_localPort = 0;
};
