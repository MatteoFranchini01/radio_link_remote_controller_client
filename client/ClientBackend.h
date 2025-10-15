#pragma once
#include <QObject>
#include <QThread>
#include <QHostAddress>
#include "nettypes.h"

class TcpClientWorker;
class UdpClientWorker;

class ClientBackend : public QObject
{
    Q_OBJECT

public:
    explicit ClientBackend(QObject* parent = nullptr);
    ~ClientBackend() override;

    bool start(const QHostAddress& serverIp, quint16 tcpPort, quint16 udpLocalBindPort);
    void stop();

    void sendTcp(const TcpPacket& pkt);
    void sendUdp(const UdpPacket& pkt);

signals:
    void tcpConnected();
    void tcpDisconnected(QString reason);
    void tcpDataReceived(QByteArray data);
    void udpDataReceived(QHostAddress from, quint16 port, QByteArray data);
    void started();
    void stopped();
    void error(QString where, QString message);

private:
    QThread         m_tcpThread;
    QThread         m_udpThread;
    TcpClientWorker* m_tcpWorker = nullptr;
    UdpClientWorker* m_udpWorker = nullptr;
    bool            m_running = false;
};
