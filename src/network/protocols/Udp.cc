#include "Udp.h"

namespace RobotNetwork {

Udp::Udp(QObject *parent) : BaseConnection(parent) {
    connect(&sock_, &QUdpSocket::readyRead, this, [this]() {
        while (sock_.hasPendingDatagrams()) {
            QByteArray buf;
            buf.resize(sock_.pendingDatagramSize());
            sock_.readDatagram(buf.data(), buf.size(), nullptr, nullptr);
            emit received(buf);
        }
    });

    connect(&sock_, &QUdpSocket::errorOccurred, this, [this](auto) {
        emit error(sock_.errorString());
    });
}

Capabilities Udp::caps() const { return Capability::Datagram; }

void Udp::open(const QUrl &target) {
    open(target.host(), target.port());
}

void Udp::open(const QString &host, const quint16 port) {
    host_ = host;
    port_ = port;
    if (!sock_.bind(QHostAddress::AnyIPv4, 0)) {
        emit error("bind failed");
        return;
    }
    emit connected();
}

void Udp::close() {
    sock_.close();
    emit disconnected();
}

bool Udp::isOpen() const {
    return sock_.state() == QAbstractSocket::BoundState;
}

qint64 Udp::send(const QByteArray &data) {
    return sock_.writeDatagram(data, QHostAddress(host_), port_);
}

}  // namespace RobotNetwork