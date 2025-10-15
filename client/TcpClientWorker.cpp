#include "TcpClientWorker.h".h"
#include <QTimer>
#include <QAbstractSocket>

TcpClientWorker::TcpClientWorker(QObject* p) : QObject(p) {}

void TcpClientWorker::start(QHostAddress ip, quint16 port)
{
    // eseguito nel thread del worker (invokeMethod Queued)
    m_ip = ip; m_port = port;

    if (!m_sock) {
        m_sock = new QTcpSocket(this);
        QObject::connect(m_sock, &QTcpSocket::connected,
                         this,   &TcpClientWorker::onConnected);
        QObject::connect(m_sock, &QTcpSocket::disconnected,
                         this,   &TcpClientWorker::onDisconnected);
        QObject::connect(m_sock, &QTcpSocket::readyRead,
                         this,   &TcpClientWorker::onReadyRead);
    }

    if (!m_reconnectTimer) {
        m_reconnectTimer = new QTimer(this);
        m_reconnectTimer->setInterval(1000);
        m_reconnectTimer->setSingleShot(false);
        QObject::connect(m_reconnectTimer, &QTimer::timeout,
                         this,             &TcpClientWorker::onReconnectTimeout);
    }

    m_sock->connectToHost(m_ip, m_port);
}

void TcpClientWorker::stop()
{
    if (m_reconnectTimer) { m_reconnectTimer->stop(); m_reconnectTimer->deleteLater(); m_reconnectTimer = nullptr; }
    if (m_sock) {
        if (m_sock->state() != QAbstractSocket::UnconnectedState) m_sock->disconnectFromHost();
        m_sock->deleteLater();
        m_sock = nullptr;
    }
}

void TcpClientWorker::send(const TcpPacket& pkt)
{
    if (m_sock && m_sock->state() == QAbstractSocket::ConnectedState) {
        m_sock->write(pkt.payload);
    } else {
        emit error(QStringLiteral("TCP non connesso"));
    }
}

void TcpClientWorker::onConnected()
{
    if (m_reconnectTimer) m_reconnectTimer->stop();
    emit connected();
}

void TcpClientWorker::onDisconnected()
{
    emit disconnected(QStringLiteral("TCP disconnected"));
    if (m_reconnectTimer && !m_reconnectTimer->isActive()) m_reconnectTimer->start();
}

void TcpClientWorker::onReadyRead()
{
    if (!m_sock) return;
    emit dataReceived(m_sock->readAll());
}

void TcpClientWorker::onReconnectTimeout()
{
    if (m_sock && m_sock->state() == QAbstractSocket::UnconnectedState)
        m_sock->connectToHost(m_ip, m_port);
}
