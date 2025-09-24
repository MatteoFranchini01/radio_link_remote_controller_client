#include "ClientUdpWorker.h".h"

UdpClientWorker::UdpClientWorker(QObject* p) : QObject(p)
{
    connect(&m_sock, &QUdpSocket::readyRead, this, &UdpClientWorker::onReadyRead);
}
void UdpClientWorker::start(quint16 localBindPort)
{
    m_localPort = localBindPort;
    if (not m_sock.bind(QHostAddress::Any, localBindPort, QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint))
        emit error(QStringLiteral("UDP bind failed: %1").arg(m_sock.errorString()));
}

void UdpClientWorker::stop()
{
    m_sock.close();
}

void UdpClientWorker::send(const UdpPacket& pkt)
{
    m_sock.writeDatagram(pkt.payload, pkt.peer, pkt.port);
}

void UdpClientWorker::onReadyRead()
{
    while (m_sock.hasPendingDatagrams())
    {
        QHostAddress from; quint16 port=0; QByteArray buf; buf.resize(int(m_sock.pendingDatagramSize()));
        m_sock.readDatagram(buf.data(), buf.size(), &from, &port);
        emit dataReceived(from, port, buf);
    }
}
