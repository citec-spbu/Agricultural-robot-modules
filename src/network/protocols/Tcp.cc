#include "Tcp.h"

namespace RobotNetwork {

Tcp::Tcp(QObject* parent) : BaseConnection(parent) {
    connect(&sock_, &QTcpSocket::connected,    this, &BaseConnection::connected);
    connect(&sock_, &QTcpSocket::disconnected, this, &BaseConnection::disconnected);
    connect(&sock_, &QTcpSocket::readyRead,    this, [this]{ emit received(sock_.readAll()); });
    connect(&sock_, &QTcpSocket::errorOccurred,this, [this](auto){ emit error(sock_.errorString()); });
}

void Tcp::open(const QUrl &target) {
    open(target.host(), target.port());
}

void Tcp::open(const QString& host, const quint16 port) {
    host_ = host;
    port_ = port;
    sock_.connectToHost(host_, port_);
    if (!sock_.waitForConnected(3000)) {
        qInfo() << "connect error:" << sock_.errorString();
    }
}

void Tcp::close() {
    sock_.disconnectFromHost();
    if (!sock_.waitForConnected(3000)) {
        qInfo() << "connect error:" << sock_.errorString();
    }
}

bool Tcp::isOpen() const {
    return sock_.state() == QAbstractSocket::ConnectedState;
}

qint64 Tcp::send(const QByteArray& data) {
    if (!isOpen()) {
        qInfo() << "not connected";
    }
    return sock_.write(data);
}

Capabilities Tcp::caps() const {
    return Capability::ByteStream;
}

}