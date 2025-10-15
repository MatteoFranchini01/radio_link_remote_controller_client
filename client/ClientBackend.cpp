#include "ClientBackend.h"
#include "TcpClientWorker.h"
#include "ClientUdpWorker.h"

#include <QMetaType>
#include <QMetaObject>
#include <QDebug>

ClientBackend::ClientBackend(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<TcpPacket>("TcpPacket");
    qRegisterMetaType<UdpPacket>("UdpPacket");
    qRegisterMetaType<QByteArray>("QByteArray");
    qRegisterMetaType<QHostAddress>("QHostAddress");
}

ClientBackend::~ClientBackend()
{
    stop();
}

bool ClientBackend::start(const QHostAddress& serverIp, quint16 tcpPort, quint16 udpLocalBindPort)
{
    if (m_running) return true;

    m_tcpWorker = new TcpClientWorker();
    m_udpWorker = new UdpClientWorker();

    m_tcpWorker->moveToThread(&m_tcpThread);
    m_udpWorker->moveToThread(&m_udpThread);

    QObject::connect(&m_tcpThread, &QThread::finished, m_tcpWorker, &QObject::deleteLater);
    QObject::connect(&m_udpThread, &QThread::finished, m_udpWorker, &QObject::deleteLater);

    // ribatto segnali
    QObject::connect(m_tcpWorker, &TcpClientWorker::connected,    this, &ClientBackend::tcpConnected);
    QObject::connect(m_tcpWorker, &TcpClientWorker::disconnected, this, &ClientBackend::tcpDisconnected);
    QObject::connect(m_tcpWorker, &TcpClientWorker::dataReceived, this, &ClientBackend::tcpDataReceived);
    QObject::connect(m_tcpWorker, &TcpClientWorker::error, this,
                     [this](const QString& m){ emit error(QStringLiteral("TcpClientWorker"), m); });

    QObject::connect(m_udpWorker, &UdpClientWorker::dataReceived, this, &ClientBackend::udpDataReceived);
    QObject::connect(m_udpWorker, &UdpClientWorker::error, this,
                     [this](const QString& m){ emit error(QStringLiteral("UdpClientWorker"), m); });

    // opzionali: log dei thread
    QObject::connect(&m_tcpThread, &QThread::started, [](){ qInfo() << "[ClientBackend] TCP thread started"; });
    QObject::connect(&m_udpThread, &QThread::started, [](){ qInfo() << "[ClientBackend] UDP thread started"; });
    QObject::connect(&m_tcpThread, &QThread::finished, [this](){ if (!m_udpThread.isRunning()) emit stopped(); });
    QObject::connect(&m_udpThread, &QThread::finished, [this](){ if (!m_tcpThread.isRunning()) emit stopped(); });

    m_tcpThread.start();
    m_udpThread.start();

    const bool ok1 = QMetaObject::invokeMethod(
        m_tcpWorker, "start", Qt::QueuedConnection,
        Q_ARG(QHostAddress, serverIp),
        Q_ARG(quint16, tcpPort)
        );
    if (!ok1) { qCritical() << "[ClientBackend] invokeMethod Tcp start FAILED"; return false; }

    const bool ok2 = QMetaObject::invokeMethod(
        m_udpWorker, "start", Qt::QueuedConnection,
        Q_ARG(quint16, udpLocalBindPort)
        );
    if (!ok2) { qCritical() << "[ClientBackend] invokeMethod Udp start FAILED"; return false; }

    m_running = true;
    emit started();
    return true;
}

void ClientBackend::stop()
{
    if (!m_running) return;

    if (m_tcpWorker) QMetaObject::invokeMethod(m_tcpWorker, "stop", Qt::QueuedConnection);
    if (m_udpWorker) QMetaObject::invokeMethod(m_udpWorker, "stop", Qt::QueuedConnection);

    m_tcpThread.quit(); m_udpThread.quit();
    m_tcpThread.wait(); m_udpThread.wait();

    m_tcpWorker = nullptr;
    m_udpWorker = nullptr;
    m_running = false;
}

void ClientBackend::sendTcp(const TcpPacket& pkt)
{
    if (!m_running || !m_tcpWorker) {
        emit error(QStringLiteral("ClientBackend"), QStringLiteral("TCP non disponibile"));
        return;
    }
    QMetaObject::invokeMethod(m_tcpWorker, "send", Qt::QueuedConnection, Q_ARG(TcpPacket, pkt));
}

void ClientBackend::sendUdp(const UdpPacket& pkt)
{
    if (!m_running || !m_udpWorker) {
        emit error(QStringLiteral("ClientBackend"), QStringLiteral("UDP non disponibile"));
        return;
    }
    QMetaObject::invokeMethod(m_udpWorker, "send", Qt::QueuedConnection, Q_ARG(UdpPacket, pkt));
}
