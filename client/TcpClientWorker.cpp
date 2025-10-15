#include "TcpClientWorker.h"

TcpClientWorker::TcpClientWorker(QObject* p) : QObject(p)
{
    // connect(&m_sock, &QTcpSocket::connected,    this, &TcpClientWorker::onConnected);
    // connect(&m_sock, &QTcpSocket::disconnected, this, &TcpClientWorker::onDisconnected);
    // connect(&m_sock, &QTcpSocket::readyRead,    this, &TcpClientWorker::onReadyRead);

    // m_reconnectTimer.setInterval(1000);
    // m_reconnectTimer.setSingleShot(false);
    // connect(&m_reconnectTimer, &QTimer::timeout, this, &TcpClientWorker::onReconnectTimeout);
}

void TcpClientWorker::start(QHostAddress ip, quint16 port)
{
    m_ip = ip; m_port = port;
    m_sock->connectToHost(m_ip, m_port);

    if (m_sock)
    {
        if(m_sock->state() == QAbstractSocket::UnconnectedState)
        {
            m_sock->connectToHost(m_ip, m_port);
        }
        return;
    }

    m_sock = new QTcpSocket(this);

    QObject::connect(m_sock, &QTcpSocket::connected,
                     this,   &TcpClientWorker::onConnected);
    QObject::connect(m_sock, &QTcpSocket::disconnected,
                     this,   &TcpClientWorker::onDisconnected);
    QObject::connect(m_sock, &QTcpSocket::readyRead,
                     this,   &TcpClientWorker::onReadyRead);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(1000);
    m_reconnectTimer->setSingleShot(false);
    QObject::connect(m_reconnectTimer, &QTimer::timeout,
                     this,             &TcpClientWorker::onReconnectTimeout);

    // Prima connessione
    m_sock->connectToHost(m_ip, m_port);
}

void TcpClientWorker::stop()
{
    if (m_reconnectTimer) {
        m_reconnectTimer->stop();
        m_reconnectTimer->deleteLater();
        m_reconnectTimer = nullptr;
    }

    if (m_sock) {
        if (m_sock->state() != QAbstractSocket::UnconnectedState)
            m_sock->disconnectFromHost();
        m_sock->deleteLater();
        m_sock = nullptr;
    }
}

void TcpClientWorker::send(const TcpPacket& pkt)
{
    if (m_sock->state() == QAbstractSocket::ConnectedState) m_sock->write(pkt.payload);
}

void TcpClientWorker::onConnected()
{
    m_reconnectTimer->stop(); emit connected();
}

void TcpClientWorker::onDisconnected()
{
    emit disconnected(QStringLiteral("TCP disconnected"));
    m_reconnectTimer->start();
}

void TcpClientWorker::onReadyRead()
{
    emit dataReceived(m_sock->readAll());
}

void TcpClientWorker::onReconnectTimeout()
{
    if (m_sock->state() == QAbstractSocket::UnconnectedState) m_sock->connectToHost(m_ip, m_port);
}
