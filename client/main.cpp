#include "mainwindow.h"

#include <QApplication>
#include <QTimer>
#include <QHostAddress>
#include <QDebug>

#include "ClientBackend.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // MainWindow w;
    // w.show();

    fprintf(stderr, "Hello from client!\n");

    ClientBackend client;

    QObject::connect(&client, &ClientBackend::tcpConnected, [](){
        qDebug() << "[CLIENT] connesso al sever";
    });

    QObject::connect(&client, &ClientBackend::tcpDataReceived, [&](const QByteArray& data){
        qDebug() << "[CLIENT] Ricevuto dal server:" << data;
    });

    QTimer::singleShot(500, [&]() {
        client.sendTcp(TcpPacket { QByteArray("ciato dal client") });
    });

    QTimer::singleShot(5000, [&]() {
        qDebug() << "Arresto client";

        client.stop();

        app.quit();
    });

    return app.exec();
}
