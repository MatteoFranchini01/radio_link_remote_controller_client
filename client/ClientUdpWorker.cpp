#include "ClientUdpWorker.h"

UdpClientWorker::UdpClientWorker(QObject* p) : QObject(p) {}

void UdpClientWorker::start(quint16 localBindPort)
{
    m_localPort = localBindPort;

    if (!m_sock) {
        m_sock = new QUdpSocket(this);
        QObject::connect(m_sock, &QUdpSocket::readyRead,
                         this,   &UdpClientWorker::onReadyRead);
    }

    if (!m_sock->bind(QHostAddress::Any, m_localPort,
                      QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        emit error(QStringLiteral("UDP bind failed: %1").arg(m_sock->errorString()));
    }
}

void UdpClientWorker::stop()
{
    if (m_sock) {
        m_sock->close();
        m_sock->deleteLater();
        m_sock = nullptr;
    }
}

void UdpClientWorker::send(const UdpPacket& pkt)
{
    if (!m_sock) { emit error(QStringLiteral("UDP non inizializzato")); return; }
    m_sock->writeDatagram(pkt.payload, pkt.peer, pkt.port);
}

void UdpClientWorker::onReadyRead()
{
    if (!m_sock) return;
    while (m_sock->hasPendingDatagrams()) {
        QHostAddress from; quint16 port = 0;
        QByteArray buf; buf.resize(int(m_sock->pendingDatagramSize()));
        m_sock->readDatagram(buf.data(), buf.size(), &from, &port);
        emit dataReceived(from, port, buf);
    }
}
