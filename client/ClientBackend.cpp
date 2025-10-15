#include "ClientBackend.h".h"
#include "TcpClientWorker.h".h"
#include "ClientUdpWorker.h".h"
#include "nettypes.h"

#include <QMetaType>
#include <QMetaObject>
#include <QHostAddress>

ClientBackend::ClientBackend(QObject* parent)
    : QObject(parent)
{
    // Registra i metatipi usati tra thread
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

    // Istanzia i worker nel thread corrente (poi li spostiamo)
    m_tcpWorker = new TcpClientWorker();
    m_udpWorker = new UdpClientWorker();

    // Sposta i worker nei rispettivi thread
    m_tcpWorker->moveToThread(&m_tcpThread);
    m_udpWorker->moveToThread(&m_udpThread);

    // Garbage-collection worker a fine thread
    connect(&m_tcpThread, &QThread::finished, m_tcpWorker, &QObject::deleteLater);
    connect(&m_udpThread, &QThread::finished, m_udpWorker, &QObject::deleteLater);

    // Ribattiamo i segnali del TCP worker
    connect(m_tcpWorker, &TcpClientWorker::connected,
            this, &ClientBackend::tcpConnected);
    connect(m_tcpWorker, &TcpClientWorker::disconnected,
            this, &ClientBackend::tcpDisconnected);
    connect(m_tcpWorker, &TcpClientWorker::dataReceived,
            this, &ClientBackend::tcpDataReceived);
    connect(m_tcpWorker, &TcpClientWorker::error, this,
            [this](const QString& msg){ emit error(QStringLiteral("TcpClientWorker"), msg); });

    // Ribattiamo i segnali dell’UDP worker
    connect(m_udpWorker, &UdpClientWorker::dataReceived,
            this, &ClientBackend::udpDataReceived);
    connect(m_udpWorker, &UdpClientWorker::error, this,
            [this](const QString& msg){ emit error(QStringLiteral("UdpClientWorker"), msg); });

    // Facoltativi: segnali di lifecycle del backend
    connect(&m_tcpThread, &QThread::started, this, [this](){
        // emetti started solo quando partono entrambi
        if (m_udpThread.isRunning()) emit started();
    });
    connect(&m_udpThread, &QThread::started, this, [this](){
        if (m_tcpThread.isRunning()) emit started();
    });
    connect(&m_tcpThread, &QThread::finished, this, [this](){
        if (!m_udpThread.isRunning()) emit stopped();
    });
    connect(&m_udpThread, &QThread::finished, this, [this](){
        if (!m_tcpThread.isRunning()) emit stopped();
    });

    // Avvia i thread (creano i rispettivi event loop)
    m_tcpThread.start();
    m_udpThread.start();

    // Avvia i worker nel loro thread tramite invokeMethod (queued)
    QMetaObject::invokeMethod(
        m_tcpWorker, "start", Qt::QueuedConnection,
        Q_ARG(QHostAddress, serverIp),
        Q_ARG(quint16, tcpPort)
        );

    QMetaObject::invokeMethod(
        m_udpWorker, "start", Qt::QueuedConnection,
        Q_ARG(quint16, udpLocalBindPort) // 0 = porta effimera locale
        );

    m_running = true;
    return true;
}

void ClientBackend::stop()
{
    if (!m_running) return;

    // Chiedi ai worker di fermarsi nel loro thread
    if (m_tcpWorker) {
        QMetaObject::invokeMethod(m_tcpWorker, "stop", Qt::QueuedConnection);
    }
    if (m_udpWorker) {
        QMetaObject::invokeMethod(m_udpWorker, "stop", Qt::QueuedConnection);
    }

    // Arresta i thread e attendi la chiusura
    m_tcpThread.quit();
    m_udpThread.quit();
    m_tcpThread.wait();
    m_udpThread.wait();

    m_tcpWorker = nullptr;
    m_udpWorker = nullptr;
    m_running = false;
}

void ClientBackend::sendTcp(const TcpPacket& pkt)
{
    if (!m_running || !m_tcpWorker) {
        emit error(QStringLiteral("ClientBackend"),
                   QStringLiteral("TCP non disponibile (backend non avviato)"));
        return;
    }

    QMetaObject::invokeMethod(
        m_tcpWorker, "send", Qt::QueuedConnection,
        Q_ARG(TcpPacket, pkt)
        );
}

void ClientBackend::sendUdp(const UdpPacket& pkt)
{
    if (!m_running || !m_udpWorker) {
        emit error(QStringLiteral("ClientBackend"),
                   QStringLiteral("UDP non disponibile (backend non avviato)"));
        return;
    }

    QMetaObject::invokeMethod(
        m_udpWorker, "send", Qt::QueuedConnection,
        Q_ARG(UdpPacket, pkt)
        );
}
