#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include "nettypes.h"

class QTimer;

class TcpClientWorker : public QObject
{
    Q_OBJECT
public:
    explicit TcpClientWorker(QObject* parent = nullptr);

public slots:
    void start(QHostAddress ip, quint16 port);
    void stop();
    void send(const TcpPacket& pkt);

signals:
    void connected();
    void disconnected(QString reason);
    void dataReceived(QByteArray data);
    void error(QString message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onReconnectTimeout();

private:
    QTcpSocket*  m_sock = nullptr;         // creato in start()
    QTimer*      m_reconnectTimer = nullptr; // creato in start()
    QHostAddress m_ip;
    quint16      m_port = 0;
};
