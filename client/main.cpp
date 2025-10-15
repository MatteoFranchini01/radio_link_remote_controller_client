#include <QCoreApplication>
#include <QTimer>
#include <QHostAddress>
#include <QDebug>

#include "ClientBackend.h".h"
#include "nettypes.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qInfo() << "== avvio client ==";

    ClientBackend client;

    QObject::connect(&client, &ClientBackend::error,
                     [](const QString& where, const QString& msg){
                         qWarning() << "[CLIENT][ERROR]" << where << ":" << msg;
                     });
    QObject::connect(&client, &ClientBackend::tcpConnected, [&](){
        qInfo() << "[CLIENT] connesso, invio messaggio...";
        client.sendTcp(TcpPacket{ QByteArray("Ciao dal client!") });
    });
    QObject::connect(&client, &ClientBackend::tcpDataReceived,
                     [](const QByteArray& data){
                         qInfo() << "[CLIENT] RX:" << data;
                     });

    if (!client.start(QHostAddress::LocalHost, 5551, 5500)) {
        qCritical() << "start fallito";
        return 1;
    }

    QTimer::singleShot(8000, [&](){ qInfo() << "stop client"; client.stop(); app.quit(); });
    return app.exec();
}
