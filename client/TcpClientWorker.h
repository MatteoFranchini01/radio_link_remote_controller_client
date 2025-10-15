#ifndef TCPCLIENTWORKER_H
#define TCPCLIENTWORKER_H

#include <QObject>

#pragma once
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "nettypes.h"

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
    QTcpSocket* m_sock = nullptr;
    QTimer* m_reconnectTimer = nullptr;

    QHostAddress m_ip;
    quint16 m_port = 0;
};


#endif // TCPCLIENTWORKER_H
