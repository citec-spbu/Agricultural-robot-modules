#ifndef ROBOT_NETWORK_UDP_H_
#define ROBOT_NETWORK_UDP_H_

#include <QObject>
#include <QUdpSocket>

#include "Base.h"

namespace RobotNetwork {

class Udp : public BaseConnection {
    Q_OBJECT
public:
    explicit Udp(QObject* parent = nullptr);

    Capabilities caps() const override;

    void open(const QUrl& target) override;
    void open(const QString& host, quint16 port);
    void close() override;
    bool isOpen() const override;
    qint64 send(const QByteArray& data) override;

private:
    QUdpSocket sock_;
    QString host_{};
    quint16 port_{};
};

}

#endif // ! ROBOT_NETWORK_UDP_H_