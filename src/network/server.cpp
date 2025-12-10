#include <QCoreApplication>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>


int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    qInfo() << "started";

    /* Udp server
    QUdpSocket sock;
    sock.bind(QHostAddress("127.0.0.1"), 12345);
    QObject::connect(&sock, &QUdpSocket::readyRead, [&]() {
        while (sock.hasPendingDatagrams()) {
            QByteArray d;
            d.resize(sock.pendingDatagramSize());
            QHostAddress from;
            quint16 port;
            sock.readDatagram(d.data(), d.size(), &from, &port);
            qInfo() << "DATA: " << d;
        }
    });*/


    /* TCP server
    QTcpServer server;
    if (!server.listen(QHostAddress("127.0.0.1"), 12345)) {
        qInfo() << server.errorString();
    }
    QObject::connect(&server, &QTcpServer::newConnection,[&server]() {
        QTcpSocket* socket = server.nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead,
            socket, [socket]() {
            QByteArray data = socket->readAll();
            qInfo() << "DATA: " << data;
        });
    });*/

    return app.exec();
}

